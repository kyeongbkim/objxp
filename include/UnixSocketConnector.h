#ifndef __UNIX_SOCKET_CONNECTOR_H__
#define __UNIX_SOCKET_CONNECTOR_H__

#include <SocketConnector.h>

namespace Yail {

YAIL_BEGIN_CLASS(UnixSocketConnector, EXTENDS(SocketConnector))
  public:
    void init(String sessionName, String sockName,
        int retryPeriod, int retryLimit,
        SPtr<IoService> ioSvc,
        SPtr<ConnectionCallback> connectionCallback);

  protected:
    ConnState_t connect() override;
    String sockName_;
YAIL_END_CLASS

}

#endif

