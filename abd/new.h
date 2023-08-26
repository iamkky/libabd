#ifndef _ABDNEW_H_
#define _ABDNEW_H_

#include <stdlib.h>
#include <io/errLogf.h>

#define nullAssert(p)			((p)==NULL? (errLogf("NullException: %s[%d]: %s\n", __FILE__, __LINE__, #p), 1) : 0)

#define CSMalloc(class)			malloc(sizeof(*(class)0))

#define Class(class)			struct class##_struct; typedef struct class##_struct class##_st;typedef struct class##_struct *class; struct class##_struct
#define Destructor(class)		void class##Free(class self)
#define Constructor(class, ...)		class class##Constructor(class self, ##__VA_ARGS__) 

#define CInit(class)			do if(self==NULL) { errLogf("NUllException: Constructor: %s\n", #class); return NULL; } while(0)

#define CFree(class, self)		class##Free(self)

#ifndef DEBUG_CNEW
#define CNew(class, ...)		class##Constructor(CSMalloc(class), ##__VA_ARGS__)
#else
void static *SCMalloc_debug(size_t size, char *file, int line)
{
void *ptr;
	
	if((ptr=malloc(size))==NULL) {
		errLogf("Malloc Error: Constructor: %s:%d\n", file, line);
	}

	return ptr; 
}
#define CNew(class, ...)		class##Constructor(#class, CSMalloc_debug(sizeof(*(class)0), __FILE__, __LINE__), ##__VA_ARGS__)
#endif


#endif
