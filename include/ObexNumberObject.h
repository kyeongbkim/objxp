#ifndef __OBEX_NUMBER_OBJECT_H__
#define __OBEX_NUMBER_OBJECT_H__

#include <ObexObject.h>

namespace Yail {

YAIL_BEGIN_CLASS(ObexIntObject, EXTENDS(ObexObject))
  public:
    YAIL_OBEX_INSTANTIATOR(ObexIntObject);
    void init(int val) { val_ = val; }

    bool equals(ObexObject& oObj) override {
      ObexIntObject* oiObj = dynamic_cast<ObexIntObject*>(&oObj);
      return (oiObj->getInt() == val_);
    }

    String toString() override { return MkString(val_); }
    int getInt() { return val_; }

    String marshal() override {
      Stringstream ss;
      PtreeNode oTree;
      oTree.put("val", val_);
      JSONSerializer::serialize(ss, oTree);
      return ss.str();
    }
    void unmarshal(String& data) override {
      Stringstream ss;
      ss << data;
      PtreeNode iTree;
      JSONSerializer::deserialize(ss, iTree);
      val_ = iTree.get<int>("val");
    }

  private:
    int val_;
YAIL_END_CLASS

YAIL_BEGIN_CLASS(ObexFloatObject, EXTENDS(ObexObject))
  public:
    YAIL_OBEX_INSTANTIATOR(ObexFloatObject);
    void init(float val) { val_ = val; }

    bool equals(ObexObject& oObj) override {
      ObexFloatObject* oiObj = dynamic_cast<ObexFloatObject*>(&oObj);
      return (oiObj->getFloat() == val_);
    }

    String toString() override { return MkString(val_); }
    float getFloat() { return val_; }

    String marshal() override {
      Stringstream ss;
      PtreeNode oTree;
      oTree.put("val", val_);
      JSONSerializer::serialize(ss, oTree);
      return ss.str();
    }
    void unmarshal(String& data) override {
      Stringstream ss;
      ss << data;
      PtreeNode iTree;
      JSONSerializer::deserialize(ss, iTree);
      val_ = iTree.get<float>("val");
    }

  private:
    float val_;
YAIL_END_CLASS

YAIL_BEGIN_CLASS(ObexBoolObject, EXTENDS(ObexObject))
  public:
    YAIL_OBEX_INSTANTIATOR(ObexBoolObject);
    void init(bool val) { val_ = val; }

    bool equals(ObexObject& oObj) override {
      ObexBoolObject* oiObj = dynamic_cast<ObexBoolObject*>(&oObj);
      return (oiObj->getBool() == val_);
    }

    String toString() override { return MkString(val_); }
    bool getBool() { return val_; }

    String marshal() override {
      Stringstream ss;
      PtreeNode oTree;
      oTree.put("val", val_);
      JSONSerializer::serialize(ss, oTree);
      return ss.str();
    }
    void unmarshal(String& data) override {
      Stringstream ss;
      ss << data;
      PtreeNode iTree;
      JSONSerializer::deserialize(ss, iTree);
      val_ = iTree.get<bool>("val");
    }

  private:
    bool val_;
YAIL_END_CLASS

}

#endif

