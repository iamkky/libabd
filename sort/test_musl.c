int intcmp(const void *a, const void *b, const void *extra)
{
	return *((int *)a) - *((int *)b);
}

void test_musl_sort()
{
char	buffer[512];
int	list[16] = { 32,84,1,2,18,4,58,2,9,32,4,5,67,18,1,91 };

	// testing musl_sort (qsort from musl-libc)

	//qsort(list, 16, sizeof(int), intcmp);
	musl_sort(list, 16, sizeof(int), intcmp, NULL);

	buffer[0] = 0;
	for(int i=0; i<16; i++){
		snprintf(buffer+strlen(buffer), 256, "%d ", list[i]);
	}
	errLogf("LIST: %s ", buffer);
}

