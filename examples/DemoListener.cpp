#include <Yail.h>
#include <ObexTree.h>
#include <ObexStringObject.h>

using namespace Yail;
using namespace std;

YAIL_BEGIN_CLASS(DemoListener, EXTENDS(YObject), IMPLEMENTS(ObexCallback))
  public:
    void init(String name) { name_ = name; }

    String callbackName() { return name_; }
    void onUpdated(String cbSrc, String path,
        SPtr<ObexObject> newObj, SPtr<ObexObject> oldObj) {
      cout << callbackName() << ": " << path << " updated" << endl;
    }
    void onDeleted(String cbSrc, String path, SPtr<ObexObject> oldObj) {
      cout << callbackName() << ": " << path << " deleted" << endl;
    }
  private:
    String name_;
YAIL_END_CLASS

int main(int argc, char** argv) {
  SetTraceLevel(0);
  SPtr<ObexTree> myTree = CreateObject<ObexTree>();

  SPtr<DemoListener> l1 = CreateObject<DemoListener>("l1");
  SPtr<DemoListener> l2 = CreateObject<DemoListener>("l2");
  SPtr<DemoListener> l3 = CreateObject<DemoListener>("l3");
  SPtr<DemoListener> l4 = CreateObject<DemoListener>("l4");

  myTree->registerCallback("/apps/demo/hello1", l1);
  myTree->registerCallback("/apps/demo/hello2", l2);
  myTree->registerCallback("/apps/demo/", l3);
  myTree->registerCallback("/*", l4);

  SPtr<ObexStringObject> demoObj =
        CreateObject<ObexStringObject>("Hello World!");

  cout << "--------" << endl;
  myTree->putObject("/apps/demo/hello1", demoObj); // trigger l1, l3, l4
  cout << "--------" << endl;
  myTree->putObject("/apps/demo/hello2", demoObj); // trigger l2, l3, l4
  cout << "--------" << endl;
  myTree->putObject("/apps/demo/hello3", demoObj); // trigger l3, l4
  cout << "--------" << endl;
  myTree->putObject("/apps/demo/hello1/world", demoObj); // trigger l4
  cout << "********" << endl;
  myTree->delObject("/apps/demo/hello1");
  cout << "--------" << endl;
  myTree->delObject("/apps/demo/hello2");
  cout << "--------" << endl;
  myTree->delObject("/apps/demo/hello3");
  cout << "--------" << endl;
  myTree->delObject("/apps/demo/hello1/world");

  return 0;
}

