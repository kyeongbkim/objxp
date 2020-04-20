#include <map>
#include "Yail.h"

int gTraceLevel = 100;
log_dest_type LOG_DEST = LOG_DEST_COUT_DEFAULT;
std::shared_ptr<std::ofstream> logOutFile;
bool LOG_TRANSACTION = false;

void SetTraceLevel(int n) {
  gTraceLevel = n;
}

void setLogTransaction(bool b) {
  LOG_TRANSACTION = b;
}

void setLogDestination(log_dest_type dest, String destFile) {
  LOG_DEST = dest;
  if (dest == LOG_DEST_COUT_FILE) {
    logOutFile = std::make_shared<std::ofstream>(destFile.c_str(), std::ofstream::out | std::ofstream::app);
  }
}

void __TRACE(int level, String msg) {
  if(level > gTraceLevel)
    return;
  if (LOG_DEST == LOG_DEST_SYSLOG) {
    if (level >= LOG_DEBUG)
      level = LOG_DEBUG;
    syslog(level, "%s\n", msg.c_str());
  } else if (LOG_DEST == LOG_DEST_COUT_FILE) {
    *logOutFile.get() << msg << std::flush;
  }else {
    std::cout << msg << std::endl;
  }
}

String __getClassName(String prettyFunction) {
  size_t end   = prettyFunction.find('(');
  size_t begin = prettyFunction.find_last_of(' ', end);
  String funcName = prettyFunction.substr(begin+1,end-begin-1);
  end = funcName.find_last_of("::");
  return funcName.substr(0, end-1);
}

bool gObjStatsEnabled = false;
class ObjStats {
  public:
    ObjStats(int inc) {
      createCount_ = 0;
      deleteCount_ = 0;
      incCounter(inc);
    }
    int count() { return createCount_ - deleteCount_; }
    int createCount() { return createCount_; }
    int deleteCount() { return deleteCount_; }
    void incCounter(int inc) {
      if(inc > 0) createCount_ += inc;
      else deleteCount_ -= inc;
    }
    String getString() {
      return MkString(createCount_ << ", " << deleteCount_ << ", " << count());
    }
  private:
    int createCount_;
    int deleteCount_;
};
Map<String, ObjStats*> gObjStatsMap;

void EnableObjStats(bool val) {
  gObjStatsEnabled = val;
}

String GetObjStats(bool verbose) {
  String result = "{ ";
  Map<String, ObjStats*>::iterator iter;
  int n = 0;
  for(iter = gObjStatsMap.begin(); iter != gObjStatsMap.end(); ++iter) {
    String typeName = iter->first;
    ObjStats* stats = iter->second;
    int count = stats->count();
    if(verbose || count) {
      if(n++ > 0) result += ", ";
      result += MkString("\"" << typeName << "\" : [ " <<
                         stats->getString() << " ]");
    }
  }
  result += " }";
  return result;
}

void UpdateObjStats(String typeName, int inc) {
  Map<String, ObjStats*>::iterator iter = gObjStatsMap.find(typeName);
  if(iter != gObjStatsMap.end()) {
    ObjStats* s = iter->second;
    s->incCounter(inc);
  } else {
    gObjStatsMap.insert(Pair<String, ObjStats*>(typeName, new ObjStats(inc)));
  }
}

