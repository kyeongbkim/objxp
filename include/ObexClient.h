#ifndef __OBEX_CLIENT_H__
#define __OBEX_CLIENT_H__

#include <map>

#include <MsgIo.h>
#include <SubscriptionCallback.h>

namespace Yail {

typedef Map<String, String> SessionSummaries;
class ObexClient;

YAIL_BEGIN_CLASS(SubscribeMsg, EXTENDS(MsgInfo))
  public:
    void init(String paths) {
      msgType_ = MSG_TYPE_SUBSCRIBE;
      paths_ = paths;
    }
    SPtr<SerializedMsg> makeSerializedMsg() override {
      SPtr<SerializedMsg> sMsg = CreateObject<SerializedMsg>(msgType_);
      sMsg->setData(paths_);
      return sMsg;
    }
  private:
    String paths_;
YAIL_END_CLASS

YAIL_BEGIN_INTERFACE(ObexConnectionCallback)
  virtual void onConnected(String sessionKey) = 0;
  virtual void onConnectionFailed(String sessionKey) = 0;
  virtual void onDisconnected(String sessionKey) = 0;
YAIL_END_INTERFACE

YAIL_BEGIN_CLASS(DefaultConnectionCallback,
                 EXTENDS(YObject), IMPLEMENTS(ObexConnectionCallback))
  public:
    void init() {}
    void onConnected(String sessionKey) override {
      TRACE9("connection succeeded, " << sessionKey);
    }
    void onConnectionFailed(String sessionKey) override {
      TRACE9("connection failed, " << sessionKey);
    }
    void onDisconnected(String sessionKey) override {
      TRACE9("connection lost, " << sessionKey);
    }
YAIL_END_CLASS

YAIL_BEGIN_INTERFACE(ObexClientSession)
  virtual String getSessionKey() = 0;
  virtual String getSessionName() = 0;
  virtual String getPeerName() = 0;
  virtual SPtr<ObexObject> getObject(String objPath) = 0;
  virtual void putObject(String objPath, SPtr<ObexObject> obj, bool delOnDisconnect=false) = 0;
  virtual void delObject(String objPath) = 0;
  virtual void printTree(dumpOutFormat format=DEFAULT) = 0;
  virtual void saveTree(String filePath, dumpOutFormat format=DEFAULT) = 0;
  virtual String dumpTree(dumpOutFormat format=DEFAULT) = 0;
  virtual void subscribe(String cbPaths) = 0;
  virtual void subscribe(String cbPaths, SPtr<SubscriptionCallback> sCb) = 0;
  virtual void registerCallback(String cbPath, SPtr<ObexCallback> cbObj) = 0;
  virtual void unregisterCallback(String cbPath, SPtr<ObexCallback> cbObj) = 0;
  virtual void disconnect() = 0;
  virtual SPtr<ObexObjectMap> find(String dirPath, String matchStr,
                                   bool recursive, bool (*matchFunc)(String, String)=NULL) = 0;
  virtual void beginTransaction() = 0;
  virtual void endTransaction() = 0;
  virtual void abortTransaction() = 0;

YAIL_END_INTERFACE

YAIL_BEGIN_CLASS_EXT(ObexCohabClientSession, EXTENDS(YObject),
                                             IMPLEMENTS(ObexClientSession),
                                             IMPLEMENTS(ObexCallback))
  void __new__ObexCohabClientSession() {}
  void __del__ObexCohabClientSession();
  public:
    void init(String clientName, String sessionKey, WPtr<ObexClient> obexClient,
              SPtr<ObexTree> obexTree);

    String getSessionKey() override { return sessionKey_; }
    String getSessionName() override { return clientName_ + "-" + sessionKey_; }
    String getPeerName() override { return clientName_ + "-" + sessionKey_; }
    SPtr<ObexObject> getObject(String objPath) override;
    void putObject(String objPath, SPtr<ObexObject> obj, bool delOnDisconnect=false) override;
    void delObject(String objPath) override;

    void printTree(dumpOutFormat format=DEFAULT) { obexTree_->printTree(); }
    void saveTree(String filePath, dumpOutFormat format=DEFAULT) { obexTree_->saveTree(filePath); }
    String dumpTree(dumpOutFormat format=DEFAULT) { return obexTree_->dumpTree(); }

    void subscribe(String cbPaths, SPtr<SubscriptionCallback> sCb) override;
    void subscribe(String cbPaths) override {
      subscribe(cbPaths, nullPtr(SubscriptionCallback));
    }
    void registerCallback(String cbPath, SPtr<ObexCallback> cbObj) override;
    void unregisterCallback(String cbPath, SPtr<ObexCallback> cbObj) override;
    void disconnect() override;
    SPtr<ObexObjectMap> find(String dirPath, String matchStr,
                             bool recursive, bool (*matchFunc)(String, String)=NULL) override;
    void beginTransaction() override {}
    void endTransaction() override {}
    void abortTransaction() override {}

