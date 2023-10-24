#include "array.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define ARRAY_MIN_SIZE 16

struct array{
    void* base_ptr;
    void* current_ptr;
    void* end_ptr;
    pthread_mutex_t mutex;
    unsigned elt_size;
};

void arrayInit(array* a){
    unsigned total_size=a->elt_size*ARRAY_MIN_SIZE;
    a->base_ptr=realloc(a->base_ptr,total_size);
    a->current_ptr=a->base_ptr;
    a->end_ptr=a->base_ptr+total_size;
}


array* arrayNew(unsigned elt_size){
    array* a=malloc(sizeof(array));
    a->base_ptr=NULL;
    a->elt_size=elt_size;
    pthread_mutex_init(&a->mutex,NULL);

    arrayInit(a);

    return a;
}

unsigned arrayGetSize(array* a){
    return (a->current_ptr-a->base_ptr)/a->elt_size;
}

void arrayResize(array* a){
    unsigned total_size,current_size;

    total_size=a->end_ptr-a->base_ptr;
    current_size=a->current_ptr-a->base_ptr;

    total_size<<=1;
    a->base_ptr=realloc(a->base_ptr,total_size);
    a->current_ptr=a->base_ptr+current_size;
    a->end_ptr=a->base_ptr+total_size;
}

void arrayPush(array* a,void* data){
    pthread_mutex_lock(&a->mutex);
    if (a->current_ptr==a->end_ptr) arrayResize(a);

    memcpy(a->current_ptr,data,a->elt_size);
    a->current_ptr+=a->elt_size;
    pthread_mutex_unlock(&a->mutex);
}

void arrayClear(array* a){
    pthread_mutex_lock(&a->mutex);
    arrayInit(a);
    pthread_mutex_unlock(&a->mutex);
}

void arrayFree(array* a){
    free(a->base_ptr);
    pthread_mutex_destroy(&a->mutex);
    free(a);
}

void arrayForEach(array* a,void (*callback)(void* elt,void* userdata),void* userdata){
    pthread_mutex_lock(&a->mutex);
    unsigned elt_size=a->elt_size;
    void* base_ptr=a->base_ptr;
    void* end_ptr=a->current_ptr;

    while(base_ptr<end_ptr){
        callback(base_ptr,userdata);
        base_ptr+=elt_size;
    }
    pthread_mutex_unlock(&a->mutex);
}