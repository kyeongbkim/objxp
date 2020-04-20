#include <ObexStringObject.h>
#include <YUtils.h>
#include "AbcResource.h"
#include "AbcSales.h"
#include "MfgOrder.h"

void
AbcSales::init(SPtr<ObexClientSession> salesChannel,
               SPtr<ObexClientSession> mfgChannel) {
  salesChannel_ = salesChannel;
  mfgChannel_ = mfgChannel;

  salesChannel->subscribe(YpAbcPurchaseOrder "/*");
  salesChannel->registerCallback(YpAbcPurchaseOrder "/",
                                 getThisPtr<ObexCallback>());

  mfgChannel->subscribe(YpAbcMfgStatus "/*");
  mfgChannel->registerCallback(YpAbcMfgStatus "/", getThisPtr<ObexCallback>());
  TRACE8("Ready");
}

void
AbcSales::onUpdated(String cbSrc, String path,
    SPtr<ObexObject> newObj, SPtr<ObexObject> oldObj) {
  Vector<String> tokens;
  ParseYPath(tokens, path);
  if(tokens[0] != "abcCompany") return;

  assert(tokens.size() > 2);

  if(tokens[1] == "purchaseOrder") {
    String poId = tokens[2];
    String customer;
    String item;
    int quantity;

    // Assume PO format is like "Customer,Item,Quantity"
    SPtr<ObexStringObject> poDetail =
      DynamicPointerCast<ObexStringObject>(newObj);
    Istringstream is(poDetail->getString());
    String s;
    assert(getline(is, s, ',')); customer = s;
    assert(getline(is, s, ',')); item = s;
    assert(getline(is, s, ',')); quantity = atoi(s.c_str());

    TRACE8("Purchase order received: PO[" << poId << ", " <<
           customer << ", " << item << ", " << quantity << "]");

    String moId = generateMfgOrderId();
    String moPath = MkYPath(YpAbcMfgOrder__MO_ID, MkYPathArg("MO_ID", moId));
    SPtr<MfgOrder> mfgOrder =
      CreateObject<MfgOrder>(moId, item, quantity);

    // Remember moId to poId mapping.
    // Need to clear the original PO after shipping product to customer
    moPoMap_.insert(Pair<String, String>(moId, poId));

    // Place a manufacturing order
    TRACE8("Placing a manufacturing order " << moId);
    mfgChannel_->putObject(moPath, mfgOrder);

  } else if(tokens[1] == "mfgStatus") {
    String moId = tokens[2];
    SPtr<ObexStringObject> mfgStatus =
      DynamicPointerCast<ObexStringObject>(newObj);
    TRACE8("MO[" << moId << "] Status changed to " <<
           mfgStatus->toString());

    if(mfgStatus->toString() == "Completed") {
      Map<String, String>::iterator mIter = moPoMap_.find(moId);
      assert(mIter != moPoMap_.end());
      String poId = mIter->second;
      moPoMap_.erase(moId);

      TRACE8("MO[" << moId << "] Shipped to customer");

      String mfgStatusPath =
        MkYPath(YpAbcMfgStatus__MO_ID, MkYPathArg("MO_ID", moId));
      TRACE8("Clearing MfgStatus " << mfgStatusPath);
      mfgChannel_->delObject(mfgStatusPath);

      String poPath =
        MkYPath(YpAbcPurchaseOrder__PO_ID, MkYPathArg("PO_ID", poId));
      TRACE8("Clearing PurchaseOrder " << poPath);
      salesChannel_->delObject(poPath);
    }
  } else {
    TRACE8("Ignore " << tokens[1]);
  }
}

void
AbcSales::onDeleted(String cbSrc, String path, SPtr<ObexObject> oldObj) {
  TRACE8(path << " cleared");
}

