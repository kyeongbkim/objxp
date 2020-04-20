#ifndef __MSG_IO_H__
#define __MSG_IO_H__

#include <list>
#include <queue>

#include <ObexObject.h>
#include <ObexTree.h>
#include <IoObject.h>
#include <ConnectionCallback.h>

namespace Yail {

static const int MSG_TYPE_UNDEFINED    = 0;
static const int MSG_TYPE_PEER_INFO    = '0';
static const int MSG_TYPE_PUT_OBJECT   = '1';
static const int MSG_TYPE_DEL_OBJECT   = '2';
static const int MSG_TYPE_SUBSCRIBE    = '3';
static const int MSG_TYPE_TRANSACTION  = '4';

#define PUT_OBJECT_OPT_DEL_ON_DISCONNECT "dod"

YAIL_BEGIN_CLASS(SerializedMsg, EXTENDS(YObject))
  public:
    void init(int msgType, bool dontFrag=true) {
      msgType_ = msgType;
      dontFrag_ = dontFrag;
    }

    int msgType() { return msgType_; }
    int dontFrag() { return dontFrag_; }
    bool empty() { return sData_.empty(); }
    int dataLen() { return sData_.length(); }
    void setData(String data) { sData_ = data; }
    const unsigned char* dataPtr(int offset) {
      return reinterpret_cast<const unsigned char *>(sData_.c_str()) + offset;
    }

  private:
    int msgType_;
    bool dontFrag_;
    String sData_;
YAIL_END_CLASS

YAIL_BEGIN_CLASS(MsgInfo, EXTENDS(YObject))
  public:
    void init() {}
    virtual SPtr<SerializedMsg> makeSerializedMsg() = 0;
    int msgType() { return msgType_; }

  protected:
    int msgType_;
};

YAIL_BEGIN_CLASS(PeerInfoMsg, EXTENDS(MsgInfo))
  public:
    void init(String peerInfo) {
      msgType_ = MSG_TYPE_PEER_INFO;
      peerInfo_ = peerInfo;
    }

    SPtr<SerializedMsg> makeSerializedMsg() override {
      SPtr<SerializedMsg> sMsg = CreateObject<SerializedMsg>(msgType_);
      sMsg->setData(peerInfo_);
      return sMsg;
    }

  protected:
    String peerInfo_;
YAIL_END_CLASS

YAIL_BEGIN_CLASS(PutObjectMsg, EXTENDS(MsgInfo))
  public:
    void init(String path, SPtr<ObexObject> oObj, String opt="") {
      msgType_ = MSG_TYPE_PUT_OBJECT;
      path_ = path;
      serObj_ = oObj->getSerializedObject();
      opt_ = opt;
    }

    String path() { return path_; }
    SPtr<SerializedObject> serObj() { return serObj_; }

    SPtr<SerializedMsg> makeSerializedMsg() override;

  private:
    String path_;
    String opt_; // putObject option
    SPtr<SerializedObject> serObj_;
YAIL_END_CLASS

YAIL_BEGIN_CLASS(DelObjectMsg, EXTENDS(MsgInfo))
  public:
    void init(String path) {
      msgType_ = MSG_TYPE_DEL_OBJECT;
      path_ = path;
    }

    String path() { return path_; }

    SPtr<SerializedMsg> makeSerializedMsg() override {
      SPtr<SerializedMsg> sMsg = CreateObject<SerializedMsg>(msgType_);
      sMsg->setData(path_);
      return sMsg;
    }

  private:
    String path_;
YAIL_END_CLASS

YAIL_BEGIN_CLASS(TransactionMsg, EXTENDS(MsgInfo))
  public:
    void init(SPtr<ObexTransaction> xaction) {
        msgType_ = MSG_TYPE_TRANSACTION;
        xaction_ = xaction;
    }
    SPtr<SerializedMsg> makeSerializedMsg() override;

  private:
    SPtr<ObexTransaction> xaction_;
YAIL_END_CLASS


struct TxTracker {
  TxTracker(String id, int len) : id(id), len(len) {}
  String id;
  int len;
};

YAIL_BEGIN_CLASS_EXT(MsgIoSession, EXTENDS(IoObject))
  void __new__MsgIoSession() {
    txHead_ = 0; txTail_ = 0; txMsgHead_ = 0;
    rxHead_ = 0; rxTail_ = 0;
  }
  void __del__MsgIoSession() {}
  public:
    void init(String sessionName, int fd, SPtr<IoService> ioSvc,
              SPtr<ConnectionCallback> connectionCallback);

    String sessionName() { return sessionName_; }
    String peerName() { return peerName_; }

#if 0 // for c++11 or later
    static const int MSG_FLAG_NONE = 0;
    static const int MSG_FLAG_MORE_FRAG = 0x80;
#else
#define MSG_FLAG_NONE 0
#define MSG_FLAG_MORE_FRAG 0x80
#endif

    virtual void disconnectSession();

  private:
    String sessionName_;
    WPtr<ConnectionCallback> connectionCallback_;

    String peerName_;
    static const int MaxMsgLen = 0xFFFF;
    static const int HdrLen = 5;
    static const int FifoSize = HdrLen + MaxMsgLen;
    static const unsigned char Sig = 'Y';

    void handleRx();
    void handleTx();
    void handleIo(bool rdReady, bool wrReady) override;

    // Serialized Msg
    SPtr<SerializedMsg> sMsg_;
    int sMsgTxPos_;

    // Tx Fifo
    unsigned char txFifo_[FifoSize];
    int txHead_;
    int txTail_;
    int txMsgHead_;
    Queue<TxTracker> txTrackers_;

    // Rx Fifo
    unsigned char rxFifo_[FifoSize];
    int rxHead_;
    int rxTail_;

    int sendMsg(int type, int flags,
                const unsigned char* msgData, int msgLen, String id="");

  protected:
    static const int FragmentThreshold = MaxMsgLen / 4;
    static const int DataCopyThreshold = MaxMsgLen / 4;
    int txFifoRoom() {
      int bufRoom = FifoSize - txTail_ - HdrLen;
      return (bufRoom > 0) ? bufRoom : 0;
    }
    int txCopyMoveSize() {
      return txTail_ - txMsgHead_;
    }
    virtual void msgSent(String id) {
      /* TBD */
      TRACE9(sessionName_ << ": " << "msg " << id << " sent");
    }
    virtual void msgReceived(int type, int flags, const unsigned char* msgData, int msgLen );
    SPtr<ObexTree> obexTree_;
    Stringstream ss_;
    String objInfo_;
    void handlePutObjectMsg(int flags, const unsigned char* msgData, int msgLen);
    void handleTransactionMsg(int flags, const unsigned char* msgData, int msgLen);
    virtual void handlePutObject(String path, SPtr<ObexObject> obj, String opt) {
        obexTree_->putObject(path, obj);
    }
    virtual void handleDelObject(String path) { obexTree_->delObject(path); }
    virtual void handleTransaction(SPtr<ObexTransaction> xaction) = 0;

    // Tx Msg List
    List<SPtr<MsgInfo>> txMsgList_;
    virtual void enqMsg(SPtr<MsgInfo> msgInfo);
    virtual SPtr<MsgInfo> deqMsg();
    void sendData();

  private:
    void enqPutObjectMsg(SPtr<PutObjectMsg> putObjMsg);
    void enqDelObjectMsg(SPtr<DelObjectMsg> delObjMsg);
YAIL_END_CLASS

}

#endif

