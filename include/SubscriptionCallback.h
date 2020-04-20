#ifndef __SUBSCRIPTION_CALLBACK_H__
#define __SUBSCRIPTION_CALLBACK_H__

#include <Yail.h>
#include <ObexJsonObject.h>

namespace Yail {

YAIL_BEGIN_INTERFACE(SubscriptionCallback)
  virtual void onSubscriptionCompleted(SPtr<ObexJsonObject> arg) = 0;
YAIL_END_INTERFACE

YAIL_BEGIN_CLASS_EXT(DefaultSubscriptionCallback,
                     EXTENDS(YObject), IMPLEMENTS(SubscriptionCallback))
  void __new__DefaultSubscriptionCallback() { count_ = 0; }
  void __del__DefaultSubscriptionCallback() {}
  public:
    void init() {}

    SPtr<ObexJsonObject> lastObj() { return lastObj_; }
    void clear() { lastObj_ = nullPtr(ObexJsonObject); count_ = 0; }
    size_t count() { return count_; }

    void onSubscriptionCompleted(SPtr<ObexJsonObject> arg) override {
      TRACE9("subscription completed " << arg->toString());
      lastObj_ = arg;
      count_++;
    }

  private:
    SPtr<ObexJsonObject> lastObj_;
    size_t count_;
YAIL_END_CLASS

}

#endif

