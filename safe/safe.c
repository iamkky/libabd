#include <stdlib.h>
#include <abd/errLog.h>

void *smalloc__(size_t size, char *file, int line)
{
void *ptr;
		
	if((ptr=malloc(size))) return ptr;

	errLogf("smalloc: %s %d failed\n", file, line);

	return NULL;
}

void sfree__(void *ptr, char *file, int line)
{
	if(ptr) {
		free(ptr);
		return;
	}

	errLogf("sfree: %s %d called with NULL\n", file, line);
}
