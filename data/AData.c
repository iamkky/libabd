// Implements a C representation for genetic data values.
// A data value can be a string (char *), an integer, a double, an array of data values, or an object of data values indexed by strings (char *).
//
// The type_size field is used to indicate the type of data contained in value and, in cases where the type is an object or array, the size of the object or array.
//
// The most significant 4 bits of type_size are used to indicate the type.
// The remaining bits of type_size represent the size.
//
// Removal of elemets from arrays or objects are not implemented
//

//HEADERX(../abd/AData.c.h,_ABD_AADATA_H_)
#include <inttypes.h>
#include <stdint.h>
#include <limits.h>
#include <abd/new.h>

#define TYPE_SHIFT	((sizeof(int) * CHAR_BIT) - 4)

#define TYPE_MASK	(((unsigned int)0xf) << TYPE_SHIFT)
#define SIZE_MASK	(~TYPE_MASK)

enum ADataTypes {
		ADATA_NULL	= 0  << TYPE_SHIFT,
		ADATA_UNDEFINED	= 2  << TYPE_SHIFT,
		ADATA_NUMBER	= 4  << TYPE_SHIFT,
		ADATA_BOOLEAN	= 5  << TYPE_SHIFT,
		ADATA_STRING	= 8  << TYPE_SHIFT,
		ADATA_OBJECT	= 10 << TYPE_SHIFT,
		ADATA_ARRAY	= 12 << TYPE_SHIFT
	};

Class(AData) {
	unsigned int	type_size;
	union {
		AData		*values;
		double		f;
		char *		s;
		int64_t		b;
	} value;
	char		**keys;
	unsigned int	allocated;
};

Constructor	(AData);
Destructor	(AData);

int inline static aDataGetType(AData self)
{
	if(nullAssert(self)) return ADATA_UNDEFINED;
	return self->type_size & TYPE_MASK;
}

int inline static aDataGetSize(AData self)
{
	if(nullAssert(self)) return 0;
	if(aDataGetType(self)==ADATA_ARRAY || aDataGetType(self)==ADATA_OBJECT) return self->type_size & SIZE_MASK;
	return 0;
}

void		aDataAssign(AData self, int type, void *value);
void		aDataPrint(AData self);

int		aDataInvert(AData self);
void		aDataSetNull(AData self);

int		aData(AData self);
int		aDataGetLength(AData self);
char		*aDataGetAsNumber(AData self);
char		*aDataGetAsString(AData self);
AData		aDataGet_k(AData self, const char *key);
AData		aDataGet_i(AData self, int index);
char		*aDataGet_ik(AData self, int index);
int		aDataSet_k(AData self, const char *key, AData e);
int		aDataAdd_i(AData self, AData e);

//ENDX

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "abd/printf.h"
#include "abd/errLog.h"

Constructor(AData)
{
	CInit(AData);

	self->type_size = ADATA_NULL;
	self->value.b = 0;
	self->keys = NULL;
	self->allocated = 0;

	return self;
}

Destructor(AData)
{
	if(nullAssert(self)) return;

	aDataSetNull(self);

	free(self);
}


void aDataSetNull(AData self)
{
int c;

	if(nullAssert(self)) return;

	switch(aDataGetType(self)){
	case ADATA_STRING:
		if(self->value.s) free(self->value.s);
		break;
	case ADATA_OBJECT:
		for(c=0; c<aDataGetSize(self); c++){
			if(self->keys[c]) free(self->keys[c]);
			if(self->value.values[c]) ADataFree(self->value.values[c]);
		}
		if(self->keys) free(self->keys);
		if(self->value.values) free(self->value.values);
		break;
	case ADATA_ARRAY:
		for(c=0; c<aDataGetSize(self); c++){
			if(self->value.values[c]) ADataFree(self->value.values[c]);
		}
		if(self->value.values) free(self->value.values);
		break;
	}
	
	self->type_size = ADATA_NULL;
	self->value.b = 0;
	self->keys = NULL;
	self->allocated = 0;
}

void aDataAssign(AData self, int type, void *value)
{
	if(nullAssert(self)) return;

	aDataSetNull(self);

	switch(self->type_size = type){
	case ADATA_NULL:
		self->value.b = 0;
		break;
	case ADATA_NUMBER:
		self->value.f = *((double *)value);
		break;
	case ADATA_BOOLEAN:
		self->value.b = *((int *)value);
		break;
	case ADATA_STRING:
		self->value.s = strdup((char *)value);
		break;
	case ADATA_OBJECT:
		self->allocated = 0;
		self->value.values = NULL;
		self->keys = NULL;
		break;
	case ADATA_ARRAY:
		self->allocated = 0;
		self->value.values = NULL;
		break;
	}

	return;
}

