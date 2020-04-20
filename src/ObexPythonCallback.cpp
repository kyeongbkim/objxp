#include <Python.h>
#include <boost/python.hpp>
#include <ObexPythonCallback.h>

namespace Yail {

void
ObexPythonCallback::onUpdated(String cbSrc, String path,
    SPtr<ObexObject> newObj, SPtr<ObexObject> oldObj) {

    PyGILState_STATE state = PyGILState_Ensure();
    boost::python::call<void>(pyCallable_,
            String("onUpdated"),
            cbSrc, path,
            newObj->toString(),
            oldObj ? oldObj->toString() : String(""));
    PyGILState_Release(state);
}

void
ObexPythonCallback::onDeleted(String cbSrc, String path,
    SPtr<ObexObject> oldObj) {
    TRACE1("");
    PyGILState_STATE state = PyGILState_Ensure();
    boost::python::call<void>(pyCallable_,
            "onDeleted",
            cbSrc, path,
            oldObj->toString(),
            String(""));
    PyGILState_Release(state);
}

}
