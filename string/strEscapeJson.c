#include <stdlib.h>
#include <string.h>
#include "abd/AString.c.h"

char *strEscapeJson(char *src)
{
AString	dst;
char		*ret;

	dst = CNew(AString,strlen(src) * 2);
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

	ret = aStringGetBuffer(dst);

	aStringReplaceBuffer(dst, NULL, 0);
	AStringFree(dst);

	return ret;
}

