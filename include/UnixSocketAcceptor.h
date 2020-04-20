#ifndef __UNIX_SOCKET_ACCEPTOR_H__
#define __UNIX_SOCKET_ACCEPTOR_H__

#include <Yail.h>
#include <BasicSocketAcceptor.h>

namespace Yail {

YAIL_BEGIN_CLASS_EXT(UnixSocketAcceptor, EXTENDS(BasicSocketAcceptor))
  void __new__UnixSocketAcceptor() {}
  void __del__UnixSocketAcceptor();
  public:
    void init(String sessionNamePfx,
              String sockName,
              SPtr<IoService> ioSvc,
              SPtr<ConnectionCallback> connectionCallback);

    static const char SOCK_PATH_PREFIX[];

    void handleIo(bool rdReady, bool wrReady) override;

  private:
    String sessionNamePfx_;
    String sockPath_;
YAIL_END_CLASS

}

#endif

