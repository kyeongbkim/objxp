#include <BasicIoService.h>
#include <ObexServer.h>
#include <ObexClient.h>

#include <AbcSales.h>
#include <AbcMfg.h>

YAIL_BEGIN_CLASS(AbcFactory, EXTENDS(YObject))
  public:
    void init(SPtr<IoService> ioSvc) {
      obexServer_ = CreateObject<ObexServer>(
          "AbcFactory", "AbcFactory", 0, ioSvc);
      obexClient_ = CreateObject<ObexClient>(
          "AbcFactory", ioSvc, nullPtr(ObexConnectionCallback));

      obexClient_->connect("mfgChannel", obexServer_);
      mfgChannel_ = obexClient_->getSession("mfgChannel");

      mfg_ = CreateObject<AbcMfg>(mfgChannel_, ioSvc);
    }

  private:
    SPtr<ObexServer> obexServer_;
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

