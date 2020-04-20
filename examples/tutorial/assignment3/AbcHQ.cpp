#include <BasicIoService.h>
#include <ObexServer.h>
#include <ObexClient.h>

#include <AbcSales.h>
#include <AbcMfg.h>

YAIL_BEGIN_CLASS(AbcHQ, EXTENDS(YObject), IMPLEMENTS(ObexConnectionCallback))
  public:
    void init(SPtr<IoService> ioSvc) {
      obexClient_ = CreateObject<ObexClient>(
          "AbcHQ", ioSvc, getThisPtr<ObexConnectionCallback>());

      obexClient_->connect("abcDC", "AbcDC", 1000, 0);
    }

    void onConnected(String sessionKey) {
      TRACE8(sessionKey << " connected");
      salesChannel_ = obexClient_->getSession("abcDC");
      mfgChannel_ = salesChannel_;
      sales_ = CreateObject<AbcSales>(salesChannel_, mfgChannel_);
    }
    void onConnectionFailed(String sessionKey) {
      /* never enter this routine */
    }
    void onDisconnected(String sessionKey) {
      TRACE8("Connection lost. exiting ...");
      exit(0);
    }

  private:
    SPtr<ObexClient> obexClient_;
    SPtr<ObexClientSession> salesChannel_;
    SPtr<ObexClientSession> mfgChannel_;

    SPtr<AbcSales> sales_;
YAIL_END_CLASS

int main(int argc, char** argv) {
  SetTraceLevel(8);
  SPtr<BasicIoService> ioSvc = CreateObject<BasicIoService>();
  SPtr<AbcHQ> abcHQ = CreateObject<AbcHQ>(ioSvc);
  ioSvc->run();
  return 0;
}

