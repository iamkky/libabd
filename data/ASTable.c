// Implements a C representation for table of char *

//HEADERX(../abd/ASTable.c.h,_ABD_ASTABLE_H_)
#include <abd/new.h>

enum ASTableTypes {ASTABLE_UNDEFINED=1, ASTABLE_DOUBLE, ASTABLE_INTEGER, ASTABLE_CHARPTR, ASTABLE_NOTYPE};

static char  *ASTableTypeName[] = {"null", "undefined", "double", "integer", "charptr"};

Class(ASTable) {
	unsigned int	n_cols;
	unsigned int	n_rows;
	unsigned int	rows_allocated;
	char		**header_title;
	int		*header_type;
	char		***rows;
};

Constructor(ASTable, int n_cols);
Destructor(ASTable);

char *aSTableGetTypeName(ASTable self, int col);

int aSTableSetValue(ASTable self, int row, int col, char *value);
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

	self->header_title = malloc(sizeof(char *) * n_cols);
	if(nullAssert(self->header_title)) {
		free(self);
		return NULL;
	}

	self->header_type = malloc(sizeof(int) * n_cols);
	if(nullAssert(self->header_type)) {
		free(self->header_title);
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
	if(nullAssert(self->header_title)) return;

	for(c=0; c<self->n_cols; c++){
		if(self->header_title[c]) free(self->header_title[c]);
	}

	if(self->rows){
		for(r=0; r<self->n_rows; r++){
			if(self->rows[r]){
				for(c=0; c<self->n_cols; c++){
					if(self->rows[r][c]) free(self->rows[r][c]);
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

	if(col<0 || col> self->n_cols) return "out of bound";

	type = self->header_type[col];

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

int aSTableCheckExpand(ASTable self, int extension)
{
int new_size, c;

        if(self->n_rows + extension > self->rows_allocated){
		new_size = upperPowerOfTwo(self->n_rows + extension);
                if(self->rows_allocated == 0){
                        self->rows = malloc(sizeof(char***) * new_size);
                }else{
                        self->rows = realloc(self->rows, sizeof(char ***) * new_size);
                }
		if(self->rows == NULL) return 0;
                self->rows_allocated = new_size;
		for(c=self->n_rows; c<self->rows_allocated; c++){
			self->rows[c] = NULL;
		}
        }

	return 1;
}

int aSTableSetHeader(ASTable self, int col, char *title, int type)
{
	if(col>=self->n_cols) return -1;

	self->header_title[col] = title;
	self->header_type[col] = type;
	
	return 0;
}


int aSTableSetValue(ASTable self, int row, int col, char *value)
{
	if(col>=self->n_cols) return -1;

	if(aSTableCheckExpand(self, (row + 1) - self->n_rows)){
		if(self->rows[row]==NULL){
			self->rows[row] = malloc(sizeof(char **) * self->n_cols);
		}else{
			self->rows[row] = realloc(self->rows[row], sizeof(char **) * self->n_cols);
		}
		if(nullAssert(self->rows[row])) return -1;
		self->rows[row][col] = value;
		if(row>=self->n_rows) self->n_rows = row + 1;
	}
	
	return 0;
}

void aSTablePrint(ASTable self)
{
int c,r;

	for(c=0; c<self->n_cols; c++){
		if(self->header_title[c]!=NULL){
			printf("%s", self->header_title[c]);
		}else{
			printf("(null)");
		}
		printf("(%s)", aSTableGetTypeName(self, c));
		if(c!=self->n_cols-1) printf(" | "); else printf("\n");
	}

	for(r=0; r<self->n_rows; r++){
		for(c=0; c<self->n_cols; c++){
			if(self->rows[r]!=NULL && self->rows[r][c]!=NULL){
				printf("%s", self->rows[r][c]);
			}else{
				printf("(null)");
			}
			if(c!=self->n_cols-1) printf(" | "); else printf("\n");
		}
	}
}
