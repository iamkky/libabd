// Implements a C representation for table of generic values that can be:
//
// A double
// A integer
// A char *
//
// Support headers with title and type

//HEADERX(../abd/ASTable.c.h,_ABD_ASTABLE_H_)
#include <abd/new.h>

enum ASTableTypes {ASTABLE_UNDEFINED=1, ASTABLE_DOUBLE, ASTABLE_INTEGER, ASTABLE_CHARPTR, ASTABLE_NOTYPE};
enum ASTableSortOrder {ASTABLE_ASCENDANT=1, ASTABLE_DESCENDANT=-1};

static char  *ASTableTypeName[] = {"null", "undefined", "double", "integer", "charptr"};

#define ASTABLE_INITIAL_SIZE	128

typedef union {
	double	d;
	int	i;
	char	*c_ptr;
} ASTableData;

typedef ASTableData *ASTableRow;

Class(ASTable) {
	unsigned int	n_cols;
	unsigned int	n_cols_allocated;
	unsigned int	n_rows;
	unsigned int	rows_allocated;
	int		**cols_hash;
	char		**cols_title;
	int		*cols_type;
	ASTableRow	*rows;
};

Constructor(ASTable, int n_cols);
Destructor(ASTable);

char *aSTableGetTypeName(ASTable self, int col);

int aSTableSetValueAsDouble(ASTable self, int row, int col, double value);
int aSTableSetValueAsInteger(ASTable self, int row, int col, int value);
int aSTableSetValueAsCharPtr(ASTable self, int row, int col, char *value);

int aSTableSetHeader(ASTable self, int col, char *title, int type);

void aSTableDataPrint(ASTableData data, int type);
void aSTablePrint(ASTable self);

unsigned int inline static aSTableGetNRows(ASTable self)
{
	if(nullAssert(self)) return 0;
	return self->n_rows;
}

unsigned int inline static aSTableGetNCols(ASTable self)
{
	if(nullAssert(self)) return 0;
	return self->n_cols;
}

///////////////////////////////
// Utility Functions

void aSTableSort(ASTable self, int n_keys, int *keys, int order);

//ENDX

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include "abd/errLog.h"
#define USE_MUSL_SORT yes
#include "abd/sort.h"

// From: http://graphics.stanford.edu/%7Eseander/bithacks.html#RoundUpPowerOf2
unsigned static upperPowerOfTwo(unsigned v)
{
int c;

	if(v<=2) return v;

	v--;
	for(c=1; c<(sizeof(unsigned)*8); c<<=1){
		v |= v >> c;
	}
	
	return v + 1;
}

/*
http://www.isthe.com/chongo/tech/comp/fnv/

http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-param

The FNV_prime is dependent on the size of the hash:

    32 bit FNV_prime   = 224 + 28 + 0x93  = 16777619
    64 bit FNV_prime   = 240 + 28 + 0xb3  = 1099511628211
    128 bit FNV_prime  = 288 + 28 + 0x3b  = 309485009821345068724781371
    256 bit FNV_prime  = 2168 + 28 + 0x63 = 374144419156711147060143317175368453031918731002211
    512 bit FNV_prime  = 2344 + 28 + 0x57 = 35835915874844867368919076489095108449946327955754392558399825615420669938882575
                                            126094039892345713852759
    1024 bit FNV_prime = 2680 + 28 + 0x8d = 50164565101131186554345988110352789550307653454047907443030175238311120551081474
                                            51509157692220295382716162651878526895249385292291816524375083746691371804094271
                                            873160484737966720260389217684476157468082573 

Part of the magic of FNV is the selection of the FNV_prime for a given sized unsigned integer.
Some primes do hash better than other primes for a given integer size.
The offset_basis for FNV-1 is dependent on n, the size of the hash:

    32 bit offset_basis   = 2166136261
    64 bit offset_basis   = 14695981039346656037
    128 bit offset_basis  = 144066263297769815596495629667062367629
    256 bit offset_basis  = 100029257958052580907070968620625704837092796014241193945225284501741471925557
    512 bit offset_basis  = 965930312949666949800943540071631046609041874567263789610837432943446265799458293
                            2197716438449813051892206539805784495328239340083876191928701583869517785
    1024 bit offset_basis = 141977950649476210687220706414032183208806227954419339608784749146175827232522967
                            323037177221508640965212023555493656281746691085718147604710150761480297559698040
                            773201576924585630032153049571501574036444603635505054127112859663616102678680828
                            93823963790439336411086884584107735010676915 
*/

/*
From:
http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-reference-source
*/

