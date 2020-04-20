#include <errno.h>
#include <map>
#include <cstdarg>
#include <sstream>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/tcp.h>

#include <Yail.h>
#include <YUtils.h>

namespace Yail {

String _MkYPath(String fmt, ...) {
  va_list args;
  Map<String, String> argMap;

  va_start(args, fmt);
  for(;;) {
    char* s = va_arg(args, char*);
    if(!s || !*s) break;

    Istringstream is(s);
    String var, val;
    getline(is, var, '=');
    getline(is, val, '\0');
    assert((argMap.find(var) == argMap.end()) && "Variable duplicated");
    argMap.insert(Pair<String, String>(var, val));
  }
  va_end(args);

  Ostringstream os;
  size_t curPos = 0, nextPos = 0;
  while(curPos < fmt.length()) {
    nextPos = fmt.find("${", curPos);
    if(nextPos == String::npos) {
      os << fmt.substr(curPos);
      break;
    } else { // parse variable
      String str = fmt.substr(curPos, nextPos - curPos);
      os << str;
      curPos = nextPos + 2;

      nextPos = fmt.find("}", curPos);
      assert((nextPos != String::npos) && "No matching '}'");
      String varStr = fmt.substr(curPos, nextPos - curPos);
      { // parse varStr
        String var, val;
        size_t pos = varStr.find("=");
        if(pos != String::npos) {
          var = varStr.substr(0, pos);
          val = varStr.substr(pos+1);
          assert((!var.empty() && !val.empty()) && "Bad format");
          Map<String, String>::iterator iter = argMap.find(var);
          if(iter != argMap.end()) { // found
            os << iter->second;
          } else {
            os << val;
          }
        } else {
          var = varStr;
          assert(!var.empty() && "Var empty");
          Map<String, String>::iterator iter = argMap.find(var);
          assert((iter != argMap.end()) && "Var not found");
          os << iter->second;
        }
      }
      curPos = nextPos + 1;
    }
  }

  return os.str();
}

void ParseYPath(YPathArgVector& args, String path) {
  std::istringstream is(path.substr(1));
  String s;
  while (getline(is, s, '/')) {
    args.push_back(s);
  }
}

void ParseYPath(YPathArgMap& args, String fmt, String path) {
  Istringstream isPath(path.substr(1));
  Istringstream isFmt(fmt.substr(1));
  String sPath, sFmt;
  while(true) {
    bool pathEmpty = !getline(isPath, sPath, '/');
    bool fmtEmpty = !getline(isFmt, sFmt, '/');
    if(!pathEmpty && !fmtEmpty) {
      if(sFmt.find("${") == 0) { // variable
        size_t end;
        if((end = sFmt.find("=")) == String::npos) {
          assert((end = sFmt.find("}")) != String::npos);
        }
        String var = sFmt.substr(2, end - 2);
        args.insert(Pair<String, String>(var, sPath));
      } else if(sFmt != sPath) {
        throw(ENOENT);
      }
    } else if(fmtEmpty) {
      break; // finished
    } else {
      throw(E2BIG);
    }
  }
}

String GetYPathArg(YPathArgMap& args, String var) {
  YPathArgMap::iterator iter = args.find(var);
  if(iter == args.end()) {
    throw ENOENT;
  }
  return iter->second;
}

void
SetTcpKeepalive(int fd, bool on, int idle, int interval, int cnt) {
  int optval;
  int ret = 0;

  if (fd < 0) return;

  optval = on ? 1 : 0;
  ret = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
      (const void *)&optval, sizeof(int));
  if (ret < 0) TRACE9("tcp keepalive error - " << strerror(errno));

  // if no data for idle sec
  if (idle > 0) {
    ret = setsockopt(fd, SOL_TCP, TCP_KEEPIDLE,
        (const void *)&idle, sizeof(int));
    if (ret < 0) TRACE9("tcp keepalive idle error - " << strerror(errno));
  }

  // send a syn message every interval sec
  if (interval > 0) {
    ret = setsockopt(fd, SOL_TCP, TCP_KEEPINTVL,
        (const void *)&interval, sizeof(int));
    if (ret < 0) TRACE9("tcp keepalive intvl error - " << strerror(errno));
  }

  // iterations to send
  if (cnt > 0) {
    ret = setsockopt(fd, SOL_TCP, TCP_KEEPCNT,
        (const void *)&cnt, sizeof(int));
    if (ret < 0) TRACE9("tcp keepalive cnt error - " << strerror(errno));
  }
}

}

