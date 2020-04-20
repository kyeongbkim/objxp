#include <sys/socket.h>
#include <sys/un.h>

#include <IoService.h>
#include <MsgIo.h>
#include <SocketConnector.h>

namespace Yail {

void
SocketConnector::init(String sessionName, int retryPeriod, int retryLimit, SPtr<IoService> ioSvc, SPtr<ConnectionCallback> connectionCallback) {
  sessionName_ = sessionName;
  retryPeriod_ = retryPeriod;
  retryLimit_ = retryLimit;
  ioSvc_ = ioSvc;
  connectionCallback_ = connectionCallback;

  SPtr<SocketConnector> thisPtr = getThisPtr<SocketConnector>();
  timerObj_ = CreateObject<TimerObject>(
      "SocketConnector::tryConnection", thisPtr);
  ioSvc_->registerTimer(timerObj_, 0);
}

void
SocketConnector::onTimerExpired(SPtr<TimerObject> timerObj) {
  assert(timerObj == timerObj_);

  switch(connState_) {
    case ConnState_Idle:
    {
      connState_ = connect();
      if(connState_ == ConnState_Connected) {
        TRACE9("Connection success");
        USE_WPTR(ConnectionCallback, cb, connectionCallback_,
            {},
            { cb->onConnected(sessionName_, cliSock_); },
            {}, {});
        timerObj_.reset();
      } else if(connState_ == ConnState_InProgress) {
        ioSvc_->registerTimer(timerObj_, 100);
      }
      break;
    }
    case ConnState_InProgress:
    {
      fd_set wfds;
      struct timeval tout = { tv_sec: 0, tv_usec: 0 };
      int nReady;

      FD_ZERO(&wfds);
      assert(cliSock_ >= 0);
      FD_SET(cliSock_, &wfds);

      // Note that non-blocking connect uses the wfds, not rfds
      nReady = select(cliSock_ + 1, NULL, &wfds, NULL, &tout);
      if((nReady > 0) && FD_ISSET(cliSock_, &wfds)) {
        int optval = -1;
        socklen_t optlen = sizeof(optval);

        if(getsockopt(cliSock_, SOL_SOCKET, SO_ERROR, &optval, &optlen) == 0) {
          if(optval == 0) {
            TRACE9("Connection success");
            connState_ = ConnState_Connected;
            USE_WPTR(ConnectionCallback, cb, connectionCallback_,
                {},
                { cb->onConnected(sessionName_, cliSock_); },
                {}, {});
            timerObj_.reset();
          } else {
            // connection failed
            connState_ = ConnState_Idle;
          }
        } else {
          // getsockopt failed
          connState_ = ConnState_Idle;
        }
      } else {
        // select failed or cliSock_ not ready yet
        ioSvc_->registerTimer(timerObj_, 100);
      }
      break;
    }
    default: {
      // already connected.
      break;
    }
  }

  // Connection failed. Setup retry if applicable
  if(connState_ == ConnState_Idle) {
    if(cliSock_ >= 0) close(cliSock_);
    cliSock_ = -1;

    if((retryPeriod_ > 0) &&
       ((retryLimit_ <= 0) || (retryLimit_ > retryCount_))) {
      retryCount_ += 1;
      TRACE9("Connection failed. Retry " << retryCount_ <<
             " in " << retryPeriod_ << " ms");
      ioSvc_->registerTimer(timerObj_, retryPeriod_);
    } else {
      TRACE9("connection failed");
      USE_WPTR(ConnectionCallback, cb, connectionCallback_,
          {},
          { cb->onConnectionFailed(sessionName_); },
          {}, {});
      timerObj_.reset();
    }
  }
}

}

