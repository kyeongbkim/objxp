#include <errno.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>

#include <ObexInfoProvider.h>
#include <ObexJsonObject.h>
#include <YUtils.h>
#include <YPathProc.h>

using namespace rapidjson;

namespace Yail {

void ObexInfoProvider::init(SPtr<ObexTree> obexTree) {
  obexTree_ = obexTree;
  obexTree->registerCallback(YpProcSession "/*", getThisPtr<ObexCallback>());
  obexTree->registerCallback(YpProcLogTransaction, getThisPtr<ObexCallback>());
}

static char** buildArgv(int &argc, const char* cmdStr) {
  #define MAX_ARGV 100
  #define DELIM " \r\n\t"
  int i = 0;
  char* line = strdup(cmdStr);
  char** argv = (char**)calloc(MAX_ARGV+1, sizeof(char*));
  char* token = strtok(line, DELIM);
  while(token && i < MAX_ARGV) {
    argv[i] = strdup(token);
    token = strtok(NULL, DELIM);
    i++;
  }
  free(line);
  argc = i;
  return argv;
}

static void freeArgv(char** argv) {
  int i = 0;
  while(argv[i]) {
    free(argv[i]);
    i++;
  }
  free(argv);
}

#define RapidJsonString(str, alloc) Value().SetString(StringRef(str), alloc)

void
ObexInfoProvider::handleLsCommand(String sessionId, String cmdId, int argc, char** argv) {
  String lsPath;
  String err;
  int lFlag = 0;
  int c;

  optind = 1;
  while ((c = getopt(argc, argv, "l")) != -1) {
    switch (c) {
      case 'l': lFlag = 1; break;
      default: break;
    }
  }

  if(optind >= argc) {
    TRACE9("Error: insufficient arguments");
    return;
  }
  lsPath = argv[optind];
  TRACE9("lsPath: " << lsPath << ", lFlag: " << lFlag);

  if(!obexTree_->validObjPath(lsPath)) {
    TRACE9("invalid lsPath: " << lsPath);
    return;
  }

  String dotPath = obexTree_->convertToDotPath(lsPath);

  PtNode_t ptNode;
  if(dotPath.empty()) {
    ptNode = obexTree_->ptree_;
  } else {
    try {
      ptNode = obexTree_->ptree_.get_child(dotPath);
      TRACE9("ptNode existing: " << dotPath);
    } catch(const boost::property_tree::ptree_error &e) {
      TRACE9("ptNode not existing: " << dotPath);
      err = MkString(ENOENT << " - " << strerror(ENOENT));
    }
  }

  Document doc(kObjectType);
  Document::AllocatorType& alloc = doc.GetAllocator();

  doc.AddMember("id", RapidJsonString(cmdId, alloc), alloc);
  if(!err.empty()) {
    doc.AddMember("err", RapidJsonString(err, alloc), alloc);
  } else {
    Value entries(kArrayType);
    PtNode_t::iterator iter;
    for(iter = ptNode.begin(); iter != ptNode.end(); ++iter) {
      String key = iter->first;
      PtNode_t ptChildNode = iter->second;

      entries.PushBack(Value().SetString(StringRef(key), alloc), alloc);
      TRACE9("child = " << key);
    }
    doc.AddMember("rsp", entries, alloc);
  }

  Ostringstream os;
  OStreamWrapper osw(os);
  Writer<OStreamWrapper> writer(osw);
  doc.Accept(writer);
  SPtr<ObexJsonObject> jObj = CreateObject<ObexJsonObject>(os.str());
  TRACE9("rsp = " << jObj->getString());

  String rspPath = MkYPath(YpProcSession__SID__Rsp,
                           MkYPathArg("SID", sessionId));
  obexTree_->putObject(rspPath, jObj);
}

static bool isAllPrintable(String const &input) {
  for(String::const_iterator it = input.begin(); it != input.end(); ++it) {
    String::value_type c = (*it);
    if (!isprint(c) && (c != '\n')) return false;
  }
  return true;
}

static String encode(const String& input) {
  Ostringstream output;
  output.fill('0');
  output << std::hex;

  for(String::const_iterator it = input.begin(); it != input.end(); ++it) {
    String::value_type c = (*it);

    if ((c == '%') || !isprint(c)) {
      output << std::uppercase;
      output << '%' << std::setw(2) << int((unsigned char) c);
      output << std::nouppercase;
    } else {
      output << c;
    }
  }

  return output.str();
}

void
ObexInfoProvider::handleCatCommand(String sessionId, String cmdId, int argc, char** argv) {
  String path;
  String err;

  if(argc < 2) {
    TRACE9("Error: insufficient arguments");
    return;
  }
  path = argv[1];
  TRACE9("path: " << path);

  if(!obexTree_->validObjPath(path)) {
    TRACE9("invalid path: " << path);
    return;
  }

  String dotPath = obexTree_->convertToDotPath(path);
  SPtr<ObexMetaNode> metaNode = obexTree_->getMetaNode(dotPath);
  if(!metaNode || !metaNode->oObj_) {
      err = MkString(ENOENT << " - " << strerror(ENOENT));
  }

  Document doc(kObjectType);
  Document::AllocatorType& alloc = doc.GetAllocator();

  doc.AddMember("id", RapidJsonString(cmdId, alloc), alloc);
  if(!err.empty()) {
    doc.AddMember("err", RapidJsonString(err, alloc), alloc);
  } else {
    SPtr<ObexObject> oObj = metaNode->oObj_;
    SPtr<SerializedObject> serObj = oObj->getSerializedObject();
    String serData;
    if(serObj) {
      serData = serObj->typeName() + ", " + serObj->data();
    } else {
      serData = oObj->className() + ", " + oObj->marshal();
    }
    TRACE9("serData : " << serData);
    if(isAllPrintable(serData)) {
      doc.AddMember("rsp", RapidJsonString(serData, alloc), alloc);
    } else {
      doc.AddMember("enc", Value().SetBool(true), alloc);
      String encodedData = encode(serData);
      doc.AddMember("rsp", RapidJsonString(encodedData, alloc), alloc);
    }
  }

  Ostringstream os;
  OStreamWrapper osw(os);
  Writer<OStreamWrapper> writer(osw);
  doc.Accept(writer);
  SPtr<ObexJsonObject> jObj = CreateObject<ObexJsonObject>(os.str());
  TRACE9("rsp = " << jObj->getString());

  String rspPath = MkYPath(YpProcSession__SID__Rsp,
                           MkYPathArg("SID", sessionId));
  obexTree_->putObject(rspPath, jObj);
}

void
ObexInfoProvider::handleRmCommand(String sessionId, String cmdId, int argc, char** argv) {
}

void
ObexInfoProvider::handleEchoTransactionLogCommand(bool turnOn) {
  if (turnOn) {
    setLogTransaction(true);
  } else {
    setLogTransaction(false);
  }
}

void
ObexInfoProvider::onUpdated(String cbSrc, String path,
    SPtr<ObexObject> newObj, SPtr<ObexObject> oldObj) {
  YPathArgMap args;
  try {
    ParseYPath(args, YpProcSession__SID__Cmd, path);
  } catch(int err) {
    args.clear();
    try {
      ParseYPath(args, YpProcLogTransaction, path);
      SPtr<ObexBoolObject> sObj = DynamicPointerCast<ObexBoolObject>(newObj);
      assert(sObj);
      handleEchoTransactionLogCommand(sObj->getBool());
      return;
    } catch (int errr) {
      return;
    }
  }
  String sessionId = GetYPathArg(args, "SID");

  TRACE9("newObj typeName : " << newObj->className());

  SPtr<ObexJsonObject> jObj = DynamicPointerCast<ObexJsonObject>(newObj);
  assert(jObj);

  Document jDoc;
  jDoc.Parse(jObj->getString().c_str());
  String cmdId  = jDoc["id"].GetString();
  String cmdStr = jDoc["cmd"].GetString();
  TRACE9("session:" << sessionId << ", id: " << cmdId << ", cmd: " << cmdStr);

  int argc = 0;
  char** argv = buildArgv(argc, cmdStr.c_str());
  if(argc > 0) {
    if(strncmp(argv[0], "ls", strlen(argv[0])) == 0) {
      handleLsCommand(sessionId, cmdId, argc, argv);
    } else if(strncmp(argv[0], "cat", strlen(argv[0])) == 0) {
      handleCatCommand(sessionId, cmdId, argc, argv);
    } else if(strncmp(argv[0], "rm", strlen(argv[0])) == 0) {
      handleRmCommand(sessionId, cmdId, argc, argv);
    } else {
      TRACE9("Unknown command: " << cmdStr);
    }
  }
  freeArgv(argv);
}

void
ObexInfoProvider::onDeleted(String cbSrc, String path,
    SPtr<ObexObject> oldObj) {
}

}

