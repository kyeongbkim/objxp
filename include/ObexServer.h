#ifndef __OBEX_SERVER_H__
#define __OBEX_SERVER_H__

#include <map>
#include <MsgIo.h>
#include <ObexTree.h>
#include <TimerObject.h>

namespace Yail {

class TcpSocketAcceptor;
class UnixSocketAcceptor;
class ObexServer;
class ObexServerSession;
class ObexInfoProvider;

typedef Map<String, String> SessionSummaries;

YAIL_BEGIN_CLASS(ObjectCallbackMsg, EXTENDS(MsgInfo))
  public:
    void init(String path, ObexTree::OperationType oper, SPtr<ObexObject> oObj) {
      path_ = path;
      oper_ = oper;
      oObj_ = oObj;
    }
    String path() { return path_; }
    SPtr<SerializedMsg> makeSerializedMsg() override {
      SPtr<SerializedMsg> sMsg = CreateObject<SerializedMsg>(msgType_, false);
      sMsg->setData(path_);
      return sMsg;
    }

  private:
    String path_;
    ObexTree::OperationType oper_;
    SPtr<ObexObject> oObj_;
YAIL_END_CLASS

YAIL_BEGIN_CLASS_EXT(ObexServerSession, EXTENDS(MsgIoSession),
                                        IMPLEMENTS(ObexCallback))
  void __new__ObexServerSession() { lastDeq_ = Time::Now(); }
  void __del__ObexServerSession();
  public:
    void init(String serverName, String sessionName,
              int fd, SPtr<ObexTree> obexTree,
              SPtr<IoService> ioSvc,
              SPtr<ObexServer> obexServer, unsigned int msgLIstThreshold=100, int msgTimeout=10);

    // ObexCallback implementation
    String callbackName() override { return className() + "." + sessionName(); }
    void onUpdated(String cbSrc, String path,
        SPtr<ObexObject> newObj, SPtr<ObexObject> oldObj) override;
    void onDeleted(String cbSrc, String path,
        SPtr<ObexObject> oldObj) override;

    // from MsgIo override
    void disconnectSession() override;

    void finishTransaction();

  protected:
    void msgReceived(int type, int flags, const unsigned char* msgData, int msgLen ) override;
    void enqMsg(SPtr<MsgInfo> msgInfo) override;
    SPtr<MsgInfo> deqMsg() override;

  private:
    WPtr<ObexServer> obexServer_;
    ObexTree::PathSet cbPaths_;
    Map<String, WPtr<ObexObject>> dodMap_; // delOnDisconnectMap
    Time lastDeq_;
    unsigned int msgListThreshold_;
    int msgTimeout_;  // in seconds

    void handleSubscribeMsg(String paths);

    void handlePutObject(String path, SPtr<ObexObject> oObj,
                         String opt) override;
    void handleTransaction(SPtr<ObexTransaction> xaction) override;
    SPtr<ObexTransaction> xaction_;

YAIL_END_CLASS

YAIL_BEGIN_CLASS_EXT(ObexServer, EXTENDS(YObject),
                                 IMPLEMENTS(ConnectionCallback))
  void __new__ObexServer() { xactionInProgress_ = false; }
  void __del__ObexServer() {}
  public:
    void init(String serverName, String sockName, int svcPort, SPtr<IoService> ioSvc, bool fwdOnly=false, unsigned int msgListThreshold=100, int msgTimeout=10);

    SPtr<ObexTree> getObexTree() { return obexTree_; }
    SessionSummaries getSessionSummaries();
    SPtr<ObexServerSession> getSession(int fd) {
      Map<int, SPtr<ObexServerSession>>::iterator sessionIter = sessions_.find(fd);
      if(sessionIter != sessions_.end()) {
        return sessionIter->second;
      }
      return nullPtr(ObexServerSession);
    }

    void onConnected(String sessionName, int fd) override;
    void onConnectionFailed(String sessionName) override {}
    void onDisconnected(String sessionName, int fd) override;

    void beginTransaction();
    void endTransaction();
    bool transactionInProgress() {
        return xactionInProgress_;
    }
    void addTransactionSession(SPtr<ObexServerSession> s) {
        xSessionList_.push_back(s);
    }

  private:
    String serverName_;
    SPtr<ObexTree> obexTree_;
    Map<int, SPtr<ObexServerSession>> sessions_;
    SPtr<TcpSocketAcceptor> tcpSockAcceptor_;
    SPtr<UnixSocketAcceptor> unixSockAcceptor_;
    SPtr<ObexInfoProvider> infoProvider_;
    SPtr<IoService> ioSvc_;
    bool xactionInProgress_;
    List<WPtr<ObexServerSession>> xSessionList_;
    unsigned int msgListThreshold_;
    int msgTimeout_;  // in seconds

YAIL_END_CLASS

}

#endif

