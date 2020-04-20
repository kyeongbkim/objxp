#ifndef __OBEX_PYTHON_CALLBACK_H__
#define __OBEX_PYTHON_CALLBACK_H__

#ifdef List
#undef List
#endif

#include <Python.h>

#define List std::list
#include <list>
#include <ObexCallback.h>
#include <ObexObject.h>

namespace Yail {

YAIL_BEGIN_CLASS(ObexPythonCallback, EXTENDS(YObject),
                                     IMPLEMENTS(ObexCallback))
  public:
    void init(String cbName, PyObject* callable) {
        cbName_ = cbName;
        pyCallable_ = callable;
    }

    String callbackName() override { return cbName_; }
    void onUpdated(String cbSrc, String path,
                   SPtr<ObexObject> newObj, SPtr<ObexObject> oldObj) override;
    void onDeleted(String cbSrc, String path, SPtr<ObexObject> oldObj) override;

  private:
    String cbName_;
    PyObject* pyCallable_;

YAIL_END_CLASS

}

#endif

