//HEADERX(../abd/AString.c.h,_ABD_ASTRING_H_)
#include <stdarg.h>
#include <abd/new.h>

Class(AString){
	unsigned int	size;	
        char		*buffer;
	int		allocated;
};

Constructor(AString, int initialSize);
Destructor(AString);

void	aStringHardsetLength(AString self, int len);
int	aStringLength(AString w);
char	*aStringBufferDup(AString w);
char	*aStringGetBuffer(AString w);
int	aStringCheckExpand(AString self, int extension);
void	aStringReplaceBuffer(AString self, char *buffer, int allocated);

int	aStringCompare(AString self, char *src2);

AString	aStringAddSb(AString self, AString str);
AString	aStringAddStr(AString self, const char *str);
AString	aStringAddChar(AString self, char ch);
int	aStringAddvf(AString self, const char *fmt, va_list args);
int	aStringAddf(AString self, const char *fmt, ...);

//ENDX

#include <stdlib.h>
#include <string.h>
#include <abd/printf.h>
#include <abd/errLog.h>

static char *empty = "";

Constructor(AString, int initialSize)
{
	CInit(AString);

	self->size = 0;

	if(initialSize == 0){
		self->buffer = NULL;
		self->allocated = 0;
	}else{
		if((self->buffer = malloc(initialSize)) == NULL) {
			free(self);
			errLogf("Malloc failed in AString\n");
			return NULL;
		}
		self->buffer[0] = 0;
		self->allocated = initialSize;
	}

	return self;
}

Destructor(AString)
{
	if(nullAssert(self)) return;

	if(self->buffer) free(self->buffer);
	free(self);
}

void aStringReplaceBuffer(AString self, char *buffer, int allocated)
{
	if(nullAssert(self)) return;

	if(self->buffer) free(self->buffer);

	self->buffer = buffer;
	self->allocated = allocated;
	if(buffer) self->size = strlen(buffer);
}

void aStringHardsetLength(AString self, int len)
{
	if(nullAssert(self)) return;

	if(aStringCheckExpand(self, len - aStringLength(self))){
		self->buffer[len] = 0;
		self->size = len;
	}
}

int aStringLength(AString self)
{
	if(nullAssert(self)) return -1;
	return self->size;
}

char *aStringBufferDup(AString self)
{
	if(nullAssert(self)) return NULL;

	if(self->buffer==NULL) return strdup(empty);

	return strdup(self->buffer);
}

char *aStringGetBuffer(AString self)
{
	if(nullAssert(self)) return NULL;

	if(self->buffer==NULL) return empty;

	return self->buffer;
}

//Fixit: Can use a more sophisticated size selection to alloc or realloc.
int aStringCheckExpand(AString self, int extension)
{
        if(self->size + extension >= self->allocated){
                if(self->allocated == 0){
			// Alloc required space plus 128 extra bytes to avoid recorrent calls to realloc
                        self->buffer = malloc(sizeof(char) * (extension + 128));
			if(self->buffer == NULL) return 0;
                        self->allocated = extension + 128;
                }else{
			// Alloc required space plus 50% of current size to avoid recurrent calls to realloc
                        self->buffer = realloc(self->buffer, sizeof(char) * (self->allocated + extension + (self->size/2) + 1));
			if(self->buffer == NULL) return 0;
                        self->allocated = self->allocated + extension + (self->size/2) + 1;
                }
        }

	return 1;
}

int aStringCompare(AString self, char *str)
{
	if(nullAssert(self)) return 0;
	if(!str) return 0;
	if(!(self->buffer)) return 0;

	return strcmp(self->buffer, str);
}

AString aStringAddSb(AString self, AString str)
{
int len;

	if(nullAssert(self)) return NULL;
	if(!str) return NULL;

	if(aStringCheckExpand(self, len = aStringLength(str))){
		strcpy(self->buffer + self->size, str->buffer);
		self->size += len;
		return self;
	}

	return NULL;
}

int aStringAddvf(AString self, const char *fmt, va_list args )
{
int	len;

	if(nullAssert(self)) return -1;
	if(fmt==NULL) return -1;

	len = vsnprintf(self->buffer + self->size, self->allocated - self->size, fmt, args);

	if(len >= self->allocated - self->size){
		if(aStringCheckExpand(self, len)){
			len = vsnprintf(self->buffer + self->size, self->allocated - self->size, fmt, args);
		}else{
			return -1;
		}
	}

	self->size = self->size + len;

	return 0;
}

int aStringAddf(AString self, const char *fmt, ... )
{
va_list	args;
int	ret;

	va_start(args, fmt);
	ret = aStringAddvf(self, fmt, args);
	va_end(args);

	return ret;
}

AString aStringAddStr(AString self, const char *str)
{
int len;

	if(nullAssert(self)) return NULL;
	if(!str) return NULL;

	if(aStringCheckExpand(self, len = strlen(str))){
		strcpy(self->buffer + self->size, str);
		self->size += len;
		return self;
	}

	return NULL;
}

AString aStringAddChar(AString self, char ch)
{
int len;

	if(nullAssert(self)) return NULL;

	if(aStringCheckExpand(self, 1)){
		self->buffer[self->size] = ch;
		self->buffer[self->size+1] = 0;
		self->size += 1;
		return self;
	}

	return NULL;
}

