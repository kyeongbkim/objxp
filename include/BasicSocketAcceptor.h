#ifndef __BASIC_SOCKET_ACCEPTOR_H__
#define __BASIC_SOCKET_ACCEPTOR_H__

#include <Yail.h>
#include <IoObject.h>

namespace Yail {

YAIL_BEGIN_CLASS(BasicSocketAcceptor, EXTENDS(IoObject))
  public:
    void init(String name, int fd, SPtr<IoService> ioSvc,
              SPtr<ConnectionCallback> connectionCallback);

    virtual void handleIo(bool rdReady, bool wrReady) override = 0;

  protected:
    WPtr<ConnectionCallback> connectionCallback() {
      return connectionCallback_;
    }

  private:
    WPtr<ConnectionCallback> connectionCallback_;
YAIL_END_CLASS

}

#endif

