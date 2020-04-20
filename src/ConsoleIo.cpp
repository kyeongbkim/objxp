#include <IoService.h>
#include <ConsoleIo.h>

namespace Yail {

void
ConsoleIoSession::init(SPtr<IoService> ioSvc) {
  IoObject::init("console", 0, ioSvc, true);
  fprintf(stdout, "%% " ); fflush(stdout);
}

void
ConsoleIoSession::handleIo(bool rdReady, bool wrReady) {
  assert(rdReady);
  assert(!wrReady);

  int nRead;
  char *line = NULL;
  size_t len = 0;

  nRead = getline(&line, &len, stdin);
  if(nRead > 0) {
    handleCommand(line);
    fprintf(stdout, "%% " ); fflush(stdout);
  } else if(nRead == 0) {
    TRACE9("empty");
  } else {
    TRACE9("terminated");
    exit(0);
  }
  free(line);
}

}

