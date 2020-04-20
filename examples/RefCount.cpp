#include <Yail.h>

using namespace Yail;
using namespace std;

YAIL_BEGIN_CLASS(Hello, EXTENDS(YObject))
  public:
    void init(String msg) { msg_ = msg; }
    void print() {
      std::cout << msg_ << std::endl;
    }

  private:
    String msg_;
YAIL_END_CLASS

void foo(SPtr<Hello> helloObj) {
  cout << "use_cnt: " << helloObj.use_count() << endl;
  SPtr<Hello> anotherPtr = helloObj;
  cout << "use_cnt: " << helloObj.use_count() << endl;
}

void resetTest() {
  SPtr<Hello> obj1 = CreateObject<Hello>("Hi!");
  assert(obj1.use_count() == 1);

  SPtr<Hello> obj2 = obj1;
  assert(obj1.use_count() == 2);
  assert(obj2.use_count() == 2);

  obj1.reset();
  assert(obj1.use_count() == 0);
  assert(obj2.use_count() == 1);

  obj2.reset();
  assert(obj1.use_count() == 0);
  assert(obj2.use_count() == 2);
}

int main(int argc, char **argv) {
  SPtr<Hello> myObject = CreateObject<Hello>("Hello Yail!");
  cout << "use_cnt: " << myObject.use_count() << endl;

  foo(myObject);

  myObject->print();
  cout << "use_cnt: " << myObject.use_count() << endl;

  return 0;
}

