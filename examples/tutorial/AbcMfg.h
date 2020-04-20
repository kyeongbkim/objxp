#ifndef __ABC_MFG_H__
#define __ABC_MFG_H__

#include <TimerObject.h>
#include <ObexClient.h>

using namespace Yail;

YAIL_BEGIN_CLASS(AbcMfg, EXTENDS(YObject), IMPLEMENTS(ObexCallback),
                                           IMPLEMENTS(TimerCallback))
  public:
    void init(SPtr<ObexClientSession> mfgChannel, SPtr<IoService> ioSvc);

    String callbackName() override { return className(); }
    void onUpdated(String cbSrc, String path,
        SPtr<ObexObject> newObj, SPtr<ObexObject> oldObj) override;
    void onDeleted(String cbSrc, String path,
        SPtr<ObexObject> oldObj) override;

    void onTimerExpired(SPtr<TimerObject> tObj) override;

  private:
    SPtr<ObexClientSession> mfgChannel_;
    SPtr<IoService> ioSvc_;
    Map<String, SPtr<TimerObject>> timers_;
YAIL_END_CLASS

#endif

