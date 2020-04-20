#include <IoObject.h>
#include <IoService.h>
#include <fcntl.h>

namespace Yail {

void
IoObject::__del__IoObject() {
  TRACE9("name_=" << name_ << ", fd_=" << fd_);
  deinit();
}

Ostream&
operator<<(Ostream& os, const IoObject& ioObj) {
  os << "name_=" << ioObj.name_ << ", fd_=" << ioObj.fd_;
  return os;
}

void
IoObject::init(String name, int fd, SPtr<IoService> ioSvc, bool isBlocking) {
  TRACE9("name=" << name << ", fd=" << fd);
  name_ = name;
  fd_ = fd;
  rxPaused_ = true;
  txReady_ = true;
  rxCnt_ = 0;
  txCnt_ = 0;
  ioSvc_ = ioSvc;

  if(fd >= 0) {
    if(!isBlocking) {
      int flags = fcntl(fd, F_GETFL, 0);
      if(flags < 0) flags = 0;
      fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

    requestRx();
  }
}

void
IoObject::deinit() {
  TRACE9("name=" << name_ << ", fd=" << fd_);
  if(fd_ >= 0) {
    resetRx();
    resetTx();
    close(fd_);
    rxCnt_ = 0;
    txCnt_ = 0;
    fd_ = -1;
  }
}

void
IoObject::rxReq(bool on, String msgStr) {
  if(on && rxPaused_) {
    TRACE9(name_ << " " << msgStr);
    ioSvc_->registerRx(getThisPtr<IoObject>());
    rxPaused_ = false;
  } else if(!on && !rxPaused_) {
    TRACE9(name_ << " " << msgStr);
    ioSvc_->unregisterRx(fd_);
    rxPaused_ = true;
  }
}

void
IoObject::txReq(bool on, String msgStr) {
  if(on && txReady_) {
    TRACE9(name_ << " " << msgStr);
    ioSvc_->registerTx(getThisPtr<IoObject>());
    txReady_ = false;
  } else if(!on && !txReady_) {
    TRACE9(name_ << " " << msgStr);
    ioSvc_->unregisterTx(fd_);
    txReady_ = true;
  }
}

}

