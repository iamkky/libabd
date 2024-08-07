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
	unsigned int	n_rows_allocated;
	char		**cols_title;
	int		*cols_type;
	int		*cols_hash;
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
void aSTableDump(ASTable self);

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
#include "abd/util.h"
#include "abd/errLog.h"
#define USE_MUSL_SORT yes
#include "abd/sort.h"

#define ASTABLE_HASH_FREE	-1

Constructor(ASTable, int n_cols)
{
int c, *p;

	CInit(ASTable);

	self->n_rows = 0;
	self->n_rows_allocated = 0;

	self->n_cols = n_cols;
	self->n_cols_allocated = upperPowerOfTwo(self->n_cols);
	
	// Males sure there will always be at least 1/3 at extra allocated space for hash vector
	if(n_cols + (n_cols>>1) > self->n_cols_allocated) self->n_cols_allocated <<= 1;

	self->cols_title = malloc(sizeof(char *) * self->n_cols_allocated);
	self->cols_type = malloc(sizeof(int) * self->n_cols_allocated);
	self->cols_hash = malloc(sizeof(int) * self->n_cols_allocated);

	if(nullAssert(self->cols_title) || nullAssert(self->cols_type) || nullAssert(self->cols_hash)){
		if(self->cols_title) free(self->cols_title);
		if(self->cols_type) free(self->cols_type);
		if(self->cols_hash) free(self->cols_hash);
		free(self);
		return NULL;
	}
	
	c = self->n_cols_allocated;
	p = self->cols_hash;
	
	while(c--) *p++ = ASTABLE_HASH_FREE;

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

	if(self->n_rows + extension > self->n_rows_allocated){

		if(self->n_rows + extension < ASTABLE_INITIAL_SIZE)
			new_size = ASTABLE_INITIAL_SIZE;
		else
			new_size = upperPowerOfTwo(self->n_rows + extension);

		if(self->n_rows_allocated == 0)
			self->rows = malloc(sizeof(ASTableRow) * new_size);
		else
			self->rows = realloc(self->rows, sizeof(ASTableRow) * new_size);

		if(self->rows == NULL) return 0;

		for(c=self->n_rows_allocated; c<new_size; c++) self->rows[c] = NULL;
		self->n_rows_allocated = new_size;
	}

	return 1;
}

// aSTableGetColHashIndexOrFreeSlot: Search for a column title or find a free slow to add one
//
// The algorithm per si is basically a linear congruential generator (LCG), that is a well-known method for generating pseudo-random sequences.
// An LCG typically takes the form:
//
// X_(n+1) = (a * X_n + c) % m
// 
// Using parameters from CPython dictobject.c, once it has already proved good. (a = 5, c = 1)
//
// IMPORTANT: This function is inteded to private use and DOES NOT have protection agains null pointers.
// 

unsigned static aSTableGetColHashIndexOrFreeSlot(ASTable self, char *title)
{
unsigned slot, hash, pos;

	hash = fnv_32a_str(title, 0);
	
	slot = hash & (self->n_cols_allocated - 1);

	while((pos = self->cols_hash[slot]) != ASTABLE_HASH_FREE){
		if(!strcmp(self->cols_title[pos], title)) return slot;
		slot = (5 * slot  + 1) & (self->n_cols_allocated - 1);
	}

	return slot;
}

unsigned aSTableGetColIndex(ASTable self, char *title)
{
unsigned slot;
int	 pos;

	if(nullAssert(self)) return -2;
	if(nullAssert(title)) return -3;

	slot = aSTableGetColHashIndexOrFreeSlot(self, title);
	pos  = self->cols_hash[slot];

	if(pos == ASTABLE_HASH_FREE) return -1;

	return pos;
}

int aSTableSetHeader(ASTable self, int col, char *title, int type)
{  
unsigned slot;

	if(nullAssert(self)) return -2;
	if(nullAssert(title)) return -3;
	if(col>=self->n_cols) return -4;

	slot = aSTableGetColHashIndexOrFreeSlot(self,title);

	if(self->cols_hash[slot] != ASTABLE_HASH_FREE) return -1;

	self->cols_title[col] = title;
	self->cols_type[col] = type;
	self->cols_hash[slot] = col;
	
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

void aSTableDump(ASTable self)
{
int c;

	printf("Table n_cols: %d\n", self->n_cols);
	printf("Table n_cols_allocated: %d\n", self->n_cols_allocated);
	printf("Table n_rows: %d\n", self->n_rows);
	printf("Table n_rows_allocated: %d\n", self->n_rows_allocated);
	printf("\n");

	printf("Hash table:\n");
	for(c=0; c<self->n_cols_allocated; c++){
		printf(" %d:[%d]", c, self->cols_hash[c]);
	}
	printf("\n");
	printf("\n");

	printf("Table titles, rounded hash and hash content:\n");
	for(c=0; c<self->n_cols; c++){
		if(self->cols_title[c]){
			printf(" \"%s\"" , self->cols_title[c]);
			printf(" [%d]", fnv_32a_str(self->cols_title[c], 0) & (self->n_cols_allocated - 1));
			printf(" (%d)", aSTableGetColHashIndexOrFreeSlot(self, self->cols_title[c]));
		}else{
			printf(" (null)");
		}
	}
	printf("\n");
	printf("\n");

	aSTablePrint(self);
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
