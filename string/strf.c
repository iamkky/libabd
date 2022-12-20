#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <abd/printf.h>

#define STRF_BUFSIZE 128

char *strf(const char *fmt, ... )
{
char     buffer[STRF_BUFSIZE];
va_list  args;
char    *temp;
int	 size;

	va_start( args, fmt );
	size = vsnprintf( buffer, STRF_BUFSIZE, fmt, args );
	va_end(args);

	if(size < STRF_BUFSIZE - 1){
		temp = strdup(buffer);
	}else{
		if((temp=malloc(size))==NULL) return NULL;
		va_start( args, fmt );
		vsnprintf(temp, size, fmt, args);
		va_end(args);
	}

	return temp;
}

