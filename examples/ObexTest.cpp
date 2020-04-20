#include <ObexStringObject.h>
#include <ObexClient.h>
#include <BasicIoService.h>
#include <ConsoleIo.h>
#include <MsgIo.h>
#include <ObexServer.h>

using namespace Yail;
using namespace boost;
using namespace std;

YAIL_BEGIN_CLASS(PortInfo, EXTENDS(YObject))
  public:
    void init(String info) { info_ = info; }

    String info() { return info_; }
  private:
    String info_;
YAIL_END_CLASS

class TestConsole;

YAIL_BEGIN_CLASS(ObexTest, EXTENDS(YObject))
  public:
    void init(SPtr<IoService> ioSvc);

    void clientConnect(int argc, char** argv) {
      char* sessionName = argv[0];
      client_->connect(sessionName, "Yserv", 3000, 0);
    }
    void clientDisconnect(int argc, char** argv) {
      char* sessionName = argv[0];
      client_->disconnect(sessionName);
      client2_->disconnect(sessionName);
    }
    void clientPut(int argc, char** argv) {
      SPtr<ObexClientSession> cli = client_->getSession("s1");
      SPtr<ObexObject> oObj = CreateObject<ObexStringObject>(argv[1]);
      cli->putObject(argv[0], oObj);
    }
    void clientDel(int argc, char** argv) {
      SPtr<ObexClientSession> cli = client_->getSession(argv[0]);
      if(!cli) {
        cout << "Unknown client: " << argv[0] << endl;
        return;
      }
      cli->delObject(argv[1]);
    }
    void clientSubscribe(int argc, char** argv) {
      SPtr<ObexClientSession> cli = client_->getSession(argv[0]);
      if(!cli) cli = client2_->getSession(argv[0]);
      if(!cli) {
        cout << "Unknown client: " << argv[0] << endl;
        return;
      }
      cli->subscribe(argv[1]);
    }
    void registerCallback(int argc, char** argv) {
      SPtr<ObexClientSession> cli = client_->getSession(argv[0]);
      if(!cli) cli = client2_->getSession(argv[0]);
      if(!cli) {
        cout << "Unknown client: " << argv[0] << endl;
        return;
      }
      cli->registerCallback(argv[1], callbackObj_);
    }
    void showClientSessions(int argc, char** argv) {
      SessionSummaries sessionSummaries;
      sessionSummaries = client_->getSessionSummaries();
      for(SessionSummaries::iterator it = sessionSummaries.begin(); it != sessionSummaries.end(); ++it) {
        cout << it->first << " : " << it->second << endl;
      }
      sessionSummaries = client2_->getSessionSummaries();
      for(SessionSummaries::iterator it = sessionSummaries.begin(); it != sessionSummaries.end(); ++it) {
        cout << it->first << " : " << it->second << endl;
      }
    }
    void showServerSessions(int argc, char** argv) {
      SessionSummaries sessionSummaries = server_->getSessionSummaries();
      for(SessionSummaries::iterator it = sessionSummaries.begin(); it != sessionSummaries.end(); ++it) {
        cout << it->first << " : " << it->second << endl;
      }
    }

  private:
    SPtr<ConsoleIoSession> console_;
    SPtr<ObexServer> server_;
    SPtr<ObexConnectionCallback> connectionCallback_;
    SPtr<ObexClient> client_;
    SPtr<ObexClient> client2_;
    SPtr<ObexTree> obexTree_;
    SPtr<ObexDefaultCallback> callbackObj_;

    void processCommand(int argc, char** argv) {
      int aflag = 0, bflag = 0;
      char* cvalue = NULL;
      int c, index;

      optind = 1;
      while ((c = getopt (argc, argv, "abc:")) != -1) {
        switch (c) {
          case 'a': aflag = 1; break;
          case 'b': bflag = 1; break;
          case 'c': cvalue = optarg; break;
          case '?':
            if (optopt == 'c')
              fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
              fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
              fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
            return;
          default:
            fprintf (stderr, "Unknown option %c\n", c );
            break;
        }
      }
      printf ("aflag = %d, bflag = %d, cvalue = %s\n", aflag, bflag, cvalue);

      for (index = optind; index < argc; index++)
        printf ("Non-option argument %s\n", argv[index]);

      cout << "argc = " << argc << endl;
      for(int i = 0; i < argc; i++) {
        cout << "argv[" << i  << "] = " << argv[i] << endl;
      }
    }
YAIL_END_CLASS

