#include <ObexCallbackReceiver.h>

namespace Yail {

void
ObexCallbackReceiver::onUpdated(String cbSrc, String path,
    SPtr<ObexObject> newObj, SPtr<ObexObject> oldObj) {
  cbObjs_.push_back(CreateObject<ObexObjectTuple>(cbSrc, path, newObj));
}

void
ObexCallbackReceiver::onDeleted(String cbSrc, String path,
    SPtr<ObexObject> oldObj) {
  cbObjs_.push_back(CreateObject<ObexObjectTuple>(
        cbSrc, path, nullPtr(ObexObject)));
}

SPtr<ObexObjectTuple>
ObexCallbackReceiver::pop() {
  if(cbObjs_.empty()) {
    return nullPtr(ObexObjectTuple);
  }
  SPtr<ObexObjectTuple> objTuple = cbObjs_.front();
  cbObjs_.pop_front();
  return objTuple;
}

}

