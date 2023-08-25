libabd

Small support library for C projects

Provides:
	string/StringBuffer class
	strinf/strf

Including some alternatives functions, imported from external projects, to original WASI-LIBC functions:

1. M.Paland printf lib
	To provide a compact printf implementation, that does not depend on write/read syscalls

2. Musl sort expanded to accept a extra argument to forward into cmp function
	At the time WASI-LIBC didn't provide a qsort_r


