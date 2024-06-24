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
	unsigned int	n_rows;
	unsigned int	rows_allocated;
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
#include "abd/errLog.h"
#define USE_MUSL_SORT yes
#include "abd/sort.h"

Constructor(ASTable, int n_cols)
{
	CInit(ASTable);

	self->n_cols = n_cols;
	self->n_rows = 0;
	self->rows_allocated = 0;

	self->cols_title = malloc(sizeof(char *) * n_cols);
	if(nullAssert(self->cols_title)) {
		free(self);
		return NULL;
	}

	self->cols_type = malloc(sizeof(int) * n_cols);
	if(nullAssert(self->cols_type)) {
		free(self->cols_title);
		free(self);
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

// Compares to single Data Cells, considering type
int static aSTableDataCmp(ASTableData d0, ASTableData d1, int type)
{
	switch(type){
	case ASTABLE_DOUBLE: return (int) signbit(d0.i - d1.i);
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
