#ifndef __CONSOLE_IO_H__
#define __CONSOLE_IO_H__

#include <Yail.h>
#include <IoObject.h>

namespace Yail {

YAIL_BEGIN_CLASS(ConsoleIoSession, EXTENDS(IoObject))
  public:
    void init(SPtr<IoService> ioSvc);

    void handleIo(bool rdReady, bool wrReady) override;

  protected:
    virtual void handleCommand(char* line) {}
YAIL_END_CLASS

}

#endif

