#include <sys/socket.h>
#include <netinet/in.h>

#include <IoService.h>
#include <MsgIo.h>
#include <TcpSocketAcceptor.h>
#include <YUtils.h>

namespace Yail {

void
TcpSocketAcceptor::init(String sessionNamePfx, int svcPort, SPtr<IoService> ioSvc, SPtr<ConnectionCallback> connectionCallback) {
  struct sockaddr_in serverAddr; /* server's addr */
  int optval;
  int svrSock;

  sessionNamePfx_ = sessionNamePfx;
  svcPort_ = svcPort;

  svrSock = socket(AF_INET, SOCK_STREAM, 0);
  if(svrSock < 0) {
    perror("socket error");
    goto handle_err;
  }

  optval = 1;
  setsockopt(svrSock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

  bzero((char *) &serverAddr, sizeof(serverAddr));

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddr.sin_port = htons((unsigned short)svcPort);

  if (bind(svrSock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
    perror("bind error");
    goto handle_err;
  }

  if (listen(svrSock, 5) < 0) {
    perror("listen error");
    goto handle_err;
  }

  BasicSocketAcceptor::init(MkString("TcpSocketAcceptor-" << svrSock),
                            svrSock, ioSvc, connectionCallback);
  return;

handle_err:
  if(svrSock >= 0) {
      close(svrSock);
  }
}

void
TcpSocketAcceptor::handleIo(bool rdReady, bool wrReady) {
  struct sockaddr_in cli_addr;
  socklen_t clilen = sizeof(cli_addr);

  assert(!wrReady);
  if(!rdReady) return;

  int newFd = accept(fd(), (struct sockaddr *)&cli_addr, &clilen);
  if (newFd < 0) {
    perror("ERROR on accept");
    exit(-1);
  }

  // enable TCP keepalive
  SetTcpKeepalive(newFd, true,
      KEEPALIVE_IDLE, KEEPALIVE_INTERVAL, KEEPALIVE_CNT);

  String sessionFd = MkString(sessionNamePfx_ << "-" << newFd);
  USE_WPTR(ConnectionCallback, cb, connectionCallback(),
      {},
      { cb->onConnected(sessionFd,  newFd); },
      {}, {});
}

}
