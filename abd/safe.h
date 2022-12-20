#ifndef _LIBABD_SAFE_H_
#define _LIBABD_SAFE_H_

#include <stdlib.h>

void *smalloc__(size_t size);
void sfree__(void *ptr, char *file, int line);

#define smalloc(size)	smalloc__((size), __FILE__, __LINENO__)
#define sfree(ptr)	sfree__((ptr), __FILE__, __LINENO__)

#endif
