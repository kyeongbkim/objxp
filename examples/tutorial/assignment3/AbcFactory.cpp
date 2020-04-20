#include <BasicIoService.h>
#include <ObexServer.h>
#include <ObexClient.h>

#include <AbcSales.h>
#include <AbcMfg.h>

YAIL_BEGIN_CLASS(AbcFactory, EXTENDS(YObject),
                 IMPLEMENTS(ObexConnectionCallback))
  public:
    void init(SPtr<IoService> ioSvc) {
      ioSvc_ = ioSvc;
      obexClient_ = CreateObject<ObexClient>(
          "AbcFactory", ioSvc, getThisPtr<ObexConnectionCallback>());

      obexClient_->connect("abcDC", "127.0.0.1:5555", 1000, 0);
    }

    void onConnected(String sessionKey) {
      TRACE8(sessionKey << " connected");
      mfgChannel_ = obexClient_->getSession("abcDC");
      mfg_ = CreateObject<AbcMfg>(mfgChannel_, ioSvc_);
    }
    void onConnectionFailed(String sessionKey) {
      /* never enter this routine */
    }
    void onDisconnected(String sessionKey) {
      TRACE8("Connection lost. exiting ...");
      exit(0);
    }

  private:
    SPtr<IoService> ioSvc_;
    SPtr<ObexClient> obexClient_;
    SPtr<ObexClientSession> mfgChannel_;
    SPtr<AbcMfg> mfg_;
YAIL_END_CLASS

int main(int argc, char** argv) {
  SetTraceLevel(8);
  SPtr<BasicIoService> ioSvc = CreateObject<BasicIoService>();
  SPtr<AbcFactory> abcFactory = CreateObject<AbcFactory>(ioSvc);
  ioSvc->run();
  return 0;
}

