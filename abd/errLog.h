#ifndef _ERRLOGF_H_
#define _ERRLOGF_H_

#include <stdarg.h>

int errLogvf(const char *fmt, va_list args);
int errLogf(const char *fmt, ...);

#endif

