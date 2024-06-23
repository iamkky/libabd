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

static char  *ASTableTypeName[] = {"null", "undefined", "double", "integer", "charptr"};

#define ASTABLE_INITIAL_SIZE	128

typedef union {
	double	d;
	int	i;
	char	*c_ptr;
} ASTableData;

Class(ASTable) {
	unsigned int	n_cols;
	unsigned int	n_rows;
	unsigned int	rows_allocated;
	char		**cols_title;
	int		*cols_type;
	ASTableData	**rows;
};

Constructor(ASTable, int n_cols);
Destructor(ASTable);

char *aSTableGetTypeName(ASTable self, int col);

int aSTableSetValueAsDouble(ASTable self, int row, int col, double value);
int aSTableSetValueAsInteger(ASTable self, int row, int col, int value);
int aSTableSetValueAsCharPtr(ASTable self, int row, int col, char *value);

int aSTableSetHeader(ASTable self, int col, char *title, int type);
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

//ENDX

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "abd/errLog.h"

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
                        self->rows = malloc(sizeof(ASTableData *) * new_size);
                else
                        self->rows = realloc(self->rows, sizeof(ASTableData *) * new_size);

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
				switch(self->cols_type[c]){
				case ASTABLE_DOUBLE:  printf("%lf", self->rows[r][c].d);    break;
				case ASTABLE_INTEGER: printf("%d", self->rows[r][c].i);     break;
				case ASTABLE_CHARPTR: printf("%s", self->rows[r][c].c_ptr); break;
				default: printf("(unknow)"); break;
				}
			}
			if(c!=self->n_cols-1) printf(" | "); else printf("\n");
		}
	}
}
