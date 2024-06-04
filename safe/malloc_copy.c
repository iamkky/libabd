#include <stdlib.h>
#include <string.h>

void *malloc_copy(void *ptr, size_t size)
{
void *dst;

	if((dst = malloc(size))){
		memcpy(dst, ptr, size);
	}

	return dst;
}

