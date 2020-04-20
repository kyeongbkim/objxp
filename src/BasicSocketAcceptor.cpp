#include <MsgIo.h>
#include <IoService.h>
#include <BasicSocketAcceptor.h>

namespace Yail {

void
BasicSocketAcceptor::init(String name, int fd, SPtr<IoService> ioSvc,
                          SPtr<ConnectionCallback> connectionCallback) {
  connectionCallback_ = connectionCallback;
  IoObject::init(name, fd, ioSvc);
}

}

