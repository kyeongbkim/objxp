#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#include <IoService.h>
#include <MsgIo.h>
#include <UnixSocketAcceptor.h>
#include <YUtils.h>
//#include <boost/filesystem.hpp>

namespace Yail {

const char UnixSocketAcceptor::SOCK_PATH_PREFIX[] = "/var/yail/sock";

void
UnixSocketAcceptor::__del__UnixSocketAcceptor() {
  TRACE9("");
  if(!sockPath_.empty()) {
    unlink(MkCString(sockPath_));
  }
}

void
UnixSocketAcceptor::init(String sessionNamePfx,
                         String sockName,
                         SPtr<IoService> ioSvc,
                         SPtr<ConnectionCallback> connectionCallback) {
  sessionNamePfx_ = sessionNamePfx;

  //boost::filesystem::create_directories(SOCK_PATH_PREFIX);
  int svrSock = socket(AF_UNIX, SOCK_STREAM, 0);
  if(svrSock < 0) {
    perror("socket error");
    goto handle_err;
  }

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  sockPath_ =
    MkString((getenv("YAIL_SOCK_PATH") ?: SOCK_PATH_PREFIX) << "/" << sockName);
  sprintf(addr.sun_path, "%s", sockPath_.c_str());

  assert((unlink(addr.sun_path) == 0) || (errno == ENOENT));

  if(bind(svrSock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("bind error");
    goto handle_err;
  };

  if(listen(svrSock, 5) == -1) {
    perror("listen error");
    goto handle_err;
  }

  BasicSocketAcceptor::init(MkString("UnixSocketAcceptor-" << svrSock),
                            svrSock, ioSvc, connectionCallback);
  return;

handle_err:
  if(svrSock >= 0) {
      close(svrSock);
  }
}

void
UnixSocketAcceptor::handleIo(bool rdReady, bool wrReady) {
  struct sockaddr_un cli_addr;
  socklen_t clilen = sizeof(cli_addr);

  assert(!wrReady);
  if(!rdReady) return;

  int newFd = accept(fd(), (struct sockaddr *)&cli_addr, &clilen);
  if (newFd < 0) {
    perror("ERROR on accept");
    exit(-1);
  }

  String sessionFd = MkString(sessionNamePfx_ << "-" << newFd);
  USE_WPTR(ConnectionCallback, cb, connectionCallback(),
      {},
      { cb->onConnected(sessionFd, newFd); },
      {}, {});
}

}
