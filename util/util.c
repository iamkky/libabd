#include <stdlib.h>
#include <limits.h>

// From: http://graphics.stanford.edu/%7Eseander/bithacks.html#RoundUpPowerOf2
unsigned upperPowerOfTwo(unsigned v)
{
int c;

	if(v<=2) return v;

	v--;
	for(c=1; c<(sizeof(unsigned)*8); c<<=1){
		v |= v >> c;
	}

	return v + 1;
}


//From: http://www.isthe.com/chongo/tech/comp/fnv/
//FNV is Fowler/Noll/Vo hash Functions. 

/*
--------------------------------------------------------------------------------------------------------------------
The FNV_prime is dependent on the size of the hash: (http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-param)

  32 bit FNV_prime   = 224 + 28 + 0x93  = 16777619
  64 bit FNV_prime   = 240 + 28 + 0xb3  = 1099511628211
  128 bit FNV_prime  = 288 + 28 + 0x3b  = 309485009821345068724781371

Part of the magic of FNV is the selection of the FNV_prime for a given sized unsigned integer.
Some primes do hash better than other primes for a given integer size.
The offset_basis for FNV-1 is dependent on n, the size of the hash:

  32 bit offset_basis   = 2166136261
  64 bit offset_basis   = 14695981039346656037
  128 bit offset_basis  = 144066263297769815596495629667062367629
--------------------------------------------------------------------------------------------------------------------
*/
#if UINT_MAX == 4294967295U
#define FNV_PRIME ((unsigned)0x01000193)
#elif UINT_MAX == 18446744073709551615U
#define FNV_PRIME ((unsigned)0x100000001b3U)
#else
#error "Unsupported int size"
#endif

// fnv_32a_str: Generates a string hash using FNV algorithm
// (http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-reference-source)
unsigned fnv_32a_str(const char *str, unsigned hval)
{
unsigned char *s;

	if((s = (unsigned char *)str) == NULL) return 0;
	
	while (*s) {
		hval ^= (unsigned) *s++;
		hval *= FNV_PRIME;

		// 32 bits optimization
		//hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);

		// 64 bits optimization
		//hval += (hval << 1) + (hval << 4) + (hval << 5) + (hval << 7) + (hval << 8) + (hval << 40);
	}

	return hval;
}

