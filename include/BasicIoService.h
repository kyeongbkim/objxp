#ifndef __BASIC_IO_SERVICE_H__
#define __BASIC_IO_SERVICE_H__

#include <map>
#include <queue>
#include <list>

#include <Yail.h>
#include <Bitset.h>
#include <TimerObject.h>
#include <IoService.h>

namespace Yail {

class IoObject;

YAIL_BEGIN_CLASS(BasicIoService, EXTENDS(YObject),
                                 IMPLEMENTS(TimerCallback),
                                 IMPLEMENTS(IoService))
  public:
    void init() {}

    // IoService methods
    void registerRx(SPtr<IoObject> ioObj) override;
    void unregisterRx(int fd) override;
    void registerTx(SPtr<IoObject> ioObj) override;
    void unregisterTx(int fd) override;
    void registerTimer(SPtr<TimerObject> tObj, int mSec) override;
    void unregisterTimer(SPtr<TimerObject> tObj) override;

    // TimerCallback methods
    void onTimerExpired(SPtr<TimerObject> timerObj) override;

    int run(time_t duration = 0, int pollCount = 0);

  private:
    int getFdSetInfo(Bitset& bitSet, fd_set* fdset);

    Map<int, WPtr<IoObject>> rdObjs_;
    Map<int, WPtr<IoObject>> wrObjs_;

    Bitset rdBits_;
    Bitset wrBits_;
    List<WPtr<TimerObject>> timerList_;
    SPtr<TimerObject> timerObject_;
YAIL_END_CLASS

}

#endif

