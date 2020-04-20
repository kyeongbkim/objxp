#ifndef __BITSET_H__
#define __BITSET_H__

typedef void* yl_bitset_t;

#ifdef __cplusplus

#include <Yail.h>
#include <iostream>
#include <boost/dynamic_bitset.hpp>

namespace Yail {

YAIL_BEGIN_CLASS(Bitset, EXTENDS(YObject))
  public:
    void init() {}
    static const int invalidPos = -1;
    void setBit(int pos);
    void clearBit(int pos);
    int  allocBit(int pos);
    bool isSet(int pos);
    void reset();
    int  findNext(int pos); // find bit greater than or equal to pos.
    bool isEqual(Bitset& bs2);

  private:
    boost::dynamic_bitset<> bs_;
YAIL_END_CLASS

}

#else

#define yl_bitset_invalid_pos (-1)

extern yl_bitset_t yl_bitset_alloc(void);
extern void yl_bitset_free(yl_bitset_t bs);
extern void yl_bitset_set_bit(yl_bitset_t bs, int pos);
extern void yl_bitset_clear_bit(yl_bitset_t bs, int pos);
extern int yl_bitset_alloc_bit(yl_bitset_t bs, int pos);
extern int yl_bitset_is_set(yl_bitset_t bs, int pos);
extern int yl_bitset_find_next(yl_bitset_t bs, int pos);
extern int yl_bitset_is_equal(yl_bitset_t bs, yl_bitset_t bs2);

#endif

#endif // __BITSET_H__
