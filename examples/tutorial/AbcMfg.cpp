#include <ObexStringObject.h>
#include <IoService.h>
#include <YUtils.h>
#include "AbcResource.h"
#include "AbcMfg.h"
#include "MfgOrder.h"

void
AbcMfg::init(SPtr<ObexClientSession> mfgChannel, SPtr<IoService> ioSvc) {
  mfgChannel_ = mfgChannel;
  ioSvc_ = ioSvc;

  mfgChannel->subscribe(YpAbcMfgOrder "/*");
  mfgChannel->registerCallback(YpAbcMfgOrder "/", getThisPtr<ObexCallback>());

  TRACE8("Ready");
}

void
AbcMfg::onUpdated(String cbSrc, String path,
    SPtr<ObexObject> newObj, SPtr<ObexObject> oldObj) {
  Vector<String> tokens; ParseYPath(tokens, path);
  assert(tokens.size() > 2);

  if(tokens[1] == "mfgOrder") {
    SPtr<MfgOrder> mfgOrder = DynamicPointerCast<MfgOrder>(newObj);
    TRACE8("Manufacturing order received: " << mfgOrder->toString());

    String moId = mfgOrder->getMoId();
    String mfgStatusPath = MkYPath(YpAbcMfgStatus__MO_ID,
                                   MkYPathArg("MO_ID", moId));

    // Update MfgStatus to "Started"
    SPtr<ObexStringObject> mfgStatusObj =
      CreateObject<ObexStringObject>("Started");
    mfgChannel_->putObject(mfgStatusPath, mfgStatusObj);

    SPtr<TimerObject> tObj = CreateObject<TimerObject>(
        moId, getThisPtr<TimerCallback>());
    timers_.insert(Pair<String, SPtr<TimerObject>>(moId, tObj));
    ioSvc_->registerTimer(tObj, 3000);

    TRACE8("MO[" << moId << "] Production in progress ...\n\n");

  } else {
    TRACE8("Ignore " << tokens[1]);
  }
}

void
AbcMfg::onTimerExpired(SPtr<TimerObject> tObj) {
  String moId = tObj->getId();

  TRACE8("MO[" << moId << "] Ready to ship");

  timers_.erase(moId);

  String mfgStatusPath = MkYPath(YpAbcMfgStatus__MO_ID,
                                 MkYPathArg("MO_ID", moId));

  // Update MfgStatus to "Completed"
  SPtr<ObexStringObject> mfgStatusObj =
    CreateObject<ObexStringObject>("Completed");
  mfgChannel_->putObject(mfgStatusPath, mfgStatusObj);

  // Clear MfgOrder
  String mfgOrderPath = MkYPath(YpAbcMfgOrder__MO_ID,
                                MkYPathArg("MO_ID", moId));
  TRACE8("Clearing MfgOrder " << mfgOrderPath);
  mfgChannel_->delObject(mfgOrderPath);
}

void
AbcMfg::onDeleted(String cbSrc, String path, SPtr<ObexObject> oldObj) {
  TRACE8(path << " cleared");
}

