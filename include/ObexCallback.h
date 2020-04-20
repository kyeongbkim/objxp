#ifndef __OBEX_CALLBACK_H__
#define __OBEX_CALLBACK_H__

#include <Yail.h>

namespace Yail {

class ObexObject;

#define IMMEDIATE_CHILDREN "/"
#define ALL_DESCENDANTS    "/*"

YAIL_BEGIN_INTERFACE(ObexCallback)
  virtual String callbackName() = 0;
  virtual void onUpdated(String cbSrc, String path,
      SPtr<ObexObject> newObj, SPtr<ObexObject>oldObj) = 0;
  virtual void onDeleted(String cbSrc, String path,
      SPtr<ObexObject> oldObj) = 0;
YAIL_END_INTERFACE

YAIL_BEGIN_CLASS(ObexDefaultCallback, EXTENDS(YObject),
                                      IMPLEMENTS(ObexCallback))
  public:
    void init(String cbName) { cbName_ = cbName;}

    String callbackName() override { return cbName_; }
    void onUpdated(String cbSrc, String path,
        SPtr<ObexObject> newObj, SPtr<ObexObject> oldObj) override;
    void onDeleted(String cbSrc, String path,
        SPtr<ObexObject> oldObj) override;

  private:
    String cbName_;
YAIL_END_CLASS

}

#endif

