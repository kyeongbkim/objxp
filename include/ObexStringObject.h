#ifndef __OBEX_STRING_OBJECT_H__
#define __OBEX_STRING_OBJECT_H__

#include <ObexObject.h>

namespace Yail {

YAIL_BEGIN_CLASS(ObexStringObject, EXTENDS(ObexObject))
  public:
    YAIL_OBEX_INSTANTIATOR(ObexStringObject);
    void init(String str) { str_ = str; }

    bool equals(ObexObject& oObj) override {
      ObexStringObject* osObj = dynamic_cast<ObexStringObject*>(&oObj);
      return osObj && (osObj->getString() == str_);
    }

    String toString() override { return str_; }
    String getString() { return str_; }

    String marshal() override {
      Stringstream ss;
      PtreeNode oTree;
      oTree.put("str", str_);
      JSONSerializer::serialize(ss, oTree);
      return ss.str();
    }
    void unmarshal(String& data) override {
      Stringstream ss;
      ss << data;
      PtreeNode iTree;
      JSONSerializer::deserialize(ss, iTree);
      str_ = iTree.get<String>("str");
    }

  private:
    String str_;
YAIL_END_CLASS

}

#endif

