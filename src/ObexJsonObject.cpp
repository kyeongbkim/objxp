#include <ObexJsonObject.h>
#include <rapidjson/document.h>

using namespace rapidjson;

namespace Yail {

INIT_OBEX_INSTANTIATOR(ObexJsonObject);

void
ObexJsonObject::init(String jsonStr, bool checkErr) {
  if(checkErr) {
    unmarshal(jsonStr);
  } else {
    jsonStr_ = jsonStr;
  }
}

bool
ObexJsonObject::equals(ObexObject& oObj) {
  ObexJsonObject* ojObj = dynamic_cast<ObexJsonObject*>(&oObj);
  return ojObj && (ojObj->getString() == getString());
}

String
ObexJsonObject::marshal() {
  return jsonStr_;
}

void
ObexJsonObject::unmarshal(String& data) {
  Document document;
  bool err = document.Parse(data.c_str()).HasParseError();
  if(!err) jsonStr_ = data;
}

}

