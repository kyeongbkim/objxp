#ifndef __TCP_SOCKET_ACCEPTOR_H__
#define __TCP_SOCKET_ACCEPTOR_H__

#include <Yail.h>
#include <BasicSocketAcceptor.h>

namespace Yail {

YAIL_BEGIN_CLASS(TcpSocketAcceptor, EXTENDS(BasicSocketAcceptor))
  public:
    void init(String sessionNamePfx, int svcPort, SPtr<IoService> ioSvc,
              SPtr<ConnectionCallback> connectionCallback);

    void handleIo(bool rdReady, bool wrReady) override;

  private:
    String sessionNamePfx_;
    int svcPort_;
YAIL_END_CLASS

}

#endif

