#include <IoService.h>
#include <JsonSerializer.h>
#include <MsgIo.h>
#include <errno.h>

namespace Yail {

void
MsgIoSession::init(String sessionName, int fd, SPtr<IoService> ioSvc,
                   SPtr<ConnectionCallback> connectionCallback) {
  sessionName_ = sessionName;
  connectionCallback_ = connectionCallback;
  IoObject::init(sessionName + ".ioObj", fd, ioSvc);
}

Ostream&
operator<<(Ostream& os, const MsgIoSession& msgSession) {
  os << (IoObject)(msgSession);
  return os;
}

void
MsgIoSession::disconnectSession() {
  TRACE9("");

  int fd = this->fd();
  if(fd < 0) return;

  IoObject::deinit();
  USE_WPTR(ConnectionCallback, cb, connectionCallback_,
      {},
      {
        cb->onDisconnected(sessionName_, fd);
      },
      {}, {});
}

void
MsgIoSession::handleRx() {
  int nRead = read(fd(), rxFifo_ + rxTail_, FifoSize - rxTail_);
  if(nRead > 0) {
    rxTail_ += nRead;

    while(true) {
      unsigned char* buf = rxFifo_ + rxHead_;
      int bufLen = rxTail_ - rxHead_;

      if((bufLen > 0) && (*buf != Sig)) {
        // signature check failure
        TRACE0(sessionName_ << ": " <<
               "signature error:0x" << std::hex << (int)*buf << std::dec);
        disconnectSession();
        break;
      }

      if(bufLen >= HdrLen) {
        int type = buf[1];
        int flags = buf[2];
        int msgLen = ( buf[3] << 8 ) + buf[4];
        unsigned char* msgData = buf + HdrLen;

        if(bufLen >= msgLen + HdrLen) {
          msgReceived(type, flags, msgData, msgLen);
          rxHead_ += msgLen + HdrLen;
          continue;
        } else {
          TRACE9(sessionName_ << ": " << "type:" << type <<
                 ", waiting for more data:" << msgLen + HdrLen - bufLen);
        }
      }

      memmove(rxFifo_, rxFifo_ + rxHead_, bufLen);
      rxHead_ = 0;
      rxTail_ = bufLen;
      break;
    }

  } else if(nRead == 0) { // connection closed
    TRACE9(sessionName_ << ": " << "connection closed by foreign host");
    disconnectSession();
  } else {
    TRACE0(sessionName_ << ": " << "read error");
  }
}

void
MsgIoSession::handleTx() {
  int nWritten = write(fd(), txFifo_ + txHead_, txTail_ - txHead_);
  if(nWritten > 0) {
    txHead_ += nWritten;
    TRACE9("nWritten:" << nWritten);
    while(!txTrackers_.empty()) {
      TxTracker txMsgTracker = txTrackers_.front();
      //TRACE9(TARG("txHead_", txHead_) << TARG(", txTail_", txTail_) <<
      //    TARG(", txMsgHead_", txMsgHead_) <<
      //    TARG(", msgLen", txMsgTracker.len));
      if(txHead_ < txMsgHead_ + txMsgTracker.len) break;

      txMsgHead_ += txMsgTracker.len;
      txTrackers_.pop();
      msgSent(txMsgTracker.id);
    }

    if( txHead_ == txTail_) {
      assert(txTrackers_.empty());
      assert(txMsgHead_ == txHead_);
      txHead_ = txTail_ = txMsgHead_ = 0;
    } else if(!txMsgHead_ &&
              (txCopyMoveSize() < DataCopyThreshold) &&
              (txFifoRoom() < FragmentThreshold)) {
      memmove(txFifo_, txFifo_ + txMsgHead_, txTail_ - txMsgHead_);
      txHead_ -= txMsgHead_;
      txTail_ -= txMsgHead_;
      txMsgHead_ = 0;
    }

    sendData(); // refill data

  } else if(nWritten == 0) {
    TRACE0(sessionName_ << ": " << "nwritten = 0");
  } else {
    TRACE0(sessionName_ << ": " << "write error");
  }

  if( txHead_ == txTail_) {
    idleTx();
  }
}

void
MsgIoSession::handleIo(bool rdReady, bool wrReady) {
  TRACE9(sessionName_ << ": " <<
         "rdReady=" << rdReady << ", wrReady=" << wrReady);

  if(rdReady) handleRx();
  if(wrReady) handleTx();
}

void
MsgIoSession::msgReceived(int type, int flags, const unsigned char* msgData, int msgLen ) {
  TRACE9(sessionName_ << ": " << "type:" << type <<
         ", flags:" << flags << ", len:" << msgLen);

  switch(type) {
    case MSG_TYPE_PEER_INFO: {
      String s( reinterpret_cast<char const*>(msgData), msgLen );
      peerName_ = s;
      TRACET(sessionName_ << " Peer:" << peerName_ << "\n");
      break;
    }
    case MSG_TYPE_PUT_OBJECT: {
      handlePutObjectMsg(flags, msgData, msgLen);
      break;
    }
    case MSG_TYPE_DEL_OBJECT: {
      String path(reinterpret_cast<char const*>(msgData), msgLen);
      TRACET(sessionName_ << " del "  << path << "\n");
      handleDelObject(path);
      break;
    }
    case MSG_TYPE_TRANSACTION: {
      handleTransactionMsg(flags, msgData, msgLen);
      break;
    }
    default: {
      TRACE9(sessionName_ << ": " << "unknown msgType: " << type);
      break;
    }
  }
}

int
MsgIoSession::sendMsg(int type, int flags, const unsigned char* msgData, int msgLen, String id) {
  if(msgLen > MaxMsgLen) return E2BIG;

  int bufRoom = FifoSize - txTail_;
  if(bufRoom >= msgLen + HdrLen) {
    unsigned char* buf = txFifo_ + txTail_;
    *buf = Sig;
    *(buf + 1) = type;
    *(buf + 2) = flags;
    *(buf + 3) = msgLen / 256;
    *(buf + 4) = msgLen % 256;
    memcpy(buf + HdrLen, msgData, msgLen);
    txTail_ += (msgLen + HdrLen);
    txTrackers_.push(TxTracker(id, HdrLen + msgLen));
    requestTx();
    return 0;
  } else {
    return EBUSY;
  }
}

void
MsgIoSession::sendData() {
  while(true) {
    if(!sMsg_) {
      SPtr<MsgInfo> msgInfo = deqMsg();
      if(!msgInfo) break; // nothing to send
      TRACE9("serializing " << msgInfo->msgType());
      sMsg_ = msgInfo->makeSerializedMsg();
      sMsgTxPos_ = 0;
      assert(!sMsg_->empty());
    }

    int txRoom = txFifoRoom();
    int dataLen = sMsg_->dataLen() - sMsgTxPos_;
    if((txRoom >= dataLen) ||
       (!sMsg_->dontFrag() && (txRoom >= FragmentThreshold))) {
      int txLen = std::min(txRoom, dataLen);
      sendMsg(sMsg_->msgType(),
              (txRoom >= dataLen) ? MSG_FLAG_NONE : MSG_FLAG_MORE_FRAG,
              sMsg_->dataPtr(sMsgTxPos_), txLen);

      sMsgTxPos_ += txLen;
      if(sMsgTxPos_ >= sMsg_->dataLen()) {
        sMsg_.reset();
        sMsgTxPos_ = 0;
        continue;
      }
    }
    break;
  }
}

void
MsgIoSession::enqPutObjectMsg(SPtr<PutObjectMsg> putObjMsg) {
  String path = putObjMsg->path();
  txMsgList_.push_back(putObjMsg);
  TRACE9(path << " enqueued");
}

void
MsgIoSession::enqDelObjectMsg(SPtr<DelObjectMsg> delObjMsg) {
  String path = delObjMsg->path();
  txMsgList_.push_back(delObjMsg);
  TRACE9(path << " enqueued");
}

void
MsgIoSession::enqMsg(SPtr<MsgInfo> msgInfo) {
  switch(msgInfo->msgType()) {
    case MSG_TYPE_PUT_OBJECT:
    {
      SPtr<PutObjectMsg> putObjMsg = DynamicPointerCast<PutObjectMsg>(msgInfo);
      assert(putObjMsg);
      enqPutObjectMsg(putObjMsg);
      break;
    }
    case MSG_TYPE_DEL_OBJECT:
    {
      SPtr<DelObjectMsg> delObjMsg = DynamicPointerCast<DelObjectMsg>(msgInfo);
      assert(delObjMsg);
      enqDelObjectMsg(delObjMsg);
      break;
    }
    default:
      txMsgList_.push_back(msgInfo);
      break;
  }

  sendData();
}

SPtr<MsgInfo>
MsgIoSession::deqMsg() {
  if(txMsgList_.empty()) return nullPtr(MsgInfo);
  SPtr<MsgInfo> msgInfo = txMsgList_.front();
  txMsgList_.pop_front();
  return msgInfo;
}

#if 0 // just for debugging
static void print_data(const unsigned char* msgData, int msgLen) {
  char line[256] = { 0 };
  char* dptr;
  char* aptr;
  int perLine = 16;

  for(int i = 0; i < msgLen; i++) {
      if(i % perLine == 0) {
          if(line[0]) printf("%s\n", line);

          dptr = line;
          aptr = line + 56;
          bzero(line, sizeof(line));
          memset(line, ' ', aptr - line);
      }
      dptr += sprintf(dptr, "%02x ", msgData[i]);
      *dptr = ' ';
      *aptr++ = isprint(msgData[i]) ? msgData[i] : '.';
  }
  if(line[0]) printf("%s\n", line);
}
#endif

void
MsgIoSession::handlePutObjectMsg(int flags, const unsigned char* msgData, int msgLen) {
  TRACE9("received " << msgLen << " bytes");
  //print_data(msgData, msgLen);

  int n = 0;
  if(objInfo_.empty() || (objInfo_.compare(objInfo_.length() - 1, 1, "=") != 0)) {
    while(n < msgLen) {
      if(msgData[n] == '=') {
        n++;
        break;
      }
      n++;
    }
    objInfo_ += String(msgData, msgData + n);
    if(n >= msgLen) return;
  }
  ss_.write(reinterpret_cast<const char *>(msgData + n), msgLen - n);

  if(!(flags & MSG_FLAG_MORE_FRAG)) {
    TRACE9("object received, " << ss_.tellp() << ", " << objInfo_ << ss_.str());

    size_t pos = objInfo_.find(',');
    String path = objInfo_.substr(0, pos);

    String typeName, opt;
    size_t nextPos = objInfo_.find(',', pos + 1);
    if(nextPos != String::npos) {
      typeName = objInfo_.substr(pos + 1, nextPos - pos - 1);
      opt = objInfo_.substr(nextPos + 1, objInfo_.length() - nextPos - 2);
    } else {
      typeName = objInfo_.substr(pos + 1, objInfo_.length() - pos - 2);
    }

    SPtr<SerializedObject> serObj = CreateObject<SerializedObject>(typeName, ss_.str());

    TRACET(sessionName_ << " " << typeName << " "  << path << " " << serObj->data());

    SPtr<ObexObject> oObj;
    if(obexTree_->fwdOnly() && (path.compare(0, 6, "/proc/") != 0)) {
      TRACE9("path:" << path << ", typeName:" << typeName);
      oObj = CreateObject<ObexObject>(serObj);
    } else {
      TRACE9("path:" << path << ", typeName:" << typeName);
      oObj = ObexObjectFactory::instantiate(typeName, serObj);
    }

    handlePutObject(path, oObj, opt);

    objInfo_ = "";
    ss_.str("");
  }
}

void
MsgIoSession::handleTransactionMsg(int flags, const unsigned char* msgData, int msgLen) {
  TRACE9("received " << msgLen << " bytes");
  //print_data(msgData, msgLen);

  ss_.write(reinterpret_cast<const char *>(msgData), msgLen);
  if(flags & MSG_FLAG_MORE_FRAG) return;

  String xactionData = ss_.str();
  size_t pos = 0, nextPos;

  SPtr<ObexTransaction> xaction = CreateObject<ObexTransaction>();

  while(true) {
    if(pos >= xactionData.length()) break;

    YASSERT(xactionData.at(pos) == '{', "Not starting with {");
    pos++;
  
    nextPos = xactionData.find(',', pos);
    String op = xactionData.substr(pos, nextPos - pos);
  
    if(op.compare("put") == 0) {
        pos = nextPos + 1; nextPos = xactionData.find(',', pos);
        int len = atoi(MkCString(xactionData.substr(pos, nextPos - pos)));
  
        pos = nextPos + 1; nextPos = xactionData.find(',', pos);
        String path = xactionData.substr(pos, nextPos - pos);
  
        pos = nextPos + 1; nextPos = xactionData.find(',', pos);
        String typeName = xactionData.substr(pos, nextPos - pos);
  
        pos = nextPos + 1; nextPos = xactionData.find('=', pos);
        String opt = xactionData.substr(pos, nextPos - pos);
        bool delOnDisconnect =
            (opt.compare(PUT_OBJECT_OPT_DEL_ON_DISCONNECT) == 0);
  
        pos = nextPos + 1; nextPos = pos + len;
        YASSERT(xactionData.at(nextPos) == '}', "Not ending with }");
        String objData = xactionData.substr(pos, nextPos - pos);
  
        SPtr<SerializedObject> serObj =
            CreateObject<SerializedObject>(typeName, objData);
        SPtr<ObexObject> oObj;
        if(obexTree_->fwdOnly() && (path.compare(0, 6, "/proc/") != 0)) {
            oObj = CreateObject<ObexObject>(serObj);
        } else {
            oObj = ObexObjectFactory::instantiate(typeName, serObj);
        }

        TRACET(sessionName_ << " " << typeName << " "  << path << " " << serObj->data());

        TRACE9("received: {put," << len << "," << path << "," <<
                typeName << "," << delOnDisconnect << "}");
        xaction->putObject(path, oObj, delOnDisconnect);

    } else if(op.compare("del") == 0) {
        pos = nextPos + 1; nextPos = xactionData.find('}', pos);
        YASSERT(xactionData.at(nextPos) == '}', "Not ending with }");
        String path = xactionData.substr(pos, nextPos - pos);
        TRACET(sessionName_ << " del "  << path << "\n");
        TRACE9("received: {del," << path << "}");
        xaction->delObject(path);
    } else {
        YASSERT(false, "Unknown op " << op);
    }

    pos = nextPos + 1;
  }

  handleTransaction(xaction);

  objInfo_ = "";
  ss_.str("");
}

SPtr<SerializedMsg>
PutObjectMsg::makeSerializedMsg() {
  SPtr<SerializedMsg> sMsg = CreateObject<SerializedMsg>(msgType_, false);

  Stringstream ss;
  ss << path_ << "," << serObj_->typeName();
  if(!opt_.empty()) {
    ss << "," << opt_;
  }
  ss << "=" << serObj_->data();
  sMsg->setData(ss.str());

  return sMsg;
}

SPtr<SerializedMsg>
TransactionMsg::makeSerializedMsg() {
  TRACE9("msgType_ " << msgType_);
  SPtr<SerializedMsg> sMsg = CreateObject<SerializedMsg>(msgType_, false);
  Stringstream ss;

  List<SPtr<ObexTransactionElement>>& xlist = xaction_->getElementList();
  List<SPtr<ObexTransactionElement>>::iterator iter;
  for(iter = xlist.begin(); iter != xlist.end(); ++iter) {
      SPtr<ObexTransactionElement> x = *iter;
      if(x->op() == ObexTransactionElement::XOP_DEL) {
          ss << "{del," << x->path() << "}";
      } else {
          SPtr<SerializedObject> serObj = x->obj()->getSerializedObject();
          ss << "{put"
             << "," << serObj->data().size()
             << "," << x->path()
             << "," << serObj->typeName()
             << "," << x->opt()
             << "=" << serObj->data() << "}";
      }
  }
  sMsg->setData(ss.str());

  return sMsg;
}

void
ObexTransaction::putObject(String objPath, SPtr<ObexObject> obj, bool dod) {
    assert(xMap_.find(objPath) == xMap_.end());
    SPtr<ObexTransactionElement> x = CreateObject<ObexTransactionElement>(
            ObexTransactionElement::XOP_PUT, objPath, obj,
            dod ? PUT_OBJECT_OPT_DEL_ON_DISCONNECT : "");
    xList_.push_back(x);
    xMap_.insert(Pair<String, SPtr<ObexTransactionElement>>(objPath, x));
}

void
ObexTransaction::delObject(String objPath) {
    assert(xMap_.find(objPath) == xMap_.end());
    SPtr<ObexTransactionElement> x = CreateObject<ObexTransactionElement>(
            ObexTransactionElement::XOP_DEL, objPath, nullPtr(ObexObject), "");
    xList_.push_back(x);
    xMap_.insert(Pair<String, SPtr<ObexTransactionElement>>(objPath, x));
}

}

