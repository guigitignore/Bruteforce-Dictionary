#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "dictionary.h"
#include "array.h"

#define DICTIONARY_MAGIC 0x11223344
#define DICTIONNARY_PERTURB_SHIFT 5

typedef struct{
    uint32_t magic;
    uint32_t elements;
    uint32_t hash_table_index;
    uint32_t mask;
}dictionary_file_header;

typedef struct{
    uint32_t hash;
    uint32_t index;
}dictionary_hash_table_entry;

typedef struct{
    uint8_t key_size;
    uint8_t value_size;
}dictionary_data_entry;


struct dictionary{
    dictionary_file_header header;

    FILE* file;
    dictionary_hash_table_entry* hash_table;
    array* hash_table_cache;
    pthread_mutex_t file_mutex;
    pthread_mutex_t hash_table_mutex;
    unsigned initial_size;
};

void dictionaryGoToIndex(dictionary* d,uint32_t index){
    if (ftell(d->file)!=index) fseek(d->file,index,SEEK_SET);
}


void dictionaryInitMutex(dictionary* d){
    pthread_mutex_init(&d->file_mutex,NULL);
    pthread_mutex_init(&d->hash_table_mutex,NULL);
}

dictionary* dictionaryNew(char* filename){
    FILE* f=fopen(filename,"wb+");

    if (!f) return NULL;

    dictionary* d=malloc(sizeof(dictionary));
    d->file=f;
    
    d->header.magic=DICTIONARY_MAGIC;
    d->header.elements=0;
    d->initial_size=0;
    d->header.hash_table_index=sizeof(dictionary_file_header);
    d->header.mask=0xF;

    d->hash_table=calloc(d->header.mask+1,sizeof(dictionary_hash_table_entry));
    d->hash_table_cache=arrayNew(sizeof(dictionary_hash_table_entry));

    dictionaryInitMutex(d);

    return d;
}

dictionary* dictionaryOpenExisting(char* filename){
    FILE* f=fopen(filename,"rb+");

    if (!f) return NULL;  

    dictionary* d=malloc(sizeof(dictionary));

    fread(&d->header,sizeof(dictionary_file_header),1,f);
    if (d->header.magic!=DICTIONARY_MAGIC){
        fclose(f);
        free(d);
        return NULL;
    }

    d->file=f;
    d->hash_table=malloc((d->header.mask+1)*sizeof(dictionary_hash_table_entry));
    d->hash_table_cache=arrayNew(sizeof(dictionary_hash_table_entry));
    d->initial_size=d->header.elements;
        
    dictionaryGoToIndex(d,d->header.hash_table_index);
    fread(d->hash_table,sizeof(dictionary_hash_table_entry),d->header.mask+1,d->file); 

    dictionaryInitMutex(d);

    return d;
}


void dictionaryClose(dictionary* d){
    if (!d) return;

    if (d->initial_size!=d->header.elements){
        rewind(d->file);
        fwrite(&d->header,sizeof(dictionary_file_header),1,d->file);

        dictionaryGoToIndex(d,d->header.hash_table_index);
        fwrite(d->hash_table,sizeof(dictionary_hash_table_entry),d->header.mask+1,d->file);

        //truncate file to not waste disk space.
        ftruncate(fileno(d->file),ftell(d->file)+1);
    }
     

    pthread_mutex_destroy(&d->file_mutex);
    pthread_mutex_destroy(&d->hash_table_mutex);

    fclose(d->file);
    free(d->hash_table);
    arrayFree(d->hash_table_cache);
    free(d);
}

unsigned dictionaryGetSize(dictionary* d){
    return d->header.elements;
}

uint32_t dictionaryHash(void* data,size_t s){
    void* end_data=data+s;
    uint32_t hash=0;

    while (data<end_data){
        hash^=*(uint32_t*)data;
        data+=sizeof(uint32_t);
    }

    return hash;
}