#if UINT_MAX == 4294967295U
	#define FNV_PRIME ((unsigned)0x01000193)
#elif UINT_MAX == 18446744073709551615U
	#define FNV_PRIME ((unsigned)0x100000001b3U)
#else
	#error "Unsupported int size"
#endif

unsigned fnv_32a_str(const char *str, unsigned hval)
{
    unsigned char *s = (unsigned char *)str;

    while (*s) {
	hval ^= (unsigned) *s++;
	hval *= FNV_PRIME;

	// 32 bits optimization
	//hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);

	// 64 bits optimization
	//hval += (hval << 1) + (hval << 4) + (hval << 5) + (hval << 7) + (hval << 8) + (hval << 40);
    }

    return hval;
}

Constructor(ASTable, int n_cols)
{
int cols_allocated;

	CInit(ASTable);

	self->n_rows = 0;
	self->rows_allocated = 0;

	self->n_cols = n_cols;
	self->n_cols_allocated = upperPowerOfTwo(self->n_cols);

	self->cols_title = malloc(sizeof(char *) * self->n_cols_allocated);
	self->cols_type = malloc(sizeof(int) * self->n_cols_allocated);
	self->cols_hash = malloc(sizeof(int) * self->n_cols_allocated);

	if(nullAssert(self->cols_title) || nullAssert(self->cols_type) || nullAssert(self->cols_hash)){
		if(self->cols_title) free(self->cols_title);
		if(self->cols_type) free(self->cols_type);
		if(self->cols_hash) free(self->cols_hash);
		return NULL;
	}

	self->rows = NULL;

	return self;
}

Destructor(ASTable)
{
int c,r;

	if(nullAssert(self)) return;
	if(nullAssert(self->cols_title)) return;

	for(c=0; c<self->n_cols; c++){
		if(self->cols_title[c]) free(self->cols_title[c]);
	}

	if(self->rows){
		for(r=0; r<self->n_rows; r++){
			if(self->rows[r]){
				for(c=0; c<self->n_cols; c++){
					if(self->cols_type[c] == ASTABLE_CHARPTR && self->rows[r][c].c_ptr) free(self->rows[r][c].c_ptr);
				}
				free(self->rows[r]);
			}
		}
		free(self->rows);
	}

	free(self);
}

char *aSTableGetTypeName(ASTable self, int col)
{
int type;

	if(nullAssert(self)) return NULL;

	if(col<0 || col> self->n_cols) return "out of bound";

	type = self->cols_type[col];

	if(type>=ASTABLE_NOTYPE || type<0) return "notype";

	return ASTableTypeName[type];
}

int static aSTableCheckExpand(ASTable self, int extension)
{
int new_size, c;

        if(self->n_rows + extension > self->rows_allocated){

		if(self->n_rows + extension < ASTABLE_INITIAL_SIZE)
			new_size = ASTABLE_INITIAL_SIZE;
		else
			new_size = upperPowerOfTwo(self->n_rows + extension);

                if(self->rows_allocated == 0)
                        self->rows = malloc(sizeof(ASTableRow) * new_size);
                else
                        self->rows = realloc(self->rows, sizeof(ASTableRow) * new_size);

		if(self->rows == NULL) return 0;

		for(c=self->rows_allocated; c<new_size; c++) self->rows[c] = NULL;
                self->rows_allocated = new_size;
        }

	return 1;
}

int aSTableSetHeader(ASTable self, int col, char *title, int type)
{  
	if(nullAssert(self)) return -1;

	if(col>=self->n_cols) return -1;

	self->cols_title[col] = title;
	self->cols_type[col] = type;
	
	return 0;
}

int static aSTableCheckRow(ASTable self, int row)
{
	if(self->rows[row]==NULL){
		self->rows[row] = malloc(sizeof(ASTableData) * self->n_cols);
	}
	if(nullAssert(self->rows[row])) return 0;
	return 1;
}

int aSTableSetValueAsDouble(ASTable self, int row, int col, double value)
{
	if(nullAssert(self)) return -1;
	if(col>=self->n_cols) return -2;

	if(aSTableCheckExpand(self, (row + 1) - self->n_rows)==0) return -3;
	if(aSTableCheckRow(self, row) == 0) return -4;

	self->rows[row][col].d = value;
	if(row>=self->n_rows) self->n_rows = row + 1;
	
	return 0;
}

