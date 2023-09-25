#include <stdlib.h>
#include <string.h>
#include "abd/StringBuffer.h"

char *strEscapeJson(char *src)
{
StringBuffer	dst;
char		*ret;

	dst = StringBufferNew(strlen(src) * 2);
	if(dst==NULL) return NULL;

	while(*src){
		switch(*src){
		case '"' : stringBufferAddStr(dst, "\\\""); break;
		case '\n': stringBufferAddStr(dst, "\\n"); break;
		case '\t': stringBufferAddStr(dst, "\\t"); break;
		case '\r': stringBufferAddStr(dst, "\\r"); break;
		default  : stringBufferAddChar(dst, *src); break;
		}
		src++;
	}

	ret = stringBufferGetBuffer(dst);

	stringBufferReplaceBuffer(dst, NULL, 0);
	stringBufferFree(dst);

	return ret;
}

