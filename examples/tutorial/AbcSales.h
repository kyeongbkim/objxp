#ifndef __ABC_SALES_H__
#define __ABC_SALES_H__

#include <ObexClient.h>

using namespace Yail;

YAIL_BEGIN_CLASS(AbcSales, EXTENDS(YObject), IMPLEMENTS(ObexCallback))
  public:
    void init(SPtr<ObexClientSession> salesChannel,
              SPtr<ObexClientSession> mfgChannel);

    String callbackName() override { return className(); }
    void onUpdated(String cbSrc, String path,
        SPtr<ObexObject> newObj, SPtr<ObexObject> oldObj) override;
    void onDeleted(String cbSrc, String path,
        SPtr<ObexObject> oldObj) override;

    String generateMfgOrderId() {
      static int id = 101;
      return MkString("M" << id++);
    }

  private:
    SPtr<ObexClientSession> salesChannel_;
    SPtr<ObexClientSession> mfgChannel_;
    Map<String, String> moPoMap_;
YAIL_END_CLASS

#endif