void dictionaryHashTableInsert(dictionary* d,dictionary_hash_table_entry elt){
    uint32_t mask=d->header.mask;
    uint32_t perturb=elt.hash;
    uint32_t index=elt.hash;
    dictionary_hash_table_entry* hte;

    while(1){
        index&=mask;
        hte=d->hash_table+index;

        if (!hte->index) break;

        perturb>>=DICTIONNARY_PERTURB_SHIFT;
        index*=5;
        index++;
        index+=perturb;
        
    }

    hte->hash=elt.hash;
    hte->index=elt.index;
}

void dictionaryExtendHashTable(dictionary* d,uint8_t bits){
    dictionary_hash_table_entry *old_hash_table=d->hash_table;
    dictionary_hash_table_entry* end_old_table=old_hash_table+d->header.mask;

    d->header.mask++;
    d->header.mask<<=bits;
    

    d->hash_table=calloc(d->header.mask,sizeof(dictionary_hash_table_entry));
    d->header.mask--;


    while (end_old_table>=old_hash_table){
        if (end_old_table->index){
            dictionaryHashTableInsert(d,*end_old_table);
        }
        end_old_table--;
    }

    free(old_hash_table);
}


void dictionaryWrite(dictionary* d,void* key,unsigned key_size,void* data,unsigned data_size){
    dictionary_data_entry de;
    uint32_t hash_table_index;
    
    hash_table_index=d->header.hash_table_index;
    d->header.hash_table_index+=sizeof(dictionary_data_entry)+key_size+data_size;
    d->header.elements++;

    de.key_size=key_size;
    de.value_size=data_size;
    
    dictionaryGoToIndex(d,hash_table_index);
    fwrite(&de,sizeof(dictionary_data_entry),1,d->file);
    fwrite(key,key_size,1,d->file);
    fwrite(data,data_size,1,d->file);
}

//thread safe
void dictionarySafeWrite(dictionary* d,void* key,unsigned key_size,void* data,unsigned data_size){
    dictionary_hash_table_entry hte;

    pthread_mutex_lock(&d->file_mutex);
    hte.index=d->header.hash_table_index;
    dictionaryWrite(d,key,key_size,data,data_size);
    pthread_mutex_unlock(&d->file_mutex);

    hte.hash=dictionaryHash(key,key_size);

    //pthread_mutex_lock(&d->hash_table_mutex);
    arrayPush(d->hash_table_cache,&hte);
    //pthread_mutex_unlock(&d->hash_table_mutex);

}


void dictionaryGet(dictionary* d,void* key,unsigned key_size,void** data,unsigned* data_size){
    uint32_t index;
    uint32_t hash;
    uint32_t perturb;
    dictionary_hash_table_entry hte;
    dictionary_data_entry de;
    char buffer[256];

    hash=dictionaryHash(key,key_size);
    index=hash;
    perturb=hash;
    
    while(1){
        pthread_mutex_lock(&d->hash_table_mutex);
        index&=d->header.mask;
        hte=d->hash_table[index];
        pthread_mutex_unlock(&d->hash_table_mutex);
        if (!hte.index) break;

        if (hte.hash==hash){
            pthread_mutex_lock(&d->file_mutex);
            dictionaryGoToIndex(d,hte.index);
            fread(&de,sizeof(dictionary_data_entry),1,d->file);

            fread(buffer, de.key_size,1,d->file);
            

            if (key_size==de.key_size && !memcmp(buffer,key,key_size)){
                if (data){
                    *data=malloc(de.value_size);
                    fread(*data,de.value_size,1,d->file);
                }
                if (data_size) *data_size=de.value_size;
                pthread_mutex_unlock(&d->file_mutex);
                return;
            }
            pthread_mutex_unlock(&d->file_mutex);

        }

        perturb>>=DICTIONNARY_PERTURB_SHIFT;
        index*=5;
        index++;
        index+=perturb;
        

    }

    if (data) *data=NULL;
    if (data_size) *data_size=0;
}