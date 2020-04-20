#include <IoService.h>
#include <ObexTree.h>
#include <ObexInfoProvider.h>
#include <ObexServer.h>
#include <UnixSocketAcceptor.h>
#include <TcpSocketAcceptor.h>
#include <ObexNumberObject.h>
#include <ObexStringObject.h>
#include <YUtils.h>
#include <YPathProc.h>

namespace Yail {

void
ObexServerSession::init(String serverName, String sessionName,
                        int fd, SPtr<ObexTree> obexTree,
                        SPtr<IoService> ioSvc,
                        SPtr<ObexServer> obexServer,
                        unsigned int msgListThreshold,
                        int msgTimeout) {
  obexServer_ = obexServer;

  obexTree_ = obexTree;
  MsgIoSession::init(sessionName, fd, ioSvc, obexServer);

  String peerInfo = MkString(serverName << "-" << fd);
  SPtr<PeerInfoMsg> peerInfoMsg = CreateObject<PeerInfoMsg>(peerInfo);
  enqMsg(peerInfoMsg);

  SPtr<ObexBoolObject> boolObj = CreateObject<ObexBoolObject>(true);
  obexTree_->putObject(
      MkYPath(YpProcSession__SID, MkYPathArg("SID", sessionName)),
      boolObj);
  msgListThreshold_ = msgListThreshold;
  msgTimeout_ = msgTimeout;
}

void
ObexServerSession::__del__ObexServerSession() {
  obexTree_->delObject(
      MkYPath(YpProcSession__SID, MkYPathArg("SID", sessionName())));
  obexTree_->delObject(
      MkYPath(YpProcSession__SID__Cmd, MkYPathArg("SID", sessionName())));
  obexTree_->delObject(
      MkYPath(YpProcSession__SID__Rsp, MkYPathArg("SID", sessionName())));

  Map<String, WPtr<ObexObject>>::iterator it;
  for(it = dodMap_.begin(); it != dodMap_.end(); ++it) {
    USE_WPTR(ObexObject, oObj, it->second, {},
    {
      String path = it->first;
      TRACE9("deleting " << path);
      obexTree_->delObject(path);
    }, {}, {});
  }
}

void
ObexServerSession::disconnectSession() {
  TRACE9("sessionName: " << sessionName());

  SPtr<ObexServerSession> thisPtr = getThisPtr<ObexServerSession>();
  for (ObexTree::PathSet::iterator iter = cbPaths_.begin(); iter != cbPaths_.end(); iter++) {
    TRACE9("unregisterCallback:" << *iter);
    obexTree_->unregisterCallback(*iter, thisPtr);
  }

  MsgIoSession::disconnectSession();
}

void
ObexServerSession::msgReceived(int type, int flags, const unsigned char* msgData, int msgLen ) {
  TRACE9(TARG("type", type) << TARG(", flags", flags));

  switch(type) {
    case MSG_TYPE_SUBSCRIBE: {
      String paths(reinterpret_cast<char const*>(msgData), msgLen);
      TRACE9("subscribe:" << paths);
      handleSubscribeMsg(paths);
      break;
    }
    default: {
      MsgIoSession::msgReceived(type, flags, msgData, msgLen);
      break;
    }
  }
}

void
ObexServerSession::handleSubscribeMsg(String paths) {
  ObexTree::PathSet syncObjs;
  Stringstream ss(paths);
  String path;
  while(getline(ss, path, ',')) {
    TRACE9("path: " << path);
    SPtr<ObexServerSession> thisPtr = getThisPtr<ObexServerSession>();
    obexTree_->registerCallbackAndGetObjSet(path, DynamicPointerCast<ObexCallback>(thisPtr), &syncObjs);
    cbPaths_.insert(path);
  }

  for (ObexTree::PathSet::iterator iter = syncObjs.begin(); iter != syncObjs.end(); iter++) {
    String objPath = *iter;
    TRACE9("sync initial obj:" << objPath);

    SPtr<ObexObject> oObj = obexTree_->getObject(objPath);
    if(!oObj) continue;
    SPtr<PutObjectMsg> putObjMsg = CreateObject<PutObjectMsg>(objPath, oObj);
    enqMsg(putObjMsg);
  }

  // notify end of subscription
  SPtr<ObexStringObject> notiObj = CreateObject<ObexStringObject>(paths);
  SPtr<PutObjectMsg> notiMsg =
    CreateObject<PutObjectMsg>(YpProcLocalSubscription, notiObj);
  enqMsg(notiMsg);

  sendData();
}

void
ObexServerSession::handlePutObject(String path, SPtr<ObexObject> obj, String opt) {
  TRACE9("path " << path << ", opt " << opt);
  MsgIoSession::handlePutObject(path, obj, opt);

  if(opt.compare(PUT_OBJECT_OPT_DEL_ON_DISCONNECT) == 0) {
    TRACE9("register delOnDisconnect for " << path);

    // replace if already exists
    Map<String, WPtr<ObexObject>>::iterator it = dodMap_.find(path);
    if(it != dodMap_.end()) {
        dodMap_.erase(it);
    }
    dodMap_.insert(Pair<String, WPtr<ObexObject>>(path, obj));

    // clean-up already deleted objects
    for(it = dodMap_.begin(); it != dodMap_.end(); ) {
      bool doDelete = false;
      USE_WPTR(ObexObject, oObj, it->second, {},
      {}, { doDelete = true; }, {});

      if(doDelete) {
        dodMap_.erase(it++);
      } else {
        ++it;
      }
    }
  }
}

void
ObexServerSession::handleTransaction(SPtr<ObexTransaction> xaction) {
    TRACE9("");

    SPtr<ObexServer> obexServer;
    USE_WPTR(ObexServer, os, obexServer_,
            {},
            {
              obexServer = os;
            },
            {}, {});

    obexServer->beginTransaction();

    List<SPtr<ObexTransactionElement>> xlist = xaction->getElementList();
    List<SPtr<ObexTransactionElement>>::iterator iter;
    for(iter = xlist.begin(); iter != xlist.end(); ++iter) {
        SPtr<ObexTransactionElement> x = *iter;
        if(x->op() == ObexTransactionElement::XOP_PUT) {
            handlePutObject(x->path(), x->obj(), x->opt());
        } else if(x->op() == ObexTransactionElement::XOP_DEL) {
            handleDelObject(x->path());
        } else {
            YASSERT(false, "Unknown op " << x->op());
        }
    }

    obexServer->endTransaction();
}

void
ObexServerSession::onUpdated(String cbSrc, String path,
    SPtr<ObexObject> newObj, SPtr<ObexObject> oldObj) {

  SPtr<ObexServer> obexServer;
  USE_WPTR(ObexServer, os, obexServer_,
          {},
          {
            obexServer = os;
          },
          {}, {});

  if(obexServer->transactionInProgress()) {
    if(!xaction_) {
      xaction_ = CreateObject<ObexTransaction>();
      obexServer->addTransactionSession(getThisPtr<ObexServerSession>());
    }
    xaction_->putObject(path, newObj);
  } else {
    SPtr<PutObjectMsg> putObjMsg = CreateObject<PutObjectMsg>(path, newObj);
    enqMsg(putObjMsg);
  }
}

void
ObexServerSession::onDeleted(String cbSrc, String path,
    SPtr<ObexObject> oldObj) {

  SPtr<ObexServer> obexServer;
  USE_WPTR(ObexServer, os, obexServer_,
          {},
          {
            obexServer = os;
          },
          {}, {});

  if(obexServer->transactionInProgress()) {
    if(!xaction_) {
      xaction_ = CreateObject<ObexTransaction>();
      obexServer->addTransactionSession(getThisPtr<ObexServerSession>());
    }
    xaction_->delObject(path);
  } else {
    SPtr<DelObjectMsg> delObjMsg = CreateObject<DelObjectMsg>(path);
    enqMsg(delObjMsg);
  }
}

void
ObexServerSession::enqMsg(SPtr<MsgInfo> msgInfo) {
  if(txMsgList_.size() > msgListThreshold_) {
    Time now = Time::Now();
    Time elapsedTime = now - lastDeq_;
    if(elapsedTime.sec >= msgTimeout_) {
      TRACE5("ObexClient " << sessionName() << " is not responding, " <<
             "msgCnt:" << txMsgList_.size() << ", " <<
             "elapsedTime:" << elapsedTime.sec << "s " << elapsedTime.nsec);
      disconnectSession();
      return;
    }
  }
  MsgIoSession::enqMsg(msgInfo);
}

SPtr<MsgInfo>
ObexServerSession::deqMsg() {
  lastDeq_ = Time::Now();
  return MsgIoSession::deqMsg();
}

void
ObexServerSession::finishTransaction() {
  if(xaction_ && !xaction_->empty()) {
    SPtr<TransactionMsg> xactionMsg = CreateObject<TransactionMsg>(xaction_);
    enqMsg(xactionMsg);
  }
  xaction_ = nullPtr(ObexTransaction);
}

/**********************************************************************************/

void
ObexServer::onConnected(String sessionName, int fd) {
  SPtr<ObexServer> thisPtr = getThisPtr<ObexServer>();
  SPtr<ObexServerSession> session = CreateObject<ObexServerSession>(
      serverName_, sessionName, fd, obexTree_, ioSvc_, thisPtr, msgListThreshold_, msgTimeout_);
  sessions_.insert(Pair<int, SPtr<ObexServerSession>>(fd, session));
}

void
ObexServer::onDisconnected(String sessionName, int fd) {
  Map<int, SPtr<ObexServerSession>>::iterator sessionIter = sessions_.find(fd);
  assert(sessionIter != sessions_.end());
  sessions_.erase(sessionIter);
}

void
ObexServer::init(String serverName, String sockName, int svcPort, SPtr<IoService> ioSvc, bool fwdOnly, unsigned int msgListThreshold, int msgTimeout) {
  serverName_ = serverName;
  ioSvc_ = ioSvc;
  obexTree_ = CreateObject<ObexTree>(serverName_, fwdOnly);
  infoProvider_ = CreateObject<ObexInfoProvider>(obexTree_);
  msgListThreshold_ = msgListThreshold;
  msgTimeout_ = msgTimeout;

  SPtr<ObexServer> thisPtr = getThisPtr<ObexServer>();

  if(!sockName.empty()) {
    unixSockAcceptor_ =
      CreateObject<UnixSocketAcceptor>(serverName_, sockName, ioSvc, thisPtr);
  }
  if(svcPort > 0) {
    tcpSockAcceptor_ =
      CreateObject<TcpSocketAcceptor>(serverName_, svcPort, ioSvc, thisPtr);
  }
}

SessionSummaries
ObexServer::getSessionSummaries() {
  SessionSummaries summaries;

  for(Map<int, SPtr<ObexServerSession>>::iterator it = sessions_.begin(); it != sessions_.end(); ++it) {
    SPtr<ObexServerSession> serverSession = it->second;
    summaries.insert(Pair<String, String>(serverSession->sessionName(),
                     "Connected, Peer: " + serverSession->peerName()));
  }

  return summaries;
}

void
ObexServer::beginTransaction() {
  YASSERT(xSessionList_.empty(), "xSessionList_ is not empty");
  xactionInProgress_ = true;
}

void
ObexServer::endTransaction() {
  List<WPtr<ObexServerSession>>::iterator iter;
  for(iter = xSessionList_.begin(); iter != xSessionList_.end(); ++iter) {
    USE_WPTR(ObexServerSession, oss, *iter,
      {}, {
        oss->finishTransaction();
      }, {}, {});
  }

  xSessionList_.clear();
  xactionInProgress_ = false;
}

}