    // ObexCallback implementation
    String callbackName() override { return getSessionName(); }
    void onUpdated(String cbSrc, String objPath,
        SPtr<ObexObject> newObj, SPtr<ObexObject> oldObj) override;
    void onDeleted(String cbSrc, String objPath,
        SPtr<ObexObject> oldObj) override;

  private:
    String clientName_;
    String sessionKey_;
    SPtr<ObexTree> obexTree_;
    WPtr<ObexClient> obexClient_;

    ObexTree::PathSet subscribedPaths_;
    Map<String, ObexMetaNode::ObexCallbackMap_t> cbMap_;

    void getCallbackSet(ObexMetaNode::ObexCallbackSet_t& cbSet, String objPath);
    void forwardUpdateCallback(String cbSrc, String objPath,
        SPtr<ObexObject> newObj, SPtr<ObexObject> oldObj);
    void forwardDeleteCallback(String cbSrc, String objPath,
        SPtr<ObexObject> oldObj);
YAIL_END_CLASS

YAIL_BEGIN_CLASS(ObexRemoteClientSession, EXTENDS(MsgIoSession),
                                          IMPLEMENTS(ObexClientSession),
                                          IMPLEMENTS(ObexCallback))
  public:
    void init(String clientName, String sessionKey,
              int fd, SPtr<IoService> ioSvc,
	      SPtr<ConnectionCallback> connectionCallback);

    String getSessionKey() override;
    String getSessionName() override;
    String getPeerName() override;
    SPtr<ObexObject> getObject(String objPath) override;
    void putObject(String objPath, SPtr<ObexObject> obj, bool delOnDisconnect=false) override;
    void delObject(String objPath) override;

    void printTree(dumpOutFormat format=DEFAULT) { obexTree_->printTree(); }
    void saveTree(String filePath, dumpOutFormat format=DEFAULT) { obexTree_->saveTree(filePath); }
    String dumpTree(dumpOutFormat format=DEFAULT) { return obexTree_->dumpTree(); }

    void subscribe(String cbPaths, SPtr<SubscriptionCallback> sCb) override;
    void subscribe(String cbPaths) override {
      subscribe(cbPaths, nullPtr(SubscriptionCallback));
    }
    void registerCallback(String cbPath, SPtr<ObexCallback> cbObj) override;
    void unregisterCallback(String cbPath, SPtr<ObexCallback> cbObj) override;
    void disconnect() override;
    SPtr<ObexObjectMap> find(String dirPath, String matchStr,
                             bool recursive, bool (*matchFunc)(String, String)=NULL) override;
    void beginTransaction() override;
    void endTransaction() override;
    void abortTransaction() override;

    void setKeepalive(int idle, int interval, int cnt);

  protected:
    void msgReceived(int type, int flags, const unsigned char* msgData, int msgLen) override;
    void enqMsg(SPtr<MsgInfo> msgInfo) override;
    void handleTransaction(SPtr<ObexTransaction> xaction) override;

  private:
    String sessionKey_;

    WPtr<SubscriptionCallback> subscriptionCallback_;
    // ObexCallback implementation for subscription completion event
    String callbackName() override { return className(); }
    void onUpdated(String cbSrc, String path, SPtr<ObexObject> newObj,
                   SPtr<ObexObject> oldObj) override;
    void onDeleted(String cbSrc, String path,
                   SPtr<ObexObject> oldObj) override {}
    SPtr<ObexTransaction> xaction_;
YAIL_END_CLASS

class SocketConnector;
class ObexServer;

YAIL_BEGIN_CLASS(ObexClient, EXTENDS(YObject), IMPLEMENTS(ConnectionCallback))
  public:
    void init(String clientName, SPtr<IoService> ioSvc,
              SPtr<ObexConnectionCallback> connectionCallback);

    static const String SESSION_NOT_FOUND;
    static const String SESSION_CONNECTING;
    static const String SESSION_CONNECTED;

    void connect(String sessionKey, String dest, int retryPeriod, int retryLimit);
    void connect(String sessionKey, SPtr<ObexServer> obexServer);
    void disconnect(String sessionKey);

    SessionSummaries getSessionSummaries();
    SPtr<ObexClientSession> getSession(String sessionKey);
    String getSessionStatus(String sessionKey);

    // ConnectionCallback
    void onConnected(String sessionName, int fd) override;
    void onConnectionFailed(String sessionName) override;
    void onDisconnected(String sessionName, int fd) override;

  private:
    String clientName_;
    Map<String, SPtr<ObexClientSession>> sessions_;
    Map<String, SPtr<SocketConnector>> connectors_;
    WPtr<ObexConnectionCallback> connectionCallback_;
    SPtr<IoService> ioSvc_;
YAIL_END_CLASS

}

#endif

