#ifndef _ABD_UTIL_H_
#define _ABD_UTIL_H_

// From: http://graphics.stanford.edu/%7Eseander/bithacks.html#RoundUpPowerOf2
unsigned upperPowerOfTwo(unsigned v);

// From (http://www.isthe.com/chongo/tech/comp/fnv/)
// FNV is Fowler/Noll/Vo hash Functions.
unsigned fnv_32a_str(const char *str, unsigned hval);

#endif

