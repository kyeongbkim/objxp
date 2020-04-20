#include <Yail.h>

using namespace Yail;
using namespace std;

YAIL_BEGIN_CLASS_EXT(Hello, EXTENDS(YObject))
  void __new__Hello() {}
  void __del__Hello() { TRACE9("Destroying " << msg_); }
  public:
    void init(String msg) { msg_ = msg; }
    void print() {
      std::cout << msg_ << std::endl;
    }

  private:
    String msg_;
YAIL_END_CLASS

int main(int argc, char **argv) {
  SPtr<Hello> myObject = CreateObject<Hello>("Hello Yail!");
  myObject->print();

  Hello* rawPtr = myObject.get();
  rawPtr->print();

  myObject.reset();

  assert(rawPtr != NULL);
  printf("%p\n", rawPtr);

  //rawPtr->print();

  return 0;
}

