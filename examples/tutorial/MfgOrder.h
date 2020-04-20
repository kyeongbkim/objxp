#ifndef __MFG_ORDER_H__
#define __MFG_ORDER_H__

#include <ObexObject.h>

using namespace Yail;
using namespace std;

YAIL_BEGIN_CLASS_EXT(MfgOrder, EXTENDS(ObexObject))
  void __new__MfgOrder() {}
  void __del__MfgOrder() { TRACE8(toString() << " deleted"); }
  public:
    YAIL_OBEX_INSTANTIATOR(MfgOrder);

    void init(String moId, String item, int quantity) {
      moId_ = moId;
      item_ = item;
      quantity_ = quantity;
    }

    bool equals(ObexObject& oObj) override {
      MfgOrder* mo = dynamic_cast<MfgOrder*>(&oObj);
      return (mo && (mo->moId_ == moId_) &&
                    (mo->item_ == item_) && (mo->quantity_ == quantity_));
    }
    String toString() override {
      return MkString("MO[" << moId_ << ", " <<
                      item_ << ", " << quantity_ << "]");
    }
    String marshal() override {
      Stringstream ss;
      PtreeNode oTree;
      oTree.put("moId", moId_);
      oTree.put("item", item_);
      oTree.put("qty", quantity_);
      JSONSerializer::serialize(ss, oTree);
      return ss.str();
    }
    void unmarshal(String& data) override {
      Stringstream ss;
      ss << data;
      PtreeNode iTree;
      JSONSerializer::deserialize(ss, iTree);
      moId_ = iTree.get<String>("moId");
      item_ = iTree.get<String>("item");
      quantity_ = iTree.get<int>("qty");
    }

    String getMoId() { return moId_; }

  private:
    String moId_;
    String item_;
    int quantity_;
YAIL_END_CLASS

#endif

