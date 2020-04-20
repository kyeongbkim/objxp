#ifndef __TCP_SOCKET_CONNECTOR_H__
#define __TCP_SOCKET_CONNECTOR_H__

#include <SocketConnector.h>

namespace Yail {

YAIL_BEGIN_CLASS(TcpSocketConnector, EXTENDS(SocketConnector))
  public:
    void init(String sessionName, String host, int port,
        int retryPeriod, int retryLimit,
        SPtr<IoService> ioSvc,
        SPtr<ConnectionCallback> connectionCallback);

  protected:
    ConnState_t connect() override;
    String host_;
    int port_;
YAIL_END_CLASS

}

#endif

