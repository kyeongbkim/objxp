#ifndef __TIMER_OBJECT_H__
#define __TIMER_OBJECT_H__

#include <Yail.h>

namespace Yail {

YAIL_INTERFACE(IoService);
YAIL_CLASS(TimerObject);

struct Time {
  Time() : sec(0), nsec(0)  {}
  Time(time_t sec, long nsec) : sec(sec), nsec(nsec) {}
  static Time Now();

  time_t sec;
  long nsec;
};
Time operator +(Time l, const int& mSec);
Time operator -(Time l, const Time& r);
bool operator <(const Time& l, const Time& r);
bool operator ==(const Time& l, const Time& r);
bool operator >(const Time& l, const Time& r);

YAIL_BEGIN_INTERFACE(TimerCallback)
  virtual void onTimerExpired(SPtr<TimerObject> timerObj) = 0;
YAIL_END_INTERFACE

YAIL_BEGIN_CLASS_EXT(TimerObject, EXTENDS(YObject))
  void __new__TimerObject() { registered_ = false; }
  void __del__TimerObject() {}
  public:
    typedef void (*TimeoutHandler)(SPtr<TimerObject>);

    void init(String id, SPtr<TimerCallback> timerCallback);

    String getId() { return id_; }
    void setTimer(int mSec);
    bool isExpired(const Time& now);
    Time remainingTime(const Time& now);
    void onExpired();
    bool isBefore(const SPtr<SelfType>& r);
    bool isAfter(const SPtr<SelfType>& r);

    bool isRegistered() { return registered_; }
    void setRegistered(bool val) { registered_ = val; }

  private:
    bool registered_;
    String id_;
    Time expiry_;
    WPtr<TimerCallback> timerCallback_;
YAIL_END_CLASS

}

#endif

