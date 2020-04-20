#ifndef __OBEX_JSON_OBJECT_H__
#define __OBEX_JSON_OBJECT_H__

#include <ObexObject.h>

namespace Yail {

YAIL_BEGIN_CLASS(ObexJsonObject, EXTENDS(ObexObject))
  public:
    YAIL_OBEX_INSTANTIATOR(ObexJsonObject);
    void init(String jsonStr, bool checkErr=false);
    bool equals(ObexObject& oObj) override;
    String toString() override { return getString(); }
    String getString() { return marshal(); }
    String marshal() override;
    void unmarshal(String& data) override;

  private:
    String jsonStr_;
YAIL_END_CLASS

}

#endif

