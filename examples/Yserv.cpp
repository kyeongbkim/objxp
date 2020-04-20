#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <libgen.h>
#include <unistd.h>
#include <string.h>
#include <BasicIoService.h>
#include <ObexServer.h>

void usage_exit(char* progName) {
  printf("Usage: %s [-d,--daemon] [-p,--port TCP_PORT] [-h,--help] NAME\n", progName);
  exit(0);
}

using namespace Yail;

int main(int argc, char** argv) {
  int c;
  int fDaemonize = 0;
  int tcpPort = 0;
  char sockName[1024];
  unsigned int msgListThreshold = 100;
  int msgTimeout = 10;
  String logFile;
  bool writeToFile = false;

  signal(SIGPIPE, SIG_IGN);

  char* progName = basename(argv[0]);

  while(1) {
    static struct option long_options[] = {
      { "daemon",        no_argument,       0, 'd' },
      { "port",          required_argument, 0, 'p' },
      { "listthreshold", required_argument, 0, 'l' },
      { "timeout",       required_argument, 0, 't' },
      { "logFile",       required_argument, 0, 'w' },
      { "help",          no_argument,       0, 'h' },
      { 0, 0, 0, 0 }
    };

    int option_index = 0;
    c = getopt_long(argc, argv, "dp:l:t:w:h", long_options, &option_index);

    if(c < 0) break;

    switch(c) {
      case 'd':
        fDaemonize = 1;
        break;
      case 'p':
        tcpPort = atoi(optarg);
        break;
      case 'l':
        msgListThreshold = atoi(optarg);
        break;
      case 't':
        msgTimeout = atoi(optarg);
        break;
      case 'w':
        logFile = std::string(optarg);
        writeToFile = true;
        break;
      case 'h':
      default:
        usage_exit(progName);
        break;
    }
  }
  if(optind >= argc) {
    usage_exit(progName);
  }
  sprintf(sockName, "Yserv-%s", argv[optind]);

  if(fDaemonize) {
    daemon(0, 0);
  }

  if(writeToFile) {
    SetTraceLevel(LOG_INFO);
    setLogTransaction(true);
    setLogDestination(LOG_DEST_COUT_FILE, logFile);
  }

  SPtr<BasicIoService> ioSvc = CreateObject<BasicIoService>();
  SPtr<ObexServer> obexServer = CreateObject<ObexServer>(sockName, sockName, tcpPort, ioSvc, true, msgListThreshold, msgTimeout);
  ioSvc->run(0,0);
  return 0;
}

