#include <stdlib.h>
#include <string.h>
#include "abd/AString.c.h"

char *strEscapeJson(char *src)
{
AString	dst;
char	*ret;

	if(src==NULL) return NULL;

	dst = CNew(AString, strlen(src) * 2);
	if(dst==NULL) return NULL;

	while(*src){
		switch(*src){
		case '"' : aStringAddStr(dst, "\\\""); break;
		case '\n': aStringAddStr(dst, "\\n"); break;
		case '\t': aStringAddStr(dst, "\\t"); break;
		case '\r': aStringAddStr(dst, "\\r"); break;
		default  : aStringAddChar(dst, *src); break;
		}
		src++;
	}

	ret = aStringBufferDup(dst);

	AStringFree(dst);

	return ret;
}

