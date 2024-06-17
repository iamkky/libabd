#include <stdarg.h>
#include <abd/printf.h>
#include <abd/AString.c.h>

#ifdef WASM

extern void consoleLogMsg(AString w);

#else

void consoleLogMsg(AString st)
{
	if(st && st->buffer) fprintf(stderr, "%s", st->buffer);
}

#endif

int errLogvf(const char *fmt, va_list args)
{
AString	str;

	str = CNew(AString,128);
	aStringAddvf(str, fmt, args);
/*
	if((len=vsnprintf(str->buffer, 1024, fmt, args))>=1024){
		aStringFree(str);
		str = AStringNew(len+1);
		vsnprintf(str->buffer, len + 1, fmt, args);
	};
*/
	consoleLogMsg(str);
	AStringFree(str);
	
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