int aSTableSetValueAsInteger(ASTable self, int row, int col, int value)
{
	if(nullAssert(self)) return -1;
	if(col>=self->n_cols) return -2;

	if(aSTableCheckExpand(self, (row + 1) - self->n_rows)==0) return -3;
	if(aSTableCheckRow(self, row) == 0) return -4;

	self->rows[row][col].i = value;
	if(row>=self->n_rows) self->n_rows = row + 1;
	
	return 0;
}


int aSTableSetValueAsCharPtr(ASTable self, int row, int col, char *value)
{
	if(nullAssert(self)) return -1;
	if(col>=self->n_cols) return -2;

	if(aSTableCheckExpand(self, (row + 1) - self->n_rows)==0) return -3;
	if(aSTableCheckRow(self, row) == 0) return -4;

	self->rows[row][col].c_ptr = value ? strdup(value) : NULL; 
	if(row>=self->n_rows) self->n_rows = row + 1;
	
	return 0;
}

void aSTableDataPrint(ASTableData data, int type)
{
	switch(type){
	case ASTABLE_DOUBLE:  printf("%f", data.d);     break;
	case ASTABLE_INTEGER: printf("%d", data.i);     break;
	case ASTABLE_CHARPTR: printf("%s", data.c_ptr); break;
	default: printf("(unknow)"); break;
	}
}

void aSTablePrint(ASTable self)
{
int c,r;

	if(nullAssert(self)){
		printf("(null table)");
		return;
	}

	for(c=0; c<self->n_cols; c++){
		if(self->cols_title[c]!=NULL){
			printf("%s", self->cols_title[c]);
		}else{
			printf("(null)");
		}
		printf("(%s)", aSTableGetTypeName(self, c));
		if(c!=self->n_cols-1) printf(" | "); else printf("\n");
	}

	for(r=0; r<self->n_rows; r++){
		for(c=0; c<self->n_cols; c++){
			if(self->rows[r]==NULL){
				printf("(null)");
			}else{
				aSTableDataPrint(self->rows[r][c], self->cols_type[c]);
			}
			if(c!=self->n_cols-1) printf(" | "); else printf("\n");
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// ASTableSort family of functions


// Private defined here because it's only used by sort family
struct sort_key_t {
	ASTable	table;
	int	n_keys;
	int	*keys;
	int	order;
};

// Compares two single Data Cells, considering type
int static aSTableDataCmp(ASTableData d0, ASTableData d1, int type)
{
	switch(type){
	case ASTABLE_DOUBLE: return (int) signbit(d0.d - d1.d);
	case ASTABLE_INTEGER: return d0.i - d1.i;
	case ASTABLE_CHARPTR:
		if(d0.c_ptr == NULL && d1.c_ptr == NULL) return 0;
		if(d0.c_ptr == NULL)  return -1;
		if(d1.c_ptr == NULL)  return 1;
		return strcmp(d0.c_ptr, d1.c_ptr);
	}

	return 0;
}

// Sort comparator function used by qsort
// This function receives pointers to rows to compare.
int static aSTableRowCmp(const void *p0, const void *p1, const void *extra)
{
ASTableRow	*d0, *d1;
ASTable		table;
int		n_keys, *keys, order;
int		col, c, cmp;

	// Special NULL treatment: Comparison result for NULLs is still to be defined.
	// This affects the placement of undefined lines, either at the beginning or at the bottom.
	if(p0 == NULL && p1 == NULL) return 0;
	if(p0 == NULL)  return -1;
	if(p1 == NULL)  return 1;

	// Copying to local variables to improve readability
	// Hope compiler removes this
	table = ((struct sort_key_t *)extra)->table;
	n_keys = ((struct sort_key_t *)extra)->n_keys;
	keys = ((struct sort_key_t *)extra)->keys;
	order = ((struct sort_key_t *)extra)->order;

	d0 = (ASTableRow *)p0;
	d1 = (ASTableRow *)p1;

	// Traversing through keys and sorting
	for(c=0; c<n_keys; c++){
		col = keys[c];
		cmp = aSTableDataCmp((*d0)[col], (*d1)[col], table->cols_type[col]);
		if(cmp!=0) return order * cmp;
	}

	return 0;
}

void aSTableSort(ASTable self, int n_keys, int *keys, int order)
{
struct sort_key_t sort_key;

	if(nullAssert(self)) return;
	if(nullAssert(keys)) return;

	if(order!=-1 && order!=1) return;
	if(n_keys<0 || n_keys>self->n_cols) return;

	sort_key.table  = self;
	sort_key.n_keys = n_keys;
	sort_key.keys   = keys;
	sort_key.order  = order;

	qsort_r(self->rows, self->n_rows, sizeof(ASTableRow), aSTableRowCmp, &sort_key);
}
