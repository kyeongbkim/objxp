#include <TimerObject.h>
#include <sys/time.h>
#include <IoService.h>

namespace Yail {

Time
Time::Now() {
  struct timespec ts;

  clock_gettime(CLOCK_MONOTONIC, &ts);

  return Time(ts.tv_sec, ts.tv_nsec);
}

Time operator +(Time t, const int& msec) {
  const int thousand = 1000;
  const int million  = thousand * thousand;
  const int billion  = thousand * million;
  t.sec += msec / thousand;
  t.nsec += (msec % thousand) * million;
  if(t.nsec >= billion) {
    t.sec  += t.nsec / billion;
    t.nsec %= billion;
  }
  return t;
}

Time operator -(Time l, const Time& r) {
  Time t;

  if(!(r < l)) {
    t.nsec = 0;
    t.sec = 0;
    return t;
  }

  if(l.nsec < r.nsec) {
    t.nsec = 1000000000 + l.nsec - r.nsec;
    t.sec = l.sec - r.sec - 1;
  } else {
    t.nsec = l.nsec - r.nsec;
    t.sec = l.sec - r.sec;
  }
  return t;
}

bool operator <(const Time& l, const Time& r) {
  return ((l.sec < r.sec) || ((l.sec == r.sec) && (l.nsec < r.nsec)));
}

bool operator ==(const Time& l, const Time& r) {
  return ((l.sec == r.sec) && (l.nsec == r.nsec));
}

bool operator >(const Time& l, const Time& r) {
  return (!(l < r) && !(l == r));
}

void
TimerObject::init(String id, SPtr<TimerCallback> timerCallback) {
  id_ = id;
  timerCallback_ = timerCallback;
}

void
TimerObject::setTimer(int mSec) {
  expiry_ = Time::Now() + mSec;
}

bool
TimerObject::isExpired(const Time& now) {
  return !(now < expiry_);
}

Time
TimerObject::remainingTime(const Time& now) {
  return (expiry_ - now);
}

void
TimerObject::onExpired() {
  USE_WPTR(TimerCallback, tcb, timerCallback_,
      {},
      {
        SPtr<TimerObject> thisPtr = getThisPtr<TimerObject>();
        tcb->onTimerExpired(thisPtr);
      }, {}, {});
}

bool
TimerObject::isBefore(const SPtr<TimerObject>& r) {
  return (expiry_ < r->expiry_);
}

bool
TimerObject::isAfter(const SPtr<TimerObject>& r) {
  return (expiry_ > r->expiry_);
}

}

