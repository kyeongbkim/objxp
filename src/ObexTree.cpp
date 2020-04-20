#include <errno.h>
#include <ObexObject.h>
#include <ObexTree.h>

namespace Yail {

void
ObexDefaultCallback::onUpdated(String cbSrc, String objPath,
    SPtr<ObexObject> newObj, SPtr<ObexObject> oldObj) {
  TRACE9(cbName_ << ", " << objPath << " updated, " <<
         "cbSrc: " << cbSrc << ", " <<
         newObj->className() << ", " << newObj->toString());
}

void
ObexDefaultCallback::onDeleted(String cbSrc, String objPath,
    SPtr<ObexObject> oldObj) {
  TRACE9(cbName_ << ", " << objPath << " deleted" << "cbSrc: " << cbSrc);
}

void
ObexTree::init(String name, bool fwdOnly) {
  name_ = name;
  fwdOnly_ = fwdOnly;
}

bool
ObexTree::validObjPath(String objPath) {
  if(objPath == "/") return true;
  if(objPath.at(0) != '/') return false;
  if(objPath.at(objPath.length()-1) == '/') return false;
  if(objPath.find("//") != String::npos) return false;
  for(String::iterator it = objPath.begin(); it < objPath.end(); it++) {
    if(!isalnum(*it) && (*it != '/') && (*it != '_') && (*it != '-')
        && (*it != '.'))
      return false;
  }
  return true;
}

bool
ObexTree::validCbPath(String cbPath) {
  String objPath;
  if(cbPath.length() >= 2 &&
     cbPath.compare(cbPath.length()-2, cbPath.length(), "/*") == 0) {
    if(cbPath.length() == 2) return true;
    objPath = cbPath.substr(0, cbPath.length()-2);
  } else if(cbPath.length() >= 1 &&
            cbPath.compare(cbPath.length()-1, cbPath.length(), "/") == 0) {
    if(cbPath.length() == 1) return true;
    objPath = cbPath.substr(0, cbPath.length()-1);
  } else {
    objPath = cbPath;
  }
  return validObjPath(objPath);
}

/*********************************/

static const char ESC = '\\';

String
ObexTree::convertToDotPath(String slashPath) {
  if(slashPath.empty()) return String("");
  assert(slashPath.find(ESC) == String::npos);

  assert(slashPath.compare(0, 1, "/") == 0);
  String dotPath = slashPath.substr(1);

  std::replace( dotPath.begin(), dotPath.end() , '.', ESC);
  std::replace( dotPath.begin(), dotPath.end() , '/', '.');
  return dotPath;
}

String
ObexTree::convertToSlashPath(String dotPath) {
  assert(dotPath.find('/') == String::npos);
  String slashPath = "." + dotPath;
  std::replace(slashPath.begin(), slashPath.end() , '.', '/');
  std::replace(slashPath.begin(), slashPath.end() , ESC, '.');
  return slashPath;
}

String
ObexTree::parsePath(String pStr, CallbackType& callbackType) {
  String path;
  if(pStr.length() >= 2 && pStr.compare(pStr.length()-2, pStr.length(), "/*") == 0) {
    callbackType = allDescendants;
    path = pStr.substr(0, pStr.length()-2);
  } else if(pStr.length() >= 1 && pStr.compare(pStr.length()-1, pStr.length(), "/") == 0) {
    callbackType = immediateChildren;
    path = pStr.substr(0, pStr.length()-1);
  } else {
    callbackType = currentNodeOnly;
    path = pStr;
  }
  return convertToDotPath(path);
}

SPtr<ObexMetaNode>
ObexTree::getMetaNode(String dotPath) {
  if(dotPath.empty()) return ptree_.get_value<SPtr<ObexMetaNode>>(tr_);
  try {
    return ptree_.get<SPtr<ObexMetaNode>>(dotPath, tr_);
  } catch(const boost::property_tree::ptree_error &e) {
    return nullPtr(ObexMetaNode);
  }
}

SPtr<ObexMetaNode>
ObexTree::getMetaNode(PtNode_t& ptNode, bool autoCreate) {
  SPtr<ObexMetaNode> metaNode;
  metaNode = ptNode.get_value<SPtr<ObexMetaNode>>(tr_);
  if(!metaNode && autoCreate) {
    metaNode = CreateObject<ObexMetaNode>();
    ptNode.put_value(metaNode);
  }
  return metaNode;
}

ObexTree::PtNode_t&
ObexTree::getChildPtNode(PtNode_t& ptNode, String token, bool autoCreate) {
  try {
    return ptNode.get_child(token);
  } catch(const boost::property_tree::ptree_error &e) {
    if(autoCreate) {
      PtNode_t ptChildNode;
      return ptNode.put_child(token, ptChildNode);
    } else {
      throw ENOENT;
    }
  }
}

void
ObexTree::putMetaNode(String dotPath, SPtr<ObexMetaNode> metaNode) {
  if(dotPath.empty()) ptree_.put_value(metaNode);
  else ptree_.put(dotPath, metaNode);
}

bool
ObexTree::objPathMatchesCbPath(String objPath, String cbPath) {
  if(!ObexTree::validObjPath(objPath) || !ObexTree::validCbPath(cbPath)) {
    return false;
  }

  if(cbPath.compare(cbPath.length()-2, cbPath.length(), "/*") == 0) {
    String matchStr = cbPath.substr(0, cbPath.length()-2);
    return (objPath.compare(0, matchStr.length(), matchStr) == 0);
  }

  if(cbPath.compare(cbPath.length()-1, cbPath.length(), "/") == 0) {
    if(objPath.compare(0, cbPath.length(), cbPath) != 0) return false;
    String remainder = objPath.substr(cbPath.length());
    return remainder.find('/') == String::npos;
  }

  return objPath == cbPath;
}

void
ObexTree::addToCallbackSet(ObexMetaNode::ObexCallbackSet_t& cbSet,
                           ObexMetaNode::ObexCallbackMap_t& cbMap) {
  ObexMetaNode::ObexCallbackMap_t::iterator it;
  it = cbMap.begin();
  while(it != cbMap.end()) {
    USE_WPTR(ObexCallback, cb, it->second, {},
      { cbSet.insert(cb); ++it; },
      { ObexMetaNode::ObexCallbackMap_t::iterator toErase = it; ++it;
        cbMap.erase(toErase);
        TRACE9("remove deleted callback");
      }, {}
    );
  }
}

SPtr<ObexObject>
ObexTree::getObject(String objPath) {
  if(!validObjPath(objPath)) {
    TRACE9("invalid objPath: " << objPath);
    return nullPtr(ObexObject);
  }
  String dotPath = convertToDotPath(objPath);
  SPtr<ObexMetaNode> metaNode = getMetaNode(dotPath);
  if(!metaNode || !metaNode->oObj_) return nullPtr(ObexObject);
  return metaNode->oObj_;
}

void
ObexTree::putObject(String objPath, SPtr<ObexObject> newObj) {
  if(!validObjPath(objPath)) {
    TRACE9("invalid objPath: " << objPath);
    return;
  }

  String dotPath = convertToDotPath(objPath);
  Istringstream is(dotPath);

  PtNode_t* pPtNode = &ptree_;
  UnorderedSet<SPtr<ObexCallback>> cbSet;
  SPtr<ObexMetaNode> metaNode;
  SPtr<ObexMetaNode> prevMetaNode;

  while(true) {
    String nextToken;
    bool isLast = !getline(is, nextToken, '.');
    if(!isLast) {
      metaNode = getMetaNode(*pPtNode, false);
      if(metaNode && !metaNode->aCbs_.empty()) {
        addToCallbackSet(cbSet, metaNode->aCbs_);
      }
      prevMetaNode = metaNode;
      PtNode_t& ptChildNode = getChildPtNode(*pPtNode, nextToken, true);
      pPtNode = &ptChildNode;
    } else {
      metaNode = getMetaNode(*pPtNode, true);
      assert(metaNode);
      if(metaNode->oObj_ && metaNode->oObj_->equals(*newObj)) {
        TRACE9("object is identical for " << "'" << objPath << "'");
        return;
      }
      if(prevMetaNode) {
        addToCallbackSet(cbSet, prevMetaNode->iCbs_);
      }
      addToCallbackSet(cbSet, metaNode->cCbs_);
      break;
    }
  }

  // handle callbacks
  assert(metaNode);
  OperationType oper = metaNode->oObj_ ? Update : Create;
  TRACE9("object " << newObj->className() << " " <<
         ((oper == Update) ? "updated" : "created") <<
         " for " << "'" << objPath << "'");

  SPtr<ObexObject> oldObj = metaNode->oObj_;
  metaNode->oObj_ = newObj;

  for(ObexMetaNode::ObexCallbackSet_t::iterator cbIter = cbSet.begin();
      cbIter != cbSet.end(); ++cbIter) {
    SPtr<ObexCallback> cb = *cbIter;
    cb->onUpdated(name(), objPath, newObj, oldObj);
  }
}

void
ObexTree::delObject(String objPath) {
  if(!validObjPath(objPath)) {
    TRACE9("invalid objPath: " << objPath);
    return;
  }

  typedef Pair<PtNode_t*, String> PtParentInfo_t;
  // <pPtChildNode, <pPtParentNode, childKey>>
  typedef Pair<PtNode_t*, PtParentInfo_t> PtCleanUpInfo_t;
  List<PtCleanUpInfo_t> cleanUpInfoList;

  String dotPath = convertToDotPath(objPath);
  Istringstream is(dotPath);

  PtNode_t* pPtNode = &ptree_;
  UnorderedSet<SPtr<ObexCallback>> cbSet;
  SPtr<ObexMetaNode> metaNode;
  SPtr<ObexMetaNode> prevMetaNode;

  while(true) {
    String nextToken;
    bool isLast = !getline(is, nextToken, '.');
    if(!isLast) {
      metaNode = getMetaNode(*pPtNode, false);
      if(metaNode && !metaNode->aCbs_.empty()) {
        addToCallbackSet(cbSet, metaNode->aCbs_);
      }
      prevMetaNode = metaNode;
      try {
        PtNode_t& ptChildNode = getChildPtNode(*pPtNode, nextToken, false);
        cleanUpInfoList.push_front(
            PtCleanUpInfo_t(&ptChildNode,
                            PtParentInfo_t(pPtNode, nextToken)));
        pPtNode = &ptChildNode;
      } catch(int err) {
        // trying to delete non-existing node
        return;
      } 
    } else {
      metaNode = getMetaNode(*pPtNode, false);
      if(!metaNode || !metaNode->oObj_) return;
      if(prevMetaNode) {
        addToCallbackSet(cbSet, prevMetaNode->iCbs_);
      }
      addToCallbackSet(cbSet, metaNode->cCbs_);
      break;
    }
  }

  TRACE9("object deleted for '" << objPath << "'");
  SPtr<ObexObject> oldObj = metaNode->oObj_;
  metaNode->oObj_.reset();

  // handle callbacks
  assert(metaNode);
  for(UnorderedSet<SPtr<ObexCallback>>::iterator cbIter = cbSet.begin();
      cbIter != cbSet.end(); ++cbIter) {
    SPtr<ObexCallback> cb = *cbIter;
    cb->onDeleted(name(), objPath, oldObj);
  }

  // Clean up unused meta PtNodes
  for( ; !cleanUpInfoList.empty(); cleanUpInfoList.pop_front()) {
    PtCleanUpInfo_t cleanUpInfo = cleanUpInfoList.front();

    PtNode_t* pPtChildNode  = cleanUpInfo.first;
    PtParentInfo_t parentInfo = cleanUpInfo.second;

    PtNode_t* pPtParentNode = parentInfo.first;
    String childKey = parentInfo.second;

    if(!pPtChildNode->empty()) break;

    SPtr<ObexMetaNode> metaNode = getMetaNode(*pPtChildNode, false);
    if(!metaNode) {
      TRACE9("deleting empty ptNode " << childKey);
      pPtParentNode->erase(childKey);
    } else {
      if(metaNode->oObj_) break;
      if(!metaNode->cCbs_.empty()) break;
      if(!metaNode->iCbs_.empty()) break;
      if(!metaNode->aCbs_.empty()) break;

      TRACE9("deleting trivial ptNode " << childKey);
      pPtParentNode->erase(childKey);
    }
  }
}

void
ObexTree::searchObjects(String dotPath, PtNode_t ptNode, PathSet* pathSet, bool recursive) {
  for(PtNode_t::iterator iter = ptNode.begin(); iter != ptNode.end(); iter++) {
    String key = iter->first;
    PtNode_t ptChildNode = iter->second;

    String childDotPath = dotPath.empty() ? key : dotPath + "." + key;
    SPtr<ObexMetaNode> mNode = ptChildNode.get_value<SPtr<ObexMetaNode>>(tr_);
    if(mNode && mNode->oObj_) pathSet->insert(convertToSlashPath(childDotPath));
    if(recursive) {
      searchObjects(childDotPath, ptChildNode, pathSet, recursive);
    }
  }
}

void
ObexTree::registerCallbackAndGetObjSet(String cbPath, SPtr<ObexCallback> oCb, PathSet *pathSet) {
  if(!validCbPath(cbPath)) {
    TRACE9("invalid cbPath: " << cbPath);
    return;
  }

  CallbackType callbackType;
  String dotPath = parsePath(cbPath, callbackType);
  TRACE9( "cbPath:" << cbPath << ", dotPath:" << "'" << dotPath << "'" <<
          " callbackType:" << callbackType);

  SPtr<ObexMetaNode> metaNode = getMetaNode(dotPath);
  if(!metaNode) {
    metaNode = CreateObject<ObexMetaNode>();
    putMetaNode(dotPath, metaNode);
  }

  if(callbackType == allDescendants) {
    metaNode->aCbs_.insert(
        Pair<ObexCallback*, WPtr<ObexCallback>>(oCb.get(), oCb));
    if(pathSet) {
      PtNode_t ptNode = ptree_.get_child(dotPath);
      searchObjects(dotPath, ptNode, pathSet, true);
    }
  } else if(callbackType == immediateChildren) {
    metaNode->iCbs_.insert(
        Pair<ObexCallback*, WPtr<ObexCallback>>(oCb.get(), oCb));
    if(pathSet) {
      PtNode_t ptNode = ptree_.get_child(dotPath);
      searchObjects(dotPath, ptNode, pathSet, false);
    }
  } else {
    metaNode->cCbs_.insert(
        Pair<ObexCallback*, WPtr<ObexCallback>>(oCb.get(), oCb));
    if(pathSet) {
      if(metaNode->oObj_) pathSet->insert(convertToSlashPath(dotPath));
    }
  }
  TRACE9( dotPath << " Cb:" << oCb->callbackName() <<
      ", callbackSize:" << metaNode->cCbs_.size() <<
      ", iCallbackSize:" << metaNode->iCbs_.size() <<
      ", aCallbackSize:" << metaNode->aCbs_.size());
}

void
ObexTree::registerCallback(String cbPath, SPtr<ObexCallback> callbackObj) {
  registerCallbackAndGetObjSet(cbPath, callbackObj, NULL);
}

void
ObexTree::unregisterCallback(String cbPath, SPtr<ObexCallback> oCb) {
  if(!validCbPath(cbPath)) {
    TRACE9("invalid cbPath: " << cbPath);
    return;
  }

  if(!oCb) {
    TRACE9("invalid oCb");
    return;
  }

  CallbackType callbackType;
  String dotPath = parsePath(cbPath, callbackType);
  TRACE9( "cbPath:" << cbPath << ", dotPath:" << "'" << dotPath << "'" <<
          " callbackType:" << callbackType);

  SPtr<ObexMetaNode> metaNode = getMetaNode(dotPath);
  if(!metaNode) {
    TRACE9(cbPath << ": meta node not found");
    return;
  }

  if(callbackType == allDescendants) {
    metaNode->aCbs_.erase(oCb.get());
  } else if(callbackType == immediateChildren) {
    metaNode->iCbs_.erase(oCb.get());
  } else {
    metaNode->cCbs_.erase(oCb.get());
  }
  TRACE9( dotPath << " Cb:" << oCb->callbackName() <<
      ", callbackSize:" << metaNode->cCbs_.size() <<
      ", iCallbackSize:" << metaNode->iCbs_.size() <<
      ", aCallbackSize:" << metaNode->aCbs_.size());
}

SPtr<ObexObjectMap>
ObexTree::find(String dirPath, String matchStr, bool recursive, bool (*matchFunction)(String, String)) {
  if(!validObjPath(dirPath)) {
    TRACE9("invalid dirPath: " << dirPath);
    return nullPtr(ObexObjectMap);
  }

  String dotPath = convertToDotPath(dirPath);

  PtNode_t ptNode;
  try {
    ptNode = ptree_.get_child(dotPath);
  } catch(const boost::property_tree::ptree_error &e) {
    TRACE9("no such node: " << dirPath);
    return nullPtr(ObexObjectMap);
  }
  SPtr<ObexObjectMap> result = CreateObject<ObexObjectMap>();
  findObjects("", matchStr, ptNode, result, recursive, matchFunction);
  return result;
}

void
ObexTree::findObjects(String dotPathPfx, String matchStr, PtNode_t ptNode,
                      SPtr<ObexObjectMap>& result, bool recursive, bool (*matchFunction)(String, String)) {
  for(PtNode_t::iterator iter = ptNode.begin(); iter != ptNode.end(); iter++) {
    String key = iter->first;
    PtNode_t ptChildNode = iter->second;

    String childDotPath = dotPathPfx.empty() ? key : dotPathPfx + "." + key;
    SPtr<ObexMetaNode> mNode = ptChildNode.get_value<SPtr<ObexMetaNode>>(tr_);

    if(mNode && mNode->oObj_) {
      // Currently ".*" is the only supported regular expression
      if((matchStr == ".*") || (key == matchStr) || (matchFunction && matchFunction(key, matchStr))) {
        String childSlashPath = convertToSlashPath(childDotPath);
        childSlashPath = childSlashPath.substr(1);
        result->map.insert(
            Pair<String, SPtr<ObexObject>>(childSlashPath, mNode->oObj_));
      }
    }

    if(recursive) {
      findObjects(childDotPath, matchStr, ptChildNode, result, recursive, matchFunction);
    }
  }
}

void
ObexTree::printTree(dumpOutFormat format) {
  String dumped = dumpTree(format=format);
  std::cout << dumped << std::endl;
}

void
ObexTree::saveTree(String filePath, dumpOutFormat format) {
  std::ofstream outFile(filePath.c_str());
  std::stringstream buffer;
  dumpTree(buffer, ptree_, 0, format);
  outFile << buffer.rdbuf();
  outFile.close();
}

String
ObexTree::dumpTree(dumpOutFormat format) {
  std::stringstream buffer;
  dumpTree(buffer, ptree_, 0, format);
  return buffer.str();
}

void
ObexTree::dumpTree(std::stringstream &buffer, PtNode_t &pt, int level, dumpOutFormat format) {
  if (pt.empty()) {
    return;
  }
  PtNode_t::iterator pos = pt.begin();
  while (pos != pt.end()) {
    if (format == DEFAULT) {
      for (int i=0; i<level; i++) {
        buffer << "--";
      }
      buffer << "/" << pos->first;
      SPtr<ObexMetaNode> mNode =  pos->second.get_value<SPtr<ObexMetaNode>>(tr_);
      if (mNode && mNode->oObj_)
        buffer << ": " << mNode->oObj_->toString();
      buffer << std::endl;
      dumpTree(buffer, pos->second, level + 1, format);
      ++pos;
    }
  }
}

}

