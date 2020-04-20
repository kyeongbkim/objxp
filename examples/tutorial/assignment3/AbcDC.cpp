#include <BasicIoService.h>
#include <ObexServer.h>
#include <ObexClient.h>

#include <AbcSales.h>
#include <AbcMfg.h>

YAIL_BEGIN_CLASS(AbcDC, EXTENDS(YObject))
  public:
    void init(SPtr<IoService> ioSvc) {
      obexServer_ = CreateObject<ObexServer>(
          "AbcDC", "AbcDC", 5555, ioSvc);
    }

  private:
    SPtr<ObexServer> obexServer_;
YAIL_END_CLASS

int main(int argc, char** argv) {
  SetTraceLevel(8);
  SPtr<BasicIoService> ioSvc = CreateObject<BasicIoService>();
  SPtr<AbcDC> abcDC = CreateObject<AbcDC>(ioSvc);
  ioSvc->run();
  return 0;
}

