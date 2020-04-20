#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <IoService.h>
#include <MsgIo.h>
#include <TcpSocketConnector.h>
#include <YUtils.h>

namespace Yail {

void
TcpSocketConnector::init(String sessionName, String host, int port, int retryPeriod, int retryLimit, SPtr<IoService> ioSvc, SPtr<ConnectionCallback> connectionCallback) {
  SocketConnector::init(sessionName, retryPeriod, retryLimit, ioSvc, connectionCallback);
  host_ = host;
  port_ = port;
}

SocketConnector::ConnState_t
TcpSocketConnector::connect() {
  int flags;
  struct sockaddr_in addr;
  int cliSock = socket(AF_INET, SOCK_STREAM, 0);

  if ( cliSock < 0 ) {
    TRACE9("socket error");
    goto handle_err;
  }

  flags = fcntl(cliSock, F_GETFL);
  if(fcntl(cliSock, F_SETFL, flags | O_NONBLOCK) < 0) {
    TRACE9("fcntl error");
    goto handle_err;
  }

  // enable TCP keepalive
  SetTcpKeepalive(cliSock, true,
      KEEPALIVE_IDLE, KEEPALIVE_INTERVAL, KEEPALIVE_CNT);

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(host_.c_str());
  addr.sin_port = htons(port_);

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

