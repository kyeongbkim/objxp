#ifndef __OBEX_INFO_PROVIDER_H__
#define __OBEX_INFO_PROVIDER_H__

#include <ObexCallback.h>
#include <ObexTree.h>
#include <ObexNumberObject.h>

namespace Yail {

class ObexTree;

YAIL_BEGIN_CLASS(ObexInfoProvider, EXTENDS(YObject),
                                   IMPLEMENTS(ObexCallback))
  public:
    void init(SPtr<ObexTree> obexTree);

    typedef ObexTree::PtNode_t PtNode_t;
    typedef ObexTree::MetaNodeTranslator_t MetaNodeTranslator_t;

    String callbackName() override { return className(); }
    void onUpdated(String cbSrc, String path,
        SPtr<ObexObject> newObj, SPtr<ObexObject> oldObj) override;
    void onDeleted(String cbSrc, String path,
        SPtr<ObexObject> oldObj) override;

  private:
    SPtr<ObexTree> obexTree_;

    void handleLsCommand(String sessionId, String cmdId,
                         int argc, char** argv);
    void handleCatCommand(String sessionId, String cmdId,
                          int argc, char** argv);
    void handleRmCommand(String sessionId, String cmdId,
                         int argc, char** argv);
    void handleEchoTransactionLogCommand(bool turnOn);
YAIL_END_CLASS

}

#endif

