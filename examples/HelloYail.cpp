#include <Yail.h>

using namespace Yail;

YAIL_BEGIN_CLASS(Hello, EXTENDS(YObject))
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
    return 0;
}

