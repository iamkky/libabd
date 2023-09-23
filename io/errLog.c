#include <stdarg.h>
#include <abd/printf.h>
#include <abd/StringBuffer.h>

#ifdef WASM

extern void consoleLogMsg(StringBuffer w);

#else

void consoleLogMsg(StringBuffer st)
{
	if(st && st->buffer) fprintf(stderr, "%s", st->buffer);
}

#endif

int errLogvf(const char *fmt, va_list args)
{
StringBuffer	str;

	str = StringBufferNew(128);
	stringBufferAddvf(str, fmt, args);
/*
	if((len=vsnprintf(str->buffer, 1024, fmt, args))>=1024){
		stringBufferFree(str);
		str = StringBufferNew(len+1);
		vsnprintf(str->buffer, len + 1, fmt, args);
	};
*/
	consoleLogMsg(str);
	stringBufferFree(str);
	
	return 0;
}

int errLogf(const char *fmt, ...)
{
va_list		args;

	va_start(args, fmt);
	errLogvf(fmt, args);
	va_end(args);
	
	return 0;
}

