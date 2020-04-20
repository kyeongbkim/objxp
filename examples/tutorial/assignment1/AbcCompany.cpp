#include <BasicIoService.h>
#include <ObexServer.h>
#include <ObexClient.h>

#include <AbcSales.h>
#include <AbcMfg.h>

YAIL_BEGIN_CLASS(AbcCompany, EXTENDS(YObject))
  public:
    void init(SPtr<IoService> ioSvc) {
      obexServer_ = CreateObject<ObexServer>(
          "AbcCompany", "AbcCompany", 0, ioSvc);
      obexClient_ = CreateObject<ObexClient>(
          "AbcClient", ioSvc, nullPtr(ObexConnectionCallback));

      obexClient_->connect("abcCompany", obexServer_);
      cohabSession_ = obexClient_->getSession("abcCompany");

      mfg_ = CreateObject<AbcMfg>(cohabSession_, ioSvc);
      sales_ = CreateObject<AbcSales>(cohabSession_, cohabSession_);
    }

  private:
    SPtr<ObexServer> obexServer_;
    SPtr<ObexClient> obexClient_;
    SPtr<ObexClientSession> cohabSession_;

    SPtr<AbcSales> sales_;
    SPtr<AbcMfg> mfg_;
YAIL_END_CLASS

int main(int argc, char** argv) {
  SetTraceLevel(8);
  SPtr<BasicIoService> ioSvc = CreateObject<BasicIoService>();
  SPtr<AbcCompany> abcCompany = CreateObject<AbcCompany>(ioSvc);
  ioSvc->run();
  return 0;
}