YAIL_BEGIN_CLASS(TestConsole, EXTENDS(ConsoleIoSession))
  public:
    void init(SPtr<ObexTest> tester, SPtr<IoService> ioSvc) {
      ConsoleIoSession::init(ioSvc);
      tester_ = tester;
    }

  protected:
    char** buildArgv(char* line) {
      #define MAX_ARGV 100
      #define DELIM " \r\n\t"
      int i = 0;;
      char** argv = (char**)calloc(MAX_ARGV+1, sizeof(char*));
      char* token = strtok(line, DELIM);
      while(token && i < MAX_ARGV) {
        argv[i] = strdup(token);
        token = strtok(NULL, DELIM);
        i++;
      }
      return argv;
    }

    void freeArgv(char** argv) {
      int i = 0;
      while(argv[i]) {
        free(argv[i]);
        i++;
      }
      free(argv);
    }

    void handleCommand(char* line) override {
        int argc = 0;
        char** argv = buildArgv(line);
        while(argv[argc] && *argv[argc]) argc++;
        if(argc > 0) {
          if(strncmp(argv[0], "clients", strlen(argv[0])) == 0) {
            tester_->showClientSessions(argc, argv);
          } else if(strncmp(argv[0], "servers", strlen(argv[0])) == 0) {
            tester_->showServerSessions(argc, argv);
          } else if(strncmp(argv[0], "connect", strlen(argv[0])) == 0) {
            tester_->clientConnect(argc-1, argv+1);
          } else if(strncmp(argv[0], "disconnect", strlen(argv[0])) == 0) {
            tester_->clientDisconnect(argc-1, argv+1);
          } else if(strncmp(argv[0], "put", strlen(argv[0])) == 0) {
            tester_->clientPut(argc-1, argv+1);
          } else if(strncmp(argv[0], "del", strlen(argv[0])) == 0) {
            tester_->clientDel(argc-1, argv+1);
          } else if(strncmp(argv[0], "subscribe", strlen(argv[0])) == 0) {
            tester_->clientSubscribe(argc-1, argv+1);
          } else if(strncmp(argv[0], "regcb", strlen(argv[0])) == 0) {
            tester_->registerCallback(argc-1, argv+1);
          } else {
            TRACE9( "Unknown command: " << argv[0]);
          }
        }
        freeArgv(argv);
    }

  private:
    SPtr<ObexTest> tester_;
YAIL_END_CLASS

void ObexTest::init(SPtr<IoService> ioSvc) {
  SPtr<ObexTest> thisPtr = getThisPtr<ObexTest>();
  console_ = CreateObject<TestConsole>(thisPtr, ioSvc);
  connectionCallback_ = nullPtr(ObexConnectionCallback);
  client_ = CreateObject<ObexClient>("Client1", ioSvc, connectionCallback_);
  client2_ = CreateObject<ObexClient>("Client2", ioSvc, connectionCallback_);
  client_->connect("s1", "Yserv", 2000, 0);
  client2_->connect("s2", "127.0.0.1:5010", 3000, 0);
  server_ = CreateObject<ObexServer>("Yserv", "Yserv", 5010, ioSvc/*, true*/);
  callbackObj_= CreateObject<ObexDefaultCallback>("DefaultCallback");

  //client_->connect("session2", "Yserv", 2000, 0);
  //client_->connect("session3", "Yserv", 2000, 0);

#if 0
  obexTree_ = CreateObject<ObexTree>();

  SPtr<ObexCallback> oCb1= CreateObject<ObexCallback>("Cb1");
  SPtr<ObexCallback> oCb2= CreateObject<ObexCallback>("Cb2");
  SPtr<ObexCallback> oCb3= CreateObject<ObexCallback>("Cb3");
  SPtr<ObexCallback> oCb4= CreateObject<ObexCallback>("Cb4");
  SPtr<ObexCallback> oCb5= CreateObject<ObexCallback>("Cb5");
  SPtr<ObexObject> oObj1 = CreateObject<ObexStringObject>("Obj1");
  SPtr<ObexObject> oObj2 = CreateObject<ObexStringObject>("Obj2");
  SPtr<ObexObject> oObj3 = CreateObject<ObexStringObject>("Obj3");
  obexTree_->registerCallback("/", oCb1);
  obexTree_->registerCallback("/*", oCb2);
  obexTree_->registerCallback("/hardware/*", oCb2);
  obexTree_->registerCallback("/hardware/portInfo/et1", oCb3);
  obexTree_->registerCallback("/hardware/portInfo/", oCb4);
  obexTree_->registerCallback("/hardware/portInfo/*", oCb5);
  obexTree_->put("/hardware/portInfo/et1", oObj1);
  obexTree_->put("/hardware/portInfo/et2", oObj1);
  obexTree_->put("/hardware/portInfo/et2", oObj2);
  obexTree_->put("/hardware/portInfo/lag/po1", oObj1);
  obexTree_->put("/hardware/portInfo/et2/po1", oObj3);
  obexTree_->put("/hardware/portInfo", oObj2);

  String path = "/hardware/portInfo/et2/po1";
  SPtr<ObexStringObject> obj = obexTree_->get<ObexStringObject>(path);
  cout << path << " " << (obj ? obj->getString() : "null") << endl;
  obexTree_->erase(path);
  obj = obexTree_->get<ObexStringObject>(path);
  cout << path << " " << (obj ? obj->getString() : "null") << endl;
#endif
}

int main() {
  SPtr<BasicIoService> ioSvc = CreateObject<BasicIoService>();
  SPtr<ObexTest> obexTest = CreateObject<ObexTest>(ioSvc);
  ioSvc->run();
  return 0;
}

