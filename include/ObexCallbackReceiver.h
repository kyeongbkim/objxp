#ifndef __OBEX_CALLBACK_RECEIVER_H__
#define __OBEX_CALLBACK_RECEIVER_H__

#include <list>
#include <ObexCallback.h>
#include <ObexObject.h>

namespace Yail {

YAIL_BEGIN_CLASS(ObexCallbackReceiver, EXTENDS(YObject),
                                       IMPLEMENTS(ObexCallback))
  public:
    void init(String cbName) { cbName_ = cbName;}

    String callbackName() override { return cbName_; }
    void onUpdated(String cbSrc, String path,
        SPtr<ObexObject> newObj, SPtr<ObexObject> oldObj) override;
    void onDeleted(String cbSrc, String path,
        SPtr<ObexObject> oldObj) override;

    bool empty() { return cbObjs_.empty(); }
    SPtr<ObexObjectTuple> pop();
    void clear() { cbObjs_.clear(); }
    size_t count() { return cbObjs_.size(); }

  private:
    String cbName_;
    List<SPtr<ObexObjectTuple>> cbObjs_;
YAIL_END_CLASS
 
}

#endif

