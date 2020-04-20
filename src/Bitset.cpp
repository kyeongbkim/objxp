#include "Bitset.h"

namespace Yail {

void
Bitset::setBit(int pos) {
  assert(pos >= 0);
  if( (size_t)pos >= bs_.size() ) bs_.resize(pos + 1);
  bs_[pos] = true;
}

void
Bitset::clearBit(int pos) {
  assert(pos >= 0);
  if((size_t)pos < bs_.size())
    bs_[pos] = false;
}

int
Bitset::allocBit(int pos) {
  if(pos < 0) pos = 0;

  if((size_t)pos >= bs_.size()) {
    setBit(pos);
    return pos;
  } else {
    Bitset fbs;
    fbs.bs_ = bs_;
    fbs.bs_.flip();

    int idx = fbs.findNext(pos);
    if(idx == invalidPos) {
      pos = bs_.size();
      setBit(pos);
      return pos;
    } else {
      bs_[idx] = true;
      return idx;
    }
  }
}

bool
Bitset::isSet(int pos) {
  assert(pos >= 0);
  if( (size_t)pos < bs_.size() ) {
    return bs_[pos];
  }
  return false;
}

void
Bitset::reset() {
  bs_.reset();
}

int
Bitset::findNext(int pos) {
  size_t idx;
  if((pos <= 0)) {
    idx = bs_.find_first();
  } else {
    idx = bs_.find_next(pos - 1);
  }
  return (idx == boost::dynamic_bitset<>::npos) ? invalidPos : idx;
}

bool
Bitset::isEqual(Bitset& bs2) {
  int idx1 = findNext(invalidPos);
  int idx2 = bs2.findNext(invalidPos);
  while(idx1 == idx2) {
    if(idx1 == invalidPos)
      return true;
    idx1 = findNext(idx1);
    idx2 = bs2.findNext(idx2);
  }
  return false;
}

}

extern "C" {

  using namespace Yail;

  yl_bitset_t yl_bitset_alloc(void) {
      return new Bitset();
  }
  void yl_bitset_free(yl_bitset_t bs) {
      delete (Bitset*)bs;
  }
  void yl_bitset_set_bit(yl_bitset_t bs, int pos) {
      ((Bitset*)bs)->setBit(pos);
  }
  void yl_bitset_clear_bit(yl_bitset_t bs, int pos) {
      ((Bitset*)bs)->clearBit(pos);
  }
  int yl_bitset_alloc_bit(yl_bitset_t bs, int pos) {
      return ((Bitset*)bs)->allocBit(pos);
  }
  int yl_bitset_is_set(yl_bitset_t bs, int pos) {
      return ((Bitset*)bs)->isSet(pos);
  }
  int yl_bitset_find_next(yl_bitset_t bs, int pos) {
      return ((Bitset*)bs)->findNext(pos);
  }
  int yl_bitset_is_equal(yl_bitset_t bs, yl_bitset_t bs2) {
      return ((Bitset*)bs)->isEqual(*((Bitset*)bs2));
  }
};

