#include <IoService.h>
#include <UnixSocketConnector.h>
#include <TcpSocketConnector.h>
#include <ObexNumberObject.h>
#include <ObexStringObject.h>
#include <ObexClient.h>
#include <ObexServer.h>
#include <ObexJsonObject.h>
#include <YPathProc.h>
#include <YUtils.h>
#include <sys/socket.h>
#include <cerrno>

namespace Yail {

const String ObexClient::SESSION_NOT_FOUND  = "NOT_FOUND";
const String ObexClient::SESSION_CONNECTING = "CONNECTING";
const String ObexClient::SESSION_CONNECTED  = "CONNECTED";

void
ObexClient::init(String clientName, SPtr<IoService> ioSvc,
                 SPtr<ObexConnectionCallback> connectionCallback) {
  clientName_ = clientName;
  connectionCallback_ = connectionCallback;
  ioSvc_ = ioSvc;
}

void
ObexClient::onConnected(String sessionName, int fd) {
  String sessionKey = sessionName;
  TRACE9(clientName_ << "." << sessionName << " connected");

  SPtr<ObexClient> thisPtr = getThisPtr<ObexClient>();
  SPtr<ObexClientSession> session = CreateObject<ObexRemoteClientSession>(
        clientName_, sessionKey, fd, ioSvc_, thisPtr);
  sessions_.insert(Pair<String, SPtr<ObexClientSession>>(
        sessionKey, session));
  assert(connectors_.find(sessionKey) != connectors_.end());
  connectors_.erase(sessionKey);
  USE_WPTR(ObexConnectionCallback, cb, connectionCallback_,
      {},
      { cb->onConnected(sessionKey); },
      {}, {});
}

void
ObexClient::onConnectionFailed(String sessionName) {
  String sessionKey = sessionName;
  assert(connectors_.find(sessionKey) != connectors_.end());
  connectors_.erase(sessionKey);
  USE_WPTR(ObexConnectionCallback, cb, connectionCallback_,
      {},
      { cb->onConnectionFailed(sessionKey); },
      {}, {});
}

void
ObexClient::onDisconnected(String sessionName, int fd) {
  String sessionKey;
  TRACE9("sessionName: " << sessionName);

  Map<String, SPtr<ObexClientSession>>::iterator iter;
  
  for(iter = sessions_.begin(); iter != sessions_.end(); ++iter) {
    SPtr<ObexClientSession> clientSession = iter->second;
    if(clientSession->getSessionName() == sessionName) {
      sessionKey = clientSession->getSessionKey();
      break;
    }
  }

  if(iter != sessions_.end()) {
    if(fd >= 0) {
      USE_WPTR(ObexConnectionCallback, cb, connectionCallback_,
          {},
          { cb->onDisconnected(sessionKey); },
          {}, {});
    }
    sessions_.erase(iter);
  }
}

void
ObexClient::connect(String sessionKey, String dest, int retryPeriod, int retryLimit) {
  if(connectors_.find(sessionKey) != connectors_.end()) {
    TRACE9(clientName_ << "." << sessionKey << " connection in progress already");
    return;
  }
  if(sessions_.find(sessionKey) != sessions_.end()) {
    TRACE9(clientName_ << "." << sessionKey << " already connected");
    return;
  }

  SPtr<ObexClient> thisPtr = getThisPtr<ObexClient>();
  SPtr<SocketConnector> connector;

  if(dest.find(':') == String::npos) {
    String sockName = dest;
    connector = CreateObject<UnixSocketConnector>(
      sessionKey, sockName, retryPeriod, retryLimit, ioSvc_, thisPtr);
  } else {
    Stringstream ss(dest);
    String host;
    int port;
    getline(ss, host, ':');
    ss >> port;
    connector = CreateObject<TcpSocketConnector>(
      sessionKey, host, port, retryPeriod, retryLimit, ioSvc_, thisPtr);
  }
  connectors_.insert(Pair<String, SPtr<SocketConnector>>(sessionKey, connector));

  assert(connectors_.find(sessionKey) != connectors_.end());
}

void
ObexClient::connect(String sessionKey, SPtr<ObexServer> cohabServer) {
  SPtr<ObexTree> obexTree = cohabServer->getObexTree();
  SPtr<ObexClientSession> session = CreateObject<ObexCohabClientSession>(
      clientName_, sessionKey, getThisPtr<ObexClient>(), obexTree);
  sessions_.insert(Pair<String, SPtr<ObexClientSession>>(
        sessionKey, session));
}

void
ObexClient::disconnect(String sessionKey) {
  Map<String, SPtr<ObexClientSession>>::iterator sessionIter = sessions_.find(sessionKey);
  if(sessionIter == sessions_.end()) {
    TRACE9(clientName_ << ": session " << sessionKey << " not found");
    return;
  }
  SPtr<ObexClientSession> clientSession = sessionIter->second;
  clientSession->disconnect();
}

SessionSummaries
ObexClient::getSessionSummaries() {
  SessionSummaries summaries;
  for(Map<String, SPtr<ObexClientSession>>::iterator it = sessions_.begin(); it != sessions_.end(); ++it) {
    SPtr<ObexClientSession> clientSession = it->second;
    String summary = MkString(
        "Connected as " << clientSession->getSessionName() <<
        ", Peer: " << clientSession->getPeerName());
    summaries.insert(Pair<String, String>(clientSession->getSessionKey(), summary));
  }
  for(Map<String, SPtr<SocketConnector>>::iterator it = connectors_.begin(); it != connectors_.end(); ++it) {
    SPtr<SocketConnector> connector = it->second;
    summaries.insert(Pair<String, String>(connector->sessionName(), "Connection in progress ..."));
  }

  return summaries;
}

SPtr<ObexClientSession>
ObexClient::getSession(String sessionKey) {
  Map<String, SPtr<ObexClientSession>>::iterator sessionIter =
    sessions_.find(sessionKey);
  if(sessionIter != sessions_.end()) {
    return sessionIter->second;
  }
  return nullPtr(ObexClientSession);
}

String
ObexClient::getSessionStatus(String sessionKey) {
  if(sessions_.find(sessionKey) != sessions_.end()) {
    return SESSION_CONNECTED;
  }
  if(connectors_.find(sessionKey) != connectors_.end()) {
    return SESSION_CONNECTING;
  }
  return SESSION_NOT_FOUND;
}


void
ObexCohabClientSession::init(
    String clientName, String sessionKey, WPtr<ObexClient> obexClient, SPtr<ObexTree> obexTree) {
  clientName_ = clientName;
  sessionKey_ = sessionKey;
  obexTree_ = obexTree;
  obexClient_ = obexClient;

  if(sessionKey != obexTree_->name()) {
    TRACE9("Session key " << sessionKey <<
           ", rename callback cbSrc: " << obexTree_->name());
  }

  SPtr<ObexBoolObject> boolObj = CreateObject<ObexBoolObject>(true);
  obexTree_->putObject(
      MkYPath(YpProcSession__SID, MkYPathArg("SID", getSessionName())),
      boolObj);
}

void
ObexCohabClientSession::__del__ObexCohabClientSession() {
  TRACE9("");
  disconnect();
  obexTree_->delObject(
      MkYPath(YpProcSession__SID, MkYPathArg("SID", getSessionName())));
}
 
SPtr<ObexObject>
ObexCohabClientSession::getObject(String objPath) {
  if(subscribedPaths_.empty()) return nullPtr(ObexObject);

  ObexTree::PathSet::iterator spIter;

  // return object within subscribed scope
  for(spIter = subscribedPaths_.begin(); spIter != subscribedPaths_.end(); ++spIter) {
    if(ObexTree::objPathMatchesCbPath(objPath, *spIter)) {
      return obexTree_->getObject(objPath);
    }
  }
  return nullPtr(ObexObject);
}

void
ObexCohabClientSession::putObject(String objPath, SPtr<ObexObject> obj, bool delOnDisconnect) {
  obexTree_->putObject(objPath, obj);
}

void
ObexCohabClientSession::delObject(String objPath) {
  obexTree_->delObject(objPath);
}

void
ObexCohabClientSession::subscribe(String cbPaths, SPtr<SubscriptionCallback> sCb) {
  ObexTree::PathSet syncObjs;
  Stringstream ss(cbPaths);
  String cbPath;
  while(getline(ss, cbPath, ',')) {
    TRACE9("cbPath: " << cbPath);
    obexTree_->registerCallbackAndGetObjSet(cbPath, getThisPtr<ObexCallback>(), &syncObjs);
    subscribedPaths_.insert(cbPath);
  }

  for(ObexTree::PathSet::iterator iter = syncObjs.begin(); iter != syncObjs.end(); iter++) {
    String objPath = *iter;
    TRACE9("sync initial obj:" << objPath);

    SPtr<ObexObject> obj = obexTree_->getObject(objPath);
    assert(obj);

    forwardUpdateCallback(obexTree_->name(), objPath, obj, nullPtr(ObexObject));
  }

  if(sCb) {
    SPtr<ObexJsonObject> jObj = CreateObject<ObexJsonObject>(
        MkString("{ \"sessionKey\":\"" << sessionKey_ << "\", " <<
                 "\"cbPaths\":\"" << cbPaths << "\" }"));
    sCb->onSubscriptionCompleted(jObj);
  }
}

void
ObexCohabClientSession::getCallbackSet(ObexMetaNode::ObexCallbackSet_t& cbSet, String objPath) {
  Map<String, ObexMetaNode::ObexCallbackMap_t>::iterator cbMapIter;
  for(cbMapIter = cbMap_.begin(); cbMapIter != cbMap_.end(); ++cbMapIter) {
    String cbPath = cbMapIter->first;
    ObexMetaNode::ObexCallbackMap_t& cbMap = cbMapIter->second;

    if(ObexTree::objPathMatchesCbPath(objPath, cbPath)) {
      ObexTree::addToCallbackSet(cbSet, cbMap);
    }
  }
}

void
ObexCohabClientSession::forwardUpdateCallback(String cbSrc, String objPath,
    SPtr<ObexObject> newObj, SPtr<ObexObject> oldObj) {
  ObexMetaNode::ObexCallbackSet_t cbSet;
  getCallbackSet(cbSet, objPath);

  for(ObexMetaNode::ObexCallbackSet_t::iterator cbIter = cbSet.begin();
      cbIter != cbSet.end(); ++cbIter) {
    SPtr<ObexCallback> cb = *cbIter;
    cb->onUpdated(cbSrc, objPath, newObj, oldObj);
  }
}

void
ObexCohabClientSession::forwardDeleteCallback(String cbSrc, String objPath,
    SPtr<ObexObject> oldObj) {
  ObexMetaNode::ObexCallbackSet_t cbSet;
  getCallbackSet(cbSet, objPath);

  for(ObexMetaNode::ObexCallbackSet_t::iterator cbIter = cbSet.begin();
      cbIter != cbSet.end(); ++cbIter) {
    SPtr<ObexCallback> cb = *cbIter;
    cb->onDeleted(cbSrc, objPath, oldObj);
  }
}

void
ObexCohabClientSession::registerCallback(
    String cbPath, SPtr<ObexCallback> cbObj) {
  if(!ObexTree::validCbPath(cbPath)) {
    TRACE9("invalid cbPath: " << cbPath);
    return;
  }

  Map<String, ObexMetaNode::ObexCallbackMap_t>::iterator cbMapIter;
  
  if((cbMapIter = cbMap_.find(cbPath)) == cbMap_.end()) {
    ObexMetaNode::ObexCallbackMap_t cbMap = ObexMetaNode::ObexCallbackMap_t();
    cbMap_.insert(Pair<String, ObexMetaNode::ObexCallbackMap_t>(cbPath, cbMap));
  }

  cbMapIter = cbMap_.find(cbPath);
  assert(cbMapIter != cbMap_.end());

  ObexMetaNode::ObexCallbackMap_t& cbMap = cbMapIter->second;
  cbMap.insert(Pair<ObexCallback*, WPtr<ObexCallback>>(cbObj.get(), cbObj));
}

void
ObexCohabClientSession::unregisterCallback(
    String cbPath, SPtr<ObexCallback> cbObj) {
  if(!ObexTree::validCbPath(cbPath)) {
    TRACE9("invalid cbPath: " << cbPath);
    return;
  }

  Map<String, ObexMetaNode::ObexCallbackMap_t>::iterator cbMapIter;
  if((cbMapIter = cbMap_.find(cbPath)) == cbMap_.end()) return;
  ObexMetaNode::ObexCallbackMap_t& cbMap = cbMapIter->second;
  cbMap.erase(cbObj.get());
}

void
ObexCohabClientSession::disconnect() {
  SPtr<ObexCallback> thisPtr = getThisPtr<ObexCallback>();
  if(thisPtr) {
    for (ObexTree::PathSet::iterator iter = subscribedPaths_.begin();
         iter != subscribedPaths_.end(); ++iter)
    {
      TRACE9("unregisterCallback:" << *iter);
      obexTree_->unregisterCallback(*iter, thisPtr);
    }
  }
  subscribedPaths_.clear();
  cbMap_.clear();

  USE_WPTR(ObexClient, obexClient, obexClient_,
    {},
    { obexClient->onDisconnected(getSessionName(), -1); },
    {},
    {}
  );
}

void
ObexCohabClientSession::onUpdated(String cbSrc, String objPath,
    SPtr<ObexObject> newObj, SPtr<ObexObject> oldObj) {
  forwardUpdateCallback(cbSrc, objPath, newObj, oldObj);
}

void
ObexCohabClientSession::onDeleted(String cbSrc, String objPath,
    SPtr<ObexObject> oldObj) {
  forwardDeleteCallback(cbSrc, objPath, oldObj);
}

SPtr<ObexObjectMap>
ObexCohabClientSession::find(String dirPath, String matchStr, bool recursive, bool (*matchFunc)(String, String)) {
  if(subscribedPaths_.empty()) return nullPtr(ObexObjectMap);
  SPtr<ObexObjectMap> objMap = obexTree_->find(dirPath, matchStr, recursive, matchFunc);
  if(!objMap) return nullPtr(ObexObjectMap);

  Map<String, SPtr<ObexObject>>::iterator it;
  for(it = objMap->map.begin(); it != objMap->map.end(); ) {
    bool removeObj = true;
    String objPath = ((dirPath == "/") ? "" : dirPath) + "/" + it->first;

    for(ObexTree::PathSet::iterator spIter = subscribedPaths_.begin();
        spIter != subscribedPaths_.end(); ++spIter) {
      if(ObexTree::objPathMatchesCbPath(objPath, *spIter)) {
        removeObj = false;
        break;
      }
    }
    if(removeObj) {
      objMap->map.erase(it++);
    } else {
      ++it;
    }
  }

  return objMap->empty() ? nullPtr(ObexObjectMap) : objMap;
}

void
ObexRemoteClientSession::init(String clientName, String sessionKey,
                        int fd, SPtr<IoService> ioSvc,
                        SPtr<ConnectionCallback> connectionCallback) {
  sessionKey_ = sessionKey;
  String sessionName = MkString(clientName << "-" << fd);

  obexTree_ = CreateObject<ObexTree>(sessionKey_);
  MsgIoSession::init(sessionName, fd, ioSvc, connectionCallback);

  SPtr<PeerInfoMsg> peerInfoMsg = CreateObject<PeerInfoMsg>(sessionName);
  enqMsg(peerInfoMsg);
}

String
ObexRemoteClientSession::getSessionKey() {
  return sessionKey_;
}

String
ObexRemoteClientSession::getSessionName() {
  return sessionName();
}

String
ObexRemoteClientSession::getPeerName() {
  return peerName();
}

SPtr<ObexObject>
ObexRemoteClientSession::getObject(String objPath) {
  return obexTree_->getObject(objPath);
}

void
ObexRemoteClientSession::putObject(String objPath, SPtr<ObexObject> obj, bool delOnDisconnect) {
  if(!ObexTree::validObjPath(objPath)) {
    TRACE9("invalid objPath: " << objPath);
    return;
  }

  if(xaction_) {
      xaction_->putObject(objPath, obj, delOnDisconnect);
  } else {
      String dod = delOnDisconnect ? PUT_OBJECT_OPT_DEL_ON_DISCONNECT : "";
      SPtr<PutObjectMsg> putObjMsg =
          CreateObject<PutObjectMsg>(objPath, obj, dod);
      enqMsg(putObjMsg);
  }
}

void
ObexRemoteClientSession::delObject(String objPath) {
  if(!ObexTree::validObjPath(objPath)) {
    TRACE9("invalid objPath: " << objPath);
    return;
  }

  if(xaction_) {
      xaction_->delObject(objPath);
  } else {
      SPtr<DelObjectMsg> delObjMsg = CreateObject<DelObjectMsg>(objPath);
      enqMsg(delObjMsg);
  }
}

void
ObexRemoteClientSession::subscribe(String cbPaths, SPtr<SubscriptionCallback> sCb) {
  Stringstream ss(cbPaths);
  String cbPath;
  while(getline(ss, cbPath, ',')) {
    if(!ObexTree::validCbPath(cbPath)) {
      TRACE9("invalid cbPaths: " << cbPaths);
      return;
    }
  }
  TRACE9("cbPaths:" << cbPaths);
  if(sCb) {
    subscriptionCallback_ = sCb;
    obexTree_->registerCallback(YpProcLocalSubscription,
                                getThisPtr<ObexCallback>());
  }
  SPtr<SubscribeMsg> subscribeMsg = CreateObject<SubscribeMsg>(cbPaths);
  enqMsg(subscribeMsg);
}

void
ObexRemoteClientSession::msgReceived(int type, int flags, const unsigned char* msgData, int msgLen ) {
  TRACE9(sessionName() << ": " <<
         "type:" << type << ", flags:" << flags << ", len:" << msgLen);

  switch(type) {
    default: {
      MsgIoSession::msgReceived(type, flags, msgData, msgLen);
      break;
    }
  }
}

void
ObexRemoteClientSession::registerCallback(String cbPath, SPtr<ObexCallback> cbObj) {
  if(!ObexTree::validCbPath(cbPath)) {
    TRACE9("invalid cbPath: " << cbPath);
    return;
  }
  obexTree_->registerCallback(cbPath, cbObj);
}

void
ObexRemoteClientSession::unregisterCallback(String cbPath, SPtr<ObexCallback> cbObj) {
  if(!ObexTree::validCbPath(cbPath)) {
    TRACE9("invalid cbPath: " << cbPath);
    return;
  }
  obexTree_->unregisterCallback(cbPath, cbObj);
}

void
ObexRemoteClientSession::disconnect() {
  MsgIoSession::disconnectSession();
}

void
ObexRemoteClientSession::onUpdated(String cbSrc, String path, SPtr<ObexObject> newObj, SPtr<ObexObject> oldObj) {
  TRACE9(cbSrc << ":" << path << " updated");
  if(path == YpProcLocalSubscription) {
    SPtr<ObexStringObject> sObj =
      DynamicPointerCast<ObexStringObject>(newObj);
    YASSERT(sObj, "Wrong object type " << newObj->className());

    SPtr<ObexJsonObject> jObj = CreateObject<ObexJsonObject>(
        MkString("{ \"sessionKey\":\"" << sessionKey_ << "\", " <<
                 "\"cbPaths\":\"" << sObj->getString() << "\" }"));
    USE_WPTR(SubscriptionCallback, sCb, subscriptionCallback_,
        {},
        { sCb->onSubscriptionCompleted(jObj); },
        {}, {});
  }
}

SPtr<ObexObjectMap>
ObexRemoteClientSession::find(String dirPath, String matchStr, bool recursive, bool (*matchFunc)(String, String)) {
  SPtr<ObexObjectMap> objMap = obexTree_->find(dirPath, matchStr, recursive, matchFunc);
  if(!objMap || objMap->empty()) return nullPtr(ObexObjectMap);
  return objMap;
}

void
ObexRemoteClientSession::setKeepalive(int idle, int interval, int cnt) {
  struct sockaddr_storage ss;
  socklen_t len;

  len = sizeof(struct sockaddr_storage);
  if (getpeername(fd(), (struct sockaddr*)&ss, &len) < 0) {
    TRACE9("getpeername error - " << strerror(errno));
    return;
  }

  // checking socket type: IPv4 or IPv6
  if ((ss.ss_family == AF_INET) || (ss.ss_family == AF_INET6)) {
    SetTcpKeepalive(fd(), true, idle, interval, cnt);
  }
}

void
ObexRemoteClientSession::beginTransaction() {
    YASSERT(!xaction_, "transaction has already been started");
    xaction_ = CreateObject<ObexTransaction>();
}

void
ObexRemoteClientSession::endTransaction() {
    YASSERT(xaction_, "transaction has never been started");
    if(!xaction_->empty()) {
        SPtr<TransactionMsg> xactionMsg =
            CreateObject<TransactionMsg>(xaction_);
        enqMsg(xactionMsg);
    }
    xaction_ = nullPtr(ObexTransaction);
}

void
ObexRemoteClientSession::abortTransaction() {
    YASSERT(xaction_, "transaction has never been started");
    xaction_ = nullPtr(ObexTransaction);
}

void
ObexRemoteClientSession::enqMsg(SPtr<MsgInfo> msgInfo) {
    if(msgInfo->msgType() == MSG_TYPE_TRANSACTION) {
        TRACE9("enq transaction msg");
        SPtr<TransactionMsg> xactionMsg = DynamicPointerCast<TransactionMsg>(msgInfo);
        assert(xactionMsg);
        txMsgList_.push_back(xactionMsg);
        sendData();
    } else {
        MsgIoSession::enqMsg(msgInfo);
    }
}

void
ObexRemoteClientSession::handleTransaction(SPtr<ObexTransaction> xaction) {
  TRACE9("handle transaction");

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
}

}
