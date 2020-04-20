#ifndef __OBEX_TREE_H__
#define __OBEX_TREE_H__

#include <Yail.h>
#include <ObexCallback.h>

#ifndef SWIG
namespace boost { namespace property_tree {

  template<typename U>
  struct translator_between< SPtr<U>, SPtr<U>> {
    typedef shared_ptr<U> internal_type;
    typedef shared_ptr<U> external_type;
    typedef translator_between<shared_ptr<U>, shared_ptr<U>> type;
    optional<external_type> get_value(const internal_type & i) { return i; }
    optional<internal_type> put_value(const external_type & e) { return e; }
  };

} }
#endif

enum dumpOutFormat {
  DEFAULT,
  JSON
};

namespace Yail {

class ObexObject;
class ObexObjectMap;
class ObexTree;
class ObexInfoProvider;

YAIL_BEGIN_CLASS(ObexMetaNode, EXTENDS(YObject))
  public:
    typedef UnorderedSet<SPtr<ObexCallback>> ObexCallbackSet_t;
    typedef Map<ObexCallback*, WPtr<ObexCallback>> ObexCallbackMap_t;
    void init() { version_ = 0; }

  private:
    friend class ObexTree;
    friend class ObexInfoProvider;
    ObexCallbackMap_t cCbs_; // current node only
    ObexCallbackMap_t iCbs_; // immediate children only, current node non-inclusive
    ObexCallbackMap_t aCbs_; // all descendants, current node non-inclusive

    SPtr<ObexObject> oObj_;
    size_t version_;
YAIL_END_CLASS

YAIL_BEGIN_CLASS(ObexTree, EXTENDS(YObject))
  public:
    void init(String name="", bool fwdOnly=false);
    friend class ObexInfoProvider;

    typedef boost::property_tree::basic_ptree<String,
            SPtr<ObexMetaNode>> PtNode_t;
    typedef UnorderedSet<String> PathSet; // set of paths
    typedef List<String> PathList; // list of paths
    typedef enum { Create, Update, Delete } OperationType;
    static bool validObjPath(String objPath);
    static bool validCbPath(String cbPath);
    void registerCallbackAndGetObjSet(
         String cbPath, SPtr<ObexCallback> callbackObj,
         PathSet *pathSet);
    void registerCallback(String cbPath, SPtr<ObexCallback> callbackObj);
    void unregisterCallback(String cbPath, SPtr<ObexCallback> callbackObj);
    SPtr<ObexObject> getObject(String objPath);
    template <typename T> SPtr<T> get(String objPath) {
      return DynamicPointerCast<T>(getObject(objPath));
    }
    void putObject(String objPath, SPtr<ObexObject> obj);
    void delObject(String objPath);
    String name() { return name_; }
    bool fwdOnly() { return fwdOnly_; }
    SPtr<ObexObjectMap> find(String dirPath, String matchStr, bool recursive, bool (*matchFunction)(String, String)=NULL);

    static bool objPathMatchesCbPath(String objPath, String cbPath);
    static void addToCallbackSet(ObexMetaNode::ObexCallbackSet_t& cbSet,
                                 ObexMetaNode::ObexCallbackMap_t& cbMap);
    String dumpTree(dumpOutFormat format=DEFAULT);
    void printTree(dumpOutFormat format=DEFAULT);
    void saveTree(String filePath, dumpOutFormat format=DEFAULT);

  private:
    String name_;
    bool fwdOnly_;
    typedef enum { currentNodeOnly,
                   immediateChildren,
                   allDescendants } CallbackType;
    String convertToDotPath(String slashPath);
    String convertToSlashPath(String dotPath);
    String parsePath(String pStr, CallbackType& callbackType);

    SPtr<ObexMetaNode> getMetaNode(String dotPath);
    SPtr<ObexMetaNode> getMetaNode(PtNode_t& ptNode, bool autoCreate);
    PtNode_t& getChildPtNode(PtNode_t& ptNode, String token,
                             bool autoCreate);
    void putMetaNode(String dotPath, SPtr<ObexMetaNode> metaNode);
    void searchObjects(String dotPath, PtNode_t ptNode, PathSet* pathSet,
                       bool recursive);
    void findObjects(String pathPfx, String matchStr, PtNode_t ptNode,
                     SPtr<ObexObjectMap>& result, bool recursive, bool (*matchFunction)(String, String)=NULL);

    void dumpTree(std::stringstream &buffer, PtNode_t &pt, int level, dumpOutFormat format);

    PtNode_t ptree_;
    typedef boost::property_tree::translator_between<SPtr<ObexMetaNode>, SPtr<ObexMetaNode>> MetaNodeTranslator_t;
    MetaNodeTranslator_t tr_;
YAIL_END_CLASS
 
}

#endif