// Does not resolve duplicate keys yet
int aDataSet_k(AData self, const char *key, AData e)
{
int allocated;

	if(nullAssert(self)) return -1;
	if(nullAssert(key)) return -1;
	if(nullAssert(e)) return -1;

	if(aDataGetType(self)!=ADATA_OBJECT) return -1;

	allocated = self->allocated;

	if(self->allocated <= aDataGetSize(self)){
		if(self->allocated == 0){
			self->keys = malloc(sizeof(char *) * 32);
			self->value.values = malloc(sizeof(AData) * 32);
			self->allocated = 32;
		}else{
			self->keys = realloc(self->keys, sizeof(char *) * 2 * allocated);
			self->value.values = realloc(self->value.values, sizeof(AData) * 2 * allocated);
			self->allocated = allocated * 2;
		}
	}

	self->keys[aDataGetSize(self)] = strdup(key);
	self->value.values[aDataGetSize(self)] = e;

	// Need to check for overflow
	self->type_size += 1;

	return 0;
}

int aDataAdd_i(AData self, AData e)
{
int allocated;

	if(nullAssert(self)) return -1;
	if(aDataGetType(self)!=ADATA_ARRAY) return -1;

	allocated = self->allocated;

	if(self->allocated <= aDataGetSize(self)){
		if(self->allocated == 0){
			self->value.values = malloc(sizeof(AData) * 64);
			self->allocated = 64;
		}else{
			self->value.values = realloc(self->value.values, sizeof(AData) * 2 * allocated);
			self->allocated = allocated * 2;
		}
	}

	self->value.values[aDataGetSize(self)] = e;
	self->type_size += 1;

	return 0;
}

int aDataInvert(AData self)
{
AData tmp, *values;
char	  **keys, *ktmp;
int	  size, n;

	if(nullAssert(self)) return -1;

	if(aDataGetType(self)==ADATA_ARRAY) {
		size = aDataGetSize(self);
		values = self->value.values;

		for(n=0; n<size/2; n++){
			tmp = values[n];
			values[n] = values[size-n-1];
			values[size-n-1] = tmp;
		}

		return 0;
	}

	if(aDataGetType(self)==ADATA_OBJECT) {
		size = aDataGetSize(self);
		keys = self->keys;
		values = self->value.values;

		for(n=0; n<size/2; n++){
			tmp = values[n];
			values[n] = values[size-n-1];
			values[size-n-1] = tmp;

			ktmp = keys[n];
			keys[n] = keys[size-n-1];
			keys[size-n-1] = ktmp;
		}

		return 0;
	}

	return -1;
}

AData aDataGet_k(AData self, const char *key)
{
	if(nullAssert(self)) return NULL;
	if(nullAssert(key)) return NULL;

	if(aDataGetType(self)!=ADATA_OBJECT) return NULL;

	for(int c=0; c<aDataGetSize(self); c++){
		if(!strcmp(self->keys[c], key)){
			return self->value.values[c];
		}
	}

	return NULL;
}

char *aDataGet_ik(AData self, int index)
{
	if(nullAssert(self)) return NULL;

	if(index<0) return NULL;

	if(aDataGetType(self)==ADATA_OBJECT && index<aDataGetSize(self)) return self->keys[index];
	
	return NULL;
}

AData aDataGet_i(AData self, int index)
{
	if(nullAssert(self)) return NULL;
	if(aDataGetType(self)!=ADATA_ARRAY && aDataGetType(self)!=ADATA_OBJECT) return NULL;
	if(index<0) return NULL;

	if(index<aDataGetSize(self)) return self->value.values[index];
	
	return NULL;
}

char *aDataGetAsString(AData self)
{
char buffer[64];

	if(nullAssert(self)) return strdup("(null)");

	switch(aDataGetType(self)){
	case ADATA_NULL:
		return strdup("(null)");
	case ADATA_NUMBER:
		snprintf(buffer, 60, "%f", self->value.f);
		return strdup(buffer);
	case ADATA_BOOLEAN:
		if(self->value.b)
			return strdup("true");
		else
			return strdup("false");
	case ADATA_STRING:
		return strdup(self->value.s);
	case ADATA_OBJECT:
		return strdup("object");
	case ADATA_ARRAY:
		return strdup("array");
	}

	return strdup("undefined");
}

// Print functions;

void aDataPrint(AData self)
{
int n = 0;

	if(nullAssert(self)) return;

	switch(aDataGetType(self)){
	case ADATA_NULL:
		printf("null");
		break;
/*
	case ADATA_INTEGER:
		printf("%" PRId64, self->value.i);
		break;
*/
	case ADATA_NUMBER:
		printf("%f", self->value.f);
		break;
	case ADATA_BOOLEAN:
		printf(self->value.b ? "true" : "false");
		break;
	case ADATA_STRING:
		printf("\"%s\"", self->value.s);
		break;
	case ADATA_OBJECT:
		printf("{");
		for(n=0; n<aDataGetSize(self); n++){
			if(n>0) printf(", ");
			printf("\"%s\":", self->keys[n]);
			aDataPrint(self->value.values[n]);
		}
		printf("}");
		break;
	case ADATA_ARRAY:
		printf("[");
		for(n=0; n<aDataGetSize(self); n++){
			if(n>0) printf(", ");
			aDataPrint(self->value.values[n]);
		}
		printf("]");
		break;
	}
}

	

