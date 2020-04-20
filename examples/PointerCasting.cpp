#include <Yail.h>

using namespace Yail;
using namespace std;

YAIL_BEGIN_CLASS(Parent, EXTENDS(YObject))
  public:
    void init() {}
YAIL_END_CLASS

YAIL_BEGIN_CLASS(Child, EXTENDS(Parent))
  public:
    void init() {}
YAIL_END_CLASS

YAIL_BEGIN_CLASS(Other, EXTENDS(YObject))
  public:
    void init() {}
YAIL_END_CLASS

int main(int argc, char **argv) {
  SPtr<Parent> parent = CreateObject<Parent>();
  SPtr<Child>  child = CreateObject<Child>();
  SPtr<Other>  other = CreateObject<Other>();

  SPtr<Parent> tmpParent;
  SPtr<Child>  tmpChild;
  SPtr<Other>  tmpOther;

  tmpParent = child; assert(tmpParent);
  //tmpParent = other;
  //tmpChild = parent;
  //tmpChild = other;
  //tmpOther = parent;
  //tmpOther = child;

  tmpParent = StaticPointerCast<Parent>(child); assert(tmpParent);
  //tmpParent = StaticPointerCast<Parent>(other);
  tmpChild  = StaticPointerCast<Child>(parent); assert(tmpChild);
  //tmpChild  = StaticPointerCast<Child>(other);
  //tmpOther  = StaticPointerCast<Other>(parent);
  //tmpOther  = StaticPointerCast<Other>(child);

  tmpParent = DynamicPointerCast<Parent>(child);    assert(tmpParent);
  tmpChild  = DynamicPointerCast<Child>(tmpParent); assert(tmpChild);
  tmpChild  = DynamicPointerCast<Child>(parent);    assert(!tmpChild);
  tmpParent = DynamicPointerCast<Parent>(other);    assert(!tmpParent);
  tmpChild  = DynamicPointerCast<Child>(other);     assert(!tmpChild);
  tmpOther  = DynamicPointerCast<Other>(parent);    assert(!tmpOther);
  tmpOther  = DynamicPointerCast<Other>(child);     assert(!tmpOther);

  return 0;
}

