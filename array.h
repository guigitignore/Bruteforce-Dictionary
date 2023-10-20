#ifndef ARRAY_H_INCLUDED
#define ARRAY_H_INCLUDED

typedef struct array array;

array* arrayNew(unsigned elt_size);

unsigned arrayGetSize(array* a);

void arrayPush(array* a,void* data);

void arrayClear(array* a);

void arrayFree(array* a);

void arrayForEach(array* a,void (*callback)(void* elt,void* userdata),void* userdata);

#endif