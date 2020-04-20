%{
#include "Yail.h"
#include "Bitset.h"
#include "TimerObject.h"
#include "IoService.h"
#include "BasicIoService.h"
#include "IoObject.h"
#include "MsgIo.h"
#include "BasicSocketAcceptor.h"
#include "UnixSocketAcceptor.h"
#include "TcpSocketAcceptor.h"
#include "ObexObject.h"
#include "ObexNumberObject.h"
#include "ObexStringObject.h"
#include "rapidjson/document.h"
#include "ObexJsonObject.h"
#include "ObexTree.h"
#include "ObexPythonCallback.h"
#include "ObexCallbackReceiver.h"
#include "ConnectionCallback.h"
#include "SubscriptionCallback.h"
#include "ObexServer.h"
#include "ObexClient.h"
#include "ObexInfoProvider.h"
using namespace Yail;
%}

%include "std_string.i"
%include "std_vector.i"
%include "std_map.i"
%include "std_list.i"
%include "boost_shared_ptr.i"
typedef unsigned long time_t;

%shared_ptr(Yail::YObject);
%include "Yail.h"

%shared_ptr(Yail::Bitset);
%include "Bitset.h"

%shared_ptr(Yail::TimerCallback);
%shared_ptr(Yail::TimerObject);
%include "TimerObject.h"

%shared_ptr(Yail::IoService);
%include "IoService.h"

%shared_ptr(Yail::BasicIoService);
%include "BasicIoService.h"

%shared_ptr(Yail::IoObject);
%rename(opFormattedOutput) operator<<;
%include "IoObject.h"

%ignore MsgInfo;
%shared_ptr(Yail::SerializedMsg);
%shared_ptr(Yail::MsgInfo);
%shared_ptr(Yail::PeerInfoMsg);
%shared_ptr(Yail::PutObjectMsg);
%shared_ptr(Yail::DelObjectMsg);
%shared_ptr(Yail::TransactionMsg);
%shared_ptr(Yail::MsgIoSession);
%include "MsgIo.h"

%shared_ptr(Yail::BasicSocketAcceptor);
%include "BasicSocketAcceptor.h"
%shared_ptr(Yail::UnixSocketAcceptor);
%include "UnixSocketAcceptor.h"
%shared_ptr(Yail::TcpSocketAcceptor);
%include "TcpSocketAcceptor.h"

%template(SessionSummaries) std::map<std::string, std::string>;
%template(StringToObexObjectMap) std::map<std::string, SPtr<Yail::ObexObject>>;
%shared_ptr(Yail::SerializedObject);
%shared_ptr(Yail::ObexObject);
%shared_ptr(Yail::ObexObjectMap);
%shared_ptr(Yail::ObexSerializedObject);
%shared_ptr(Yail::ObexObjectTuple);
%shared_ptr(Yail::ObexTransactionElement);
%shared_ptr(Yail::ObexTransaction);
%include "ObexObject.h"

%shared_ptr(Yail::ObexIntObject);
%shared_ptr(Yail::ObexFloatObject);
%shared_ptr(Yail::ObexBoolObject);
%include "ObexNumberObject.h"
%inline %{
  SPtr<Yail::ObexIntObject>
  DynamicCast_ObexIntObject(SPtr<Yail::ObexObject> obexObj) {
    return DynamicPointerCast<Yail::ObexIntObject>(obexObj);
  }
  SPtr<Yail::ObexFloatObject>
  DynamicCast_ObexFloatObject(SPtr<Yail::ObexObject> obexObj) {
    return DynamicPointerCast<Yail::ObexFloatObject>(obexObj);
  }
  SPtr<Yail::ObexBoolObject>
  DynamicCast_ObexBoolObject(SPtr<Yail::ObexObject> obexObj) {
    return DynamicPointerCast<Yail::ObexBoolObject>(obexObj);
  }
%}

%shared_ptr(Yail::ObexStringObject);
%include "ObexStringObject.h"
%inline %{
  SPtr<Yail::ObexStringObject>
  DynamicCast_ObexStringObject(SPtr<Yail::ObexObject> obexObj) {
    return DynamicPointerCast<Yail::ObexStringObject>(obexObj);
  }
%}

%shared_ptr(Yail::ObexJsonObject);
%include "ObexJsonObject.h"
%inline %{
  SPtr<Yail::ObexJsonObject>
  DynamicCast_ObexJsonObject(SPtr<Yail::ObexObject> obexObj) {
    return DynamicPointerCast<Yail::ObexJsonObject>(obexObj);
  }
%}

%shared_ptr(Yail::ObexCallback);
%shared_ptr(Yail::ObexDefaultCallback);
%include "ObexCallback.h"

%shared_ptr(Yail::ObexPythonCallback);
%include "ObexPythonCallback.h"

%shared_ptr(Yail::ObexCallbackReceiver);
%include "ObexCallbackReceiver.h"

%shared_ptr(Yail::ObexTree);
%shared_ptr(Yail::ObexMetaNode);
%include "ObexTree.h"

%shared_ptr(Yail::SubscriptionCallback);
%shared_ptr(Yail::DefaultSubscriptionCallback);
%include "SubscriptionCallback.h"

%shared_ptr(Yail::ConnectionCallback);
%include "ConnectionCallback.h"

%shared_ptr(Yail::ObexServerCallback);
%shared_ptr(Yail::ObjectCallbackMsg);
%shared_ptr(Yail::ObexServer);
%shared_ptr(Yail::ObexServerSession);
%include "ObexServer.h"

%shared_ptr(Yail::ObexClient);
%shared_ptr(Yail::ObexClientSession);
%shared_ptr(Yail::SubscribeMsg);
%shared_ptr(Yail::ObexCohabClientSession);
%shared_ptr(Yail::ObexRemoteClientSession);
%inline %{
  SPtr<Yail::ObexRemoteClientSession>
  DynamicCast_ObexRemoteClientSession(SPtr<Yail::ObexClientSession> session) {
    return DynamicPointerCast<Yail::ObexRemoteClientSession>(session);
  }
%}

%shared_ptr(Yail::ObexConnectionCallback);
%shared_ptr(Yail::DefaultConnectionCallback);
%include "ObexClient.h"

%shared_ptr(Yail::ObexInfoProvider);
%include "ObexInfoProvider.h"

