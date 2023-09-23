#ifndef _ABDLIB_STRINGBUFFER_H_
#define _ABDLIB_STRINGBUFFER_H_
#include <stdarg.h>

struct StringBuffer_struct;
typedef struct StringBuffer_struct *StringBuffer;

struct StringBuffer_struct {
        char	*buffer;
	int	size;		// meant to represent strlen(buffer). So excludes terminating null
	int	allocated;
};

StringBuffer	StringBufferNew(int initialSize);
void		stringBufferHardsetLength(StringBuffer self, int len);
int		stringBufferLength(StringBuffer w);
char		*stringBufferGetBuffer(StringBuffer w);
void		stringBufferFree(StringBuffer w);
int		stringBufferCheckExpand(StringBuffer self, int extension);
void		stringBufferReplaceBuffer(StringBuffer self, char *buffer, int allocated);
StringBuffer	stringBufferAddSb(StringBuffer self, StringBuffer str);
StringBuffer	stringBufferAddStr(StringBuffer self, const char *str);
StringBuffer	stringBufferAddChar(StringBuffer self, char ch);
int		stringBufferAddvf(StringBuffer self, const char *fmt, va_list args);
int		stringBufferAddf(StringBuffer self, const char *fmt, ...);

#endif

