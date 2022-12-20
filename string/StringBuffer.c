#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <abd/printf.h>
#include <abd/errLog.h>
#include <abd/StringBuffer.h>

StringBuffer StringBufferNew(int initialSize)
{
StringBuffer self;

        if((self = malloc(sizeof(struct StringBuffer_struct))) == NULL) {
                errLogf("Malloc failed in StringBuffer\n");
                return NULL;
        }

	self->size = 0;

	if(initialSize == 0){
		self->buffer = NULL;
		self->allocated = 0;
	}else{
		if((self->buffer = malloc(initialSize)) == NULL) {
			free(self);
			errLogf("Malloc failed in StringBuffer\n");
			return NULL;
		}
		self->buffer[0] = 0;
		self->allocated = initialSize;
	}

	return self;
}

void stringBufferFree(StringBuffer self)
{
	if(self==NULL) return;

	if(self->buffer) free(self->buffer);
	free(self);
}

void stringBufferReplaceBuffer(StringBuffer self, char *buffer, int allocated)
{
	if(self==NULL) return;

	self->buffer = buffer;
	self->allocated = allocated;
	if(buffer) self->size = strlen(buffer);
}

void stringBufferHardsetLength(StringBuffer self, int len)
{
	if(self==NULL) return;
	self->size = len;
}

int stringBufferLength(StringBuffer self)
{
	if(self==NULL) return -1;
	return self->size;
}

char *stringBufferGetBuffer(StringBuffer self)
{
	if(self==NULL) return NULL;
	return self->buffer;
}

//Fixit: Can use a more sophisticated size selection to alloc or realloc.
int stringBufferCheckExpand(StringBuffer self, int extension)
{
        if(self->size + extension >= self->allocated){
                if(self->allocated == 0){
			// Alloc required space plus 128 extra bytes to avoid recorrent calls to realloc
                        self->buffer = malloc(sizeof(char *) * (extension + 128));
			if(self->buffer == NULL) return 0;
                        self->allocated = extension + 128;
                }else{
			// Alloc required space plus 50% of current size to avoid recurrent calls to realloc
                        self->buffer = realloc(self->buffer, sizeof(char *) * (self->allocated + extension + (self->size/2) + 1));
			if(self->buffer == NULL) return 0;
                        self->allocated = self->allocated + extension + (self->size/2) + 1;
                }
        }

	return 1;
}

StringBuffer stringBufferAddSb(StringBuffer self, StringBuffer str)
{
int len;

	if(!self) return NULL;
	if(!str) return NULL;

	if(stringBufferCheckExpand(self, len = stringBufferLength(str))){
		strcat(self->buffer, str->buffer);
		self->size += len;
		return self;
	}

	return NULL;
}

int stringBufferAddf(StringBuffer self, char *fmt, ... )
{
va_list	args;
int	len;

	if(self==NULL) return -1;
	if(fmt==NULL) return -1;

	va_start(args, fmt);
	len = vsnprintf(self->buffer + self->size, self->allocated - self->size, fmt, args);
	va_end(args);

	if(len >= self->allocated - self->size){
		// Overflow
		// errLogf("Overflow: %d", self->allocated);
		if(stringBufferCheckExpand(self, len)){
			va_start(args, fmt);
			len = vsnprintf(self->buffer + self->size, self->allocated - self->size, fmt, args);
			va_end(args);
		}
	}

	self->size = self->size + len;

	return 0;
}

StringBuffer stringBufferAddStr(StringBuffer self, const char *str)
{
int len;

	if(!self) return NULL;
	if(!str) return NULL;

	if(stringBufferCheckExpand(self, len = strlen(str))){
		strcat(self->buffer, str);
		self->size += len;
		return self;
	}

	return NULL;
}

StringBuffer stringBufferAddChar(StringBuffer self, char ch)
{
int len;

	if(!self) return NULL;

	if(stringBufferCheckExpand(self, 1)){
		len = strlen(self->buffer);
		self->buffer[len] = ch;
		self->buffer[len+1] = 0;
		self->size += 1;
		return self;
	}

	return NULL;
}
