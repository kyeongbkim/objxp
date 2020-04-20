#ifndef __CONNECTION_CALLBACK_H__
#define __CONNECTION_CALLBACK_H__

#include <Yail.h>

namespace Yail {

YAIL_BEGIN_INTERFACE(ConnectionCallback)
  virtual void onConnected(String sessionName, int fd) = 0;
  virtual void onConnectionFailed(String sessionName) = 0;
  virtual void onDisconnected(String sessionName, int fd) = 0;
YAIL_END_INTERFACE

}

#endif

