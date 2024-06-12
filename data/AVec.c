
//HEADERX(../abd/AVec.c.h,_ABD_AVEC_H_)
#include <abd/new.h>

Class(AVec) {
	int	allocated;
	int	size;
	void	**items;
	int 	(*cmp)(void *, void *);
};

Constructor(AVec);
Destructor(AVec);

int aVecAppend(AVec self, void *item);
void *aVecGetItem(AVec self, int i);

int aVecTruncate(AVec self, int size);
int aVecRemoveItem(AVec self, int i);

static inline int aVecGetSize(AVec self)
{
	return self->size;
}

//ENDX

Constructor(AVec)
{
	CInit(AVec);

	self->allocated = 0;
	self->size = 0;
	self->items = NULL;
	self->cmp = NULL;

	return self;
}

Destructor(AVec)
{
	if(CNullAssert(self)) return;
	if(self->items) free(self->items);
	free(self);
}

static int aVecResize(AVec self, int size)
{
void	**new_vec;
int	new_allocated;

	if(CNullAssert(self)) return -1;

	if(size < self->allocated) return 0;

	new_allocated = size + (size>>1);
	if(new_allocated<8) new_allocated=8;

	if(self->items==NULL){
		new_vec = malloc(new_allocated * sizeof(void **));
	}else{
		new_vec = realloc(self->items, new_allocated * sizeof(void **));
	}

	if(new_vec == NULL) return -2;

	self->items = new_vec;
	self->allocated = new_allocated;

	return 0;
}

int aVecAppend(AVec self, void *item)
{
	if(CNullAssert(self)) return -1;

	if(aVecResize(self, self->size + 1)) return -2;

	self->items[self->size] = item;
	self->size += 1;

	return 0;
}

int aVecFind(AVec self, void *item)
{
int c;

	if(CNullAssert(self)) return -1;

	if(self->cmp==NULL) return -2;

	for(c=0; c<aVecGetSize(self); c++){
		if(self->cmp(self->items[c], item)==0) return c;
	}

	return -3;
}

void *aVecGetItem(AVec self, int i)
{
	if(CNullAssert(self)) return NULL;

	if(i < self->size) return self->items[i];

	return NULL;
}

int aVecTruncate(AVec self, int size)
{
	if(nullAssert(self)) return -1;

	if(size<self->size && size>=0) self->size = size;

	return 0;
}

int aVecRemoveItem(AVec self, int i)
{
int c;

	if(nullAssert(self)) return -1;

	for(c=i; c<aVecGetSize(self)-1; c++){
		self->items[c] = self->items[c+1];
	}

	if(self->size>0) self->size -= 1;

	return 0;
}




