#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <IoService.h>
#include <MsgIo.h>
#include <UnixSocketAcceptor.h>
#include <UnixSocketConnector.h>

namespace Yail {

void
UnixSocketConnector::init(String sessionName, String sockName, int retryPeriod, int retryLimit, SPtr<IoService> ioSvc, SPtr<ConnectionCallback> connectionCallback) {
  SocketConnector::init(sessionName, retryPeriod, retryLimit, ioSvc, connectionCallback);
  sockName_ = sockName;
}

SocketConnector::ConnState_t
UnixSocketConnector::connect() {
  int flags;
  struct sockaddr_un addr;
  int cliSock = socket(AF_UNIX, SOCK_STREAM, 0);

  if ( cliSock < 0 ) {
    TRACE9("socket error");
    goto handle_err;
  }

  flags = fcntl(cliSock, F_GETFL);
  if(fcntl(cliSock, F_SETFL, flags | O_NONBLOCK) < 0) {
    TRACE9("fcntl error");
    goto handle_err;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  sprintf(addr.sun_path, "%s/%s", getenv("YAIL_SOCK_PATH") ?: UnixSocketAcceptor::SOCK_PATH_PREFIX, sockName_.c_str());

  if (::connect(cliSock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    if(errno == EINPROGRESS) {
      TRACE9("connection in progress");
      cliSock_ = cliSock;
      return ConnState_InProgress;
    }
    TRACE9("connect error");
    goto handle_err;
  }

  TRACE9("connection success");
  cliSock_ = cliSock;
  return ConnState_Connected;

handle_err:
  if(cliSock >= 0) {
    close(cliSock);
  }
  TRACE9("connection failed");
  return ConnState_Idle;
}

}

