#ifndef __IO_SERVICE_H__
#define __IO_SERVICE_H__

#include <Yail.h>

namespace Yail {

class IoObject;
class TimerObject;

YAIL_BEGIN_INTERFACE(IoService)
    virtual void registerRx(SPtr<IoObject> ioObj) = 0;
    virtual void unregisterRx(int fd) = 0;
    virtual void registerTx(SPtr<IoObject> ioObj) = 0;
    virtual void unregisterTx(int fd) = 0;

    virtual void registerTimer(SPtr<TimerObject> tObj, int mSec) = 0;
    virtual void unregisterTimer(SPtr<TimerObject> tObj) = 0;
YAIL_END_INTERFACE

}

#endif

