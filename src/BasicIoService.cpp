#include <BasicIoService.h>
#include <IoObject.h>
#include <TimerObject.h>

namespace Yail {

void
BasicIoService::onTimerExpired(SPtr<TimerObject> tObj) {
  assert(tObj == timerObject_);
  // Nothing to do
}

int
BasicIoService::run(time_t duration, int pollCount) {
  int err = 0;
  fd_set rfds, wfds;
  int fd, maxFd;
  int nReady;

  Time now = Time::Now();
  Time until;

  if(duration > 0) {
    until = now + duration;
    if(!timerObject_) {
      timerObject_ =
        CreateObject<TimerObject>("duration", getThisPtr<TimerCallback>());
    }
    registerTimer(timerObject_, duration);
  }

  do {
    struct timeval tv, *pTimeout = NULL;

    while(!timerList_.empty()) {
      USE_WPTR(TimerObject, tObj, timerList_.front(),
        {}, 
        {
          if(tObj->isExpired(now)) {
            timerList_.pop_front();
            tObj->onExpired();
          } else {
            Time remainingTime = tObj->remainingTime(now);
            tv.tv_sec = remainingTime.sec;
            tv.tv_usec = remainingTime.nsec/1000;
            pTimeout = &tv;
            break;
          }
        },
        {
          TRACE9("Remove deleted timer");
          timerList_.pop_front();
        },
        {}
      );
    }

    maxFd = -1;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);

    fd = getFdSetInfo(rdBits_, &rfds);
    if(fd > maxFd) maxFd = fd;
    fd = getFdSetInfo(wrBits_, &wfds);
    if(fd > maxFd) maxFd = fd;

    //TRACE9("maxFd=" << maxFd);
    nReady = select(maxFd+1, &rfds, &wfds, NULL, pTimeout);
    if(nReady > 0) {
      for(fd = 0; (fd <= maxFd) && (nReady > 0); fd++) {
        bool rd = FD_ISSET(fd, &rfds);
        bool wr = FD_ISSET(fd, &wfds);
        if(rd || wr) {
          SPtr<IoObject> ioObj;
          Map<int, WPtr<IoObject>>::iterator ioObjIter;
          if(rd) {
            ioObjIter = rdObjs_.find(fd);
            if(ioObjIter != rdObjs_.end()) {
              ioObj = (ioObjIter->second).lock();
            }
          } else if(wr) {
            ioObjIter = wrObjs_.find(fd);
            if(ioObjIter != wrObjs_.end()) {
              ioObj = (ioObjIter->second).lock();
            }
          } else {
            assert(false && "rd or wr must be set");
          }
          if(ioObj) {
            ioObj->handleIo(rd, wr);
          } else {
            TRACE9("ignore removed ioObj");
            rdObjs_.erase(fd);
            rdBits_.clearBit(fd);
            wrObjs_.erase(fd);
            wrBits_.clearBit(fd);
          }
          nReady--;
        }
      }
    } else if(nReady == 0) { // timeout
      //TRACE9("select timeout");
    } else { // error
      TRACE9("select interrupted");
      err = -1;
    }

    if((pollCount > 0) && (--pollCount == 0)) {
      if(timerObject_) unregisterTimer(timerObject_);
      break;
    }

    now = Time::Now();
  } while(!duration || (now < until));

  return err;
}

void
BasicIoService::registerRx(SPtr<IoObject> ioObj) {
  TRACE9(ioObj->name() << " rx req on");

  int fd = ioObj->fd();
  assert((fd >= 0) && (rdObjs_.find(fd) == rdObjs_.end()));
  rdObjs_.insert(Pair<int, WPtr<IoObject>>(fd, ioObj));
  rdBits_.setBit(fd);
}

void
BasicIoService::unregisterRx(int fd) {
  TRACE9("fd-" << fd << " rx req off");

  assert((fd >= 0) && (rdObjs_.find(fd) != rdObjs_.end()));
  rdObjs_.erase(fd);
  rdBits_.clearBit(fd);
}

void
BasicIoService::registerTx(SPtr<IoObject> ioObj) {
  TRACE9(ioObj->name() << " tx req on");

  int fd = ioObj->fd();
  assert((fd >= 0) && (wrObjs_.find(fd) == wrObjs_.end()));
  wrObjs_.insert(Pair<int, WPtr<IoObject>>(fd, ioObj));
  wrBits_.setBit(fd);
}

void
BasicIoService::unregisterTx(int fd) {
  TRACE9("fd-" << fd << " tx req off");

  assert((fd >= 0) && (wrObjs_.find(fd) != wrObjs_.end()));
  wrObjs_.erase(fd);
  wrBits_.clearBit(fd);
}

int
BasicIoService::getFdSetInfo(Bitset& bs, fd_set* fds) {
  int fd = Bitset::invalidPos;
  int maxFd = -1;
  while((fd = bs.findNext(fd+1)) != Bitset::invalidPos) {
    //TRACE9("fd " << fd << " is set");
    FD_SET(fd, fds);
    if(fd > maxFd) maxFd = fd;
  }
  return maxFd;
}

void
BasicIoService::registerTimer(SPtr<TimerObject> tObj, int mSec) {
  if(tObj->isRegistered()) {
    unregisterTimer(tObj);
  }

  tObj->setTimer(mSec);
  List<WPtr<TimerObject>>::iterator it = timerList_.begin();
  while(it != timerList_.end()) {
    bool delTimer = false;
    USE_WPTR(TimerObject, iterObj, *it, {},
      {
        if(tObj->isBefore(iterObj)) break;
      },
      {
        TRACE9("Remove deleted timer");
        delTimer = true;
      },
      {}
    );

    if(delTimer) {
      timerList_.erase(it++);
    } else {
      ++it;
    }
  }

  if(it == timerList_.end()) {
    timerList_.push_back(tObj);
  } else {
    timerList_.insert(it, tObj);
  }
  tObj->setRegistered(true);
}

void
BasicIoService::unregisterTimer(SPtr<TimerObject> tObj) {
  if(!tObj->isRegistered()) return;

  List<WPtr<TimerObject>>::iterator it = timerList_.begin();
  while(it != timerList_.end()) {
    bool delTimer = false;
    USE_WPTR(TimerObject, iterObj, *it, {},
      {
        if(tObj.get() == iterObj.get()) break;
      },
      {
        delTimer = true;
      },
      {}
    );

    if(delTimer) {
      timerList_.erase(it++);
    } else {
      ++it;
    }
  }

  if(it != timerList_.end()) {
    timerList_.erase(it++);
  }

  tObj->setRegistered(false);
}

}

