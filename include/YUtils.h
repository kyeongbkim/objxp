#ifndef __YUTILS_H__
#define __YUTILS_H__

#include <vector>
#include <map>

#include <Yail.h>


namespace Yail {

// TCP Keepalive
// timeout = idle + (interval * cnt) = 85 sec
const int KEEPALIVE_IDLE     = 60;
const int KEEPALIVE_INTERVAL = 5;
const int KEEPALIVE_CNT      = 5;

String _MkYPath(String fmt, ...);

typedef Vector<String> YPathArgVector;
void ParseYPath(YPathArgVector& args, String path);

typedef Map<String, String> YPathArgMap;
void ParseYPath(YPathArgMap& args, String fmt, String path);
String GetYPathArg(YPathArgMap& args, String var);

void SetTcpKeepalive(int fd, bool on, int idle, int interval, int cnt);

}

#define MkYPath(fmt, args...) _MkYPath(fmt, args, (char*)NULL)
#define MkYPathArg(var, val) MkCString(var << "=" << val)

#endif

