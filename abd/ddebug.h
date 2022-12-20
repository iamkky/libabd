// Tracing debug toolset
//
// How to use:
//
// 1. every function should have a DSTART as the first statement
// 2. always return using DRETURN(...)
// 3. void functions should have DEND as last statement in every exection flow.
//
// examples:
//
// int f1(int a)
// {
// 	DSTART;
// 	if(a=0){
// 		// Do stuff
// 		DRETURN(1);
// 	}
// 	//Do other stuff
// 	DRETURN(a);
// }
//
// void unreturning(int a)
// {
// 	DSTART;
// 	// Do stuff
// 	DEND;
// }
//


#ifdef DDEBUG

#ifndef _ddebug_printf
#define _ddebug_printf printf
#endif

static int _ddebug_counter = 0;
static int _ddebug_prefix(int count)
{
	while(count-->0) _ddebug_printf(" ");
}

#ifndef __FUNCTION_NAME__
    #ifdef WIN32   //WINDOWS
        #define __FUNCTION_NAME__   __FUNCTION__  
    #else          //*NIX
        #define __FUNCTION_NAME__   __func__ 
    #endif
#endif

#define DPREFIX()	_ddebug_prefix(_ddebug_counter)

#define DSTART		do { _ddebug_prefix(_ddebug_counter); \
			     _ddebug_printf("Start: %s\n", __FUNCTION_NAME__); \
			     _ddebug_counter++; \
			} while(0)

#define DEND		do { _ddebug_counter--; \
			     _ddebug_prefix(_ddebug_counter); \
			     _ddebug_printf("End: %s\n", __FUNCTION_NAME__); \
			} while(0)

#define DRETURN(...)	do { DEND; return __VA_ARGS__; } while(0)

#define DRETURN_I(i)	do { _ddebug_counter--; \
			     _ddebug_prefix(_ddebug_counter); \
			     _ddebug_printf("End(%d): %s\n", (i), __FUNCTION_NAME__); \
			     return (i); \
			} while(0)

#else

#define DPREFIX(r)	do{}while(0)
#define DRETURN(r)	return (r)
#define DRETURN_I(i)	return (i)
#define DSTART		do{}while(0)
#define DEND		do{}while(0)

#endif

