#ifndef __SOCKET_CONNECTOR_H__
#define __SOCKET_CONNECTOR_H__

#include <Yail.h>
#include <TimerObject.h>
#include <MsgIo.h>

namespace Yail {

YAIL_INTERFACE(IoService);

YAIL_BEGIN_CLASS_EXT(SocketConnector, EXTENDS(YObject),
                     IMPLEMENTS(TimerCallback))
  void __new__SocketConnector() {
    connState_ = ConnState_Idle;
    cliSock_ = -1;
    retryPeriod_ = 0;
    retryLimit_ = 0;
    retryCount_ = 0;
  }
  void __del__SocketConnector() {}
  public:
    void init(String sessionName,
        int retryPeriod, int retryLimit,
        SPtr<IoService> ioSvc,
        SPtr<ConnectionCallback> connectionCallback);

    String sessionName() { return sessionName_; }
    void onTimerExpired(SPtr<TimerObject> timerObj) override;

    typedef enum { ConnState_Idle,
                   ConnState_InProgress,
                   ConnState_Connected } ConnState_t;

  protected:
    virtual ConnState_t connect() = 0;
    ConnState_t connState_;

    String sessionName_;
    int retryPeriod_;
    int retryLimit_;
    int retryCount_;
    int cliSock_;
    SPtr<IoService> ioSvc_;
    WPtr<ConnectionCallback> connectionCallback_;
    SPtr<TimerObject> timerObj_;

YAIL_END_CLASS

}

#endif

