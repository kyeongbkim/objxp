#ifndef __OBEX_OBJEXT_H__
#define __OBEX_OBJEXT_H__

#include <map>
#include <list>
#include <Yail.h>
#include <JsonSerializer.h>

namespace Yail {

typedef boost::property_tree::ptree PtreeNode;

YAIL_BEGIN_CLASS(SerializedObject, EXTENDS(YObject))
  public:
    void init(String typeName, String data) {
      typeName_ = typeName;
      data_ = data;
    }
    bool equals(SerializedObject& serObj) {
      if(typeName_ != serObj.typeName_) return false;
      if(data_ != serObj.data_) return false;
      return true;
    }
    String typeName() { return typeName_; }
    String& data() { return data_; }

  private:
    String typeName_;
    String data_;
YAIL_END_CLASS

YAIL_BEGIN_CLASS(ObexObject, EXTENDS(YObject))
  public:
    void init(SPtr<SerializedObject> serObj) {
      if(serObj) setSerializedObject(serObj, true);
    }
    virtual String toString() {
      SPtr<SerializedObject> serObj = getSerializedObject();
      return serObj->data();
    }
    virtual bool equals(ObexObject& oObj) {
      if(strongSerObj_) {
        assert(oObj.strongSerObj_);
        return (strongSerObj_->equals(*oObj.strongSerObj_));
      }
      return (this == &oObj);
    }

    virtual String marshal() {
      assert(false && "should be implemented in subclasses");
      return String("");
    }
    virtual void unmarshal(String& data) {
      assert(false && "should be implemented in subclasses");
    }

    SPtr<SerializedObject> getSerializedObject() {
      if(strongSerObj_) return strongSerObj_;

      SPtr<SerializedObject> serObj = weakSerObj_.lock();
      if(serObj) {
        TRACE9("reuse serialized object");
        return serObj;
      }

      serObj = CreateObject<SerializedObject>(className(), marshal());
      weakSerObj_ = serObj;

      TRACE9("make serialized object");
      return serObj;
    }
    void setSerializedObject(SPtr<SerializedObject> serObj, bool strong) {
      if(strong) strongSerObj_ = serObj;
      else weakSerObj_ = serObj;
    }

  private:
    SPtr<SerializedObject> strongSerObj_;
    WPtr<SerializedObject> weakSerObj_;
YAIL_END_CLASS

YAIL_BEGIN_CLASS(ObexObjectTuple, EXTENDS(YObject))
  public:
    void init(String cbSrc, String path, SPtr<ObexObject> obj) {
      cbSrc_ = cbSrc;
      path_ = path;
      obj_ = obj;
    }

    String cbSrc() { return cbSrc_; }
    String path() { return path_; }
    SPtr<ObexObject> obj() { return obj_; }

  private:
    String cbSrc_;
    String path_;
    SPtr<ObexObject> obj_;
YAIL_END_CLASS

YAIL_BEGIN_CLASS(ObexTransactionElement, EXTENDS(YObject))
  public:
    typedef enum { XOP_PUT, XOP_DEL } TransactionOp;
    void init(TransactionOp op, String path, SPtr<ObexObject> obj, String opt) {
        op_ = op;
        path_ = path;
        obj_ = obj;
        opt_ = opt;
    }
    TransactionOp op() { return op_; }
    String path() { return path_; }
    String opt() { return opt_; }
    SPtr<ObexObject> obj() { return obj_; }
  private:
    TransactionOp op_;
    String path_;
    SPtr<ObexObject> obj_;
    String opt_;
YAIL_END_CLASS

YAIL_BEGIN_CLASS(ObexTransaction, EXTENDS(YObject))
  public:
    void init() {}
    void putObject(String objPath, SPtr<ObexObject> obj, bool dod=false);
    void delObject(String objPath);
    bool empty() {
        assert(xList_.empty() == xMap_.empty());
        return xList_.empty();
    }
    List<SPtr<ObexTransactionElement>>& getElementList() { return xList_; }

  private:
    List<SPtr<ObexTransactionElement>> xList_;
    Map<String, SPtr<ObexTransactionElement>> xMap_;
YAIL_END_CLASS

YAIL_BEGIN_CLASS(ObexObjectMap, EXTENDS(YObject))
  public:
    Map<String, SPtr<ObexObject>> map;
    bool empty() { return map.empty(); }
YAIL_END_CLASS

struct ObexObjectFactory {
  public:
    typedef Map<String, void*> registryMap;

    static registryMap & registry() {
      static registryMap impl;
      return impl;
    }

    static SPtr<ObexObject> instantiate(String name, SPtr<SerializedObject> serObj) {
      SPtr<ObexObject> oObj;
      registryMap::iterator it = registry().find(name);
      if(it == registry().end()) {
        TRACE1("Ignore unknown typeName " << name);
        return oObj;
      }
      typedef SPtr<ObexObject> (*create_type)(SPtr<SerializedObject> serObj);
      create_type create_fun = reinterpret_cast<create_type>(it->second);
      oObj = create_fun(serObj);
      if(serObj) oObj->setSerializedObject(serObj, false);
      return oObj;
    }
};

#ifndef SWIG
struct ObexRegistrar {
  template<typename F>
  ObexRegistrar(String name, F func) {
    ObexObjectFactory::registry()[name] = reinterpret_cast<void*>(func);
  }
};
#endif

}

#define YAIL_OBEX_INSTANTIATOR(typeName) \
  static Yail::ObexRegistrar registrar_; \
  static SPtr<ObexObject> instantiate(SPtr<SerializedObject> serObj) { \
    boost::shared_ptr<Yail::ObexObject> shPtr = boost::make_shared<typeName>(); \
    shPtr->unmarshal(serObj->data()); \
    return shPtr; \
  }

#define INIT_OBEX_INSTANTIATOR(typeName) \
  Yail::ObexRegistrar typeName::registrar_(typeName::__staticClassName(), typeName::instantiate)

#endif

