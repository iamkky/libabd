#ifndef _LIBABD_SORT_H_
#define _LIBABD_SORT_H_

void __musl_sort_extra(void *, size_t, size_t, int (*)(const void *, const void *, const void *), const void *extra);
#define qsort_r __musl_sort_extra

#endif
