#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "dictionary.h"

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

typedef struct{
    FILE* file;
    dictionary_hash_table_entry* hash_table;
    pthread_mutex_t file_mutex;
    pthread_mutex_t hash_table_mutex;
    pthread_mutex_t dictionary_mutex;
}dictionary_runtime_properties;

struct dictionary{
    dictionary_file_header header;
    dictionary_runtime_properties runtime;
};

void dictionaryGoToIndex(dictionary* d,uint32_t index){
    if (ftell(d->runtime.file)!=index) fseek(d->runtime.file,index,SEEK_SET);
}

void dictionaryWriteHashTable(dictionary* d){
    dictionaryGoToIndex(d,d->header.hash_table_index);
    fwrite(d->runtime.hash_table,sizeof(dictionary_hash_table_entry),d->header.mask+1,d->runtime.file);
    //truncate file to not waste disk space.
    ftruncate(fileno(d->runtime.file),ftell(d->runtime.file)+1);
}

void dictionaryWriteHeader(dictionary* d){
    rewind(d->runtime.file);
    fwrite(&d->header,sizeof(dictionary_file_header),1,d->runtime.file);
}

uint32_t dictionaryGetHashTableSize(dictionary* d){
    return (d->header.mask+1)*sizeof(dictionary_hash_table_entry);
}

void dictionaryInitFile(dictionary* d){
    d->header.magic=DICTIONARY_MAGIC;
    d->header.elements=0;
    d->header.hash_table_index=sizeof(dictionary_file_header);
    d->header.mask=0xF;

    d->runtime.hash_table=calloc(d->header.mask+1,sizeof(dictionary_hash_table_entry));
    
    //write new header
    //printf("writing new header\n");
}

dictionary* dictionaryOpen(char* filename){
    int is_already_created=!access(filename,F_OK);

    dictionary* d=malloc(sizeof(dictionary));

    if (is_already_created){

        d->runtime.file=fopen(filename,"rb+");

        fread(&d->header,sizeof(dictionary_file_header),1,d->runtime.file);

        if (d->header.magic!=DICTIONARY_MAGIC){
            fclose(d->runtime.file);
            free(d);
            return NULL;
        }

        d->runtime.hash_table=malloc(dictionaryGetHashTableSize(d));
        
        fseek(d->runtime.file,d->header.hash_table_index,SEEK_SET);
        fread(d->runtime.hash_table,sizeof(dictionary_hash_table_entry),d->header.mask+1,d->runtime.file);
    }else{
        d->runtime.file=fopen(filename,"wb+");
        dictionaryInitFile(d);
    }

    pthread_mutex_init(&d->runtime.file_mutex,NULL);
    pthread_mutex_init(&d->runtime.hash_table_mutex,NULL);
    pthread_mutex_init(&d->runtime.dictionary_mutex,NULL);

    return d;
} 

void dictionaryClose(dictionary* d){
    if (!d) return;

    dictionaryWriteHeader(d);
    dictionaryWriteHashTable(d);

    pthread_mutex_destroy(&d->runtime.file_mutex);
    pthread_mutex_destroy(&d->runtime.hash_table_mutex);
    pthread_mutex_destroy(&d->runtime.dictionary_mutex);

    fclose(d->runtime.file);
    free(d->runtime.hash_table);
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

//thread safe
void dictionaryHashTableInsert(dictionary* d,dictionary_hash_table_entry elt){
    uint32_t mask=d->header.mask;
    uint32_t perturb=elt.hash;
    uint32_t index=elt.hash&mask;
    dictionary_hash_table_entry* hte;
    dictionary_hash_table_entry* hash_table=d->runtime.hash_table;

    while(1){
        hte=hash_table+index;

        //pthread_mutex_lock(&d->runtime.hash_table_mutex);
        if (!hte->index) break;
        //pthread_mutex_unlock(&d->runtime.hash_table_mutex);

        perturb>>=DICTIONNARY_PERTURB_SHIFT;
        index*=5;
        index++;
        index+=perturb;
        index&=mask;
    }

    hte->hash=elt.hash;
    hte->index=elt.index;
    //pthread_mutex_unlock(&d->runtime.hash_table_mutex);
}

void dictionaryExtendHashTable(dictionary* d,uint8_t bits){
    dictionary_hash_table_entry *old_hash_table=d->runtime.hash_table;
    dictionary_hash_table_entry* end_old_table=old_hash_table+d->header.mask;

    d->header.mask++;
    d->header.mask<<=bits;
    

    d->runtime.hash_table=calloc(d->header.mask,sizeof(dictionary_hash_table_entry));
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
    fwrite(&de,sizeof(dictionary_data_entry),1,d->runtime.file);
    fwrite(key,key_size,1,d->runtime.file);
    fwrite(data,data_size,1,d->runtime.file);
}

//thread safe
void dictionarySafeWrite(dictionary* d,void* key,unsigned key_size,void* data,unsigned data_size){
    dictionary_hash_table_entry hte;

    pthread_mutex_lock(&d->runtime.file_mutex);
    hte.index=d->header.hash_table_index;
    dictionaryWrite(d,key,key_size,data,data_size);
    pthread_mutex_unlock(&d->runtime.file_mutex);

    hte.hash=dictionaryHash(key,key_size);

    pthread_mutex_lock(&d->runtime.hash_table_mutex);
    if ((float)d->header.elements/d->header.mask>0.66){
        printf("before resize hash table size=%d\n",d->header.elements);
        dictionaryExtendHashTable(d,1);
        printf("after resize hash table size=%d\n",d->header.elements);
    }
    dictionaryHashTableInsert(d,hte);
    pthread_mutex_unlock(&d->runtime.hash_table_mutex);

}

static uint8_t dictionaryLog2(uint32_t value){
    uint32_t result=0xFF;

    while (value){
        value>>=1;
        result++;
    }

    return result;
}


void dictionaryGenerateHashTable(dictionary* d){
    dictionary_data_entry de;
    dictionary_hash_table_entry hte;
    char buffer[256];

    uint32_t hash_table_len=1<<(dictionaryLog2((d->header.elements*3)>>1)+1);
    printf("Hash table len=%u\n",hash_table_len);

    free(d->runtime.hash_table);
    d->runtime.hash_table=calloc(hash_table_len,sizeof(dictionary_hash_table_entry));
    d->header.mask=hash_table_len-1;

    fseek(d->runtime.file,sizeof(dictionary_file_header),SEEK_SET);

    for (uint32_t i=0;i<d->header.elements;i++){
        hte.index=ftell(d->runtime.file);

        fread(&de,sizeof(dictionary_data_entry),1,d->runtime.file);
        fread(buffer,de.key_size,1,d->runtime.file);
        fseek(d->runtime.file,de.value_size,SEEK_CUR);

        hte.hash=dictionaryHash(buffer,de.key_size);
        if (i%1000000==0) printf("%d filled\n",i);
        dictionaryHashTableInsert(d,hte);
    }
}

void dictionaryAdd(dictionary* d,void* key,unsigned key_size, void* data,unsigned data_size){
    uint32_t index;
    uint32_t hash;
    uint32_t perturb;
    dictionary_hash_table_entry* hte;
    dictionary_data_entry de;
    char buffer[256];

    pthread_mutex_lock(&d->runtime.dictionary_mutex);
    if ((float)d->header.elements/d->header.mask>0.66){
        printf("before resize hash table size=%d\n",d->header.elements);
        dictionaryExtendHashTable(d,1);
        printf("after resize hash table size=%d\n",d->header.elements);
    }

    hash=dictionaryHash(key,key_size);
    index=hash&d->header.mask;
    perturb=hash;
    
    while(1){
        hte=d->runtime.hash_table+index;
        if (!hte->index) break;

        if (hte->hash==hash){
            dictionaryGoToIndex(d,hte->index);
            fread(&de,sizeof(dictionary_data_entry),1,d->runtime.file);

            fread(buffer, de.key_size,1,d->runtime.file);

            if (key_size==de.key_size && !memcmp(buffer,key,key_size)){
                //printf("key already exist in file\n");
                //we can use the same memory space to overwrite value
                if (data_size<=de.value_size){
                    //printf("reuse same place in file\n");
                    fwrite(data,data_size,1,d->runtime.file);
                    pthread_mutex_unlock(&d->runtime.dictionary_mutex);
                    return;
                }
                break;
            }

        }

        perturb>>=DICTIONNARY_PERTURB_SHIFT;
        index*=5;
        index++;
        index+=perturb;
        index&=d->header.mask;

    }

    hte->hash=hash;
    hte->index=d->header.hash_table_index;

    dictionaryWrite(d,key,key_size,data,data_size);
    pthread_mutex_unlock(&d->runtime.dictionary_mutex);
}

void dictionaryGet(dictionary* d,void* key,unsigned key_size,void** data,unsigned* data_size){
    uint32_t index;
    uint32_t hash;
    uint32_t perturb;
    dictionary_hash_table_entry* hte;
    dictionary_data_entry de;
    char buffer[256];

    hash=dictionaryHash(key,key_size);
    index=hash&d->header.mask;
    perturb=hash;
    
    while(1){
        hte=d->runtime.hash_table+index;
        if (!hte->index) break;

        if (hte->hash==hash){
            dictionaryGoToIndex(d,hte->index);
            fread(&de,sizeof(dictionary_data_entry),1,d->runtime.file);

            fread(buffer, de.key_size,1,d->runtime.file);


            if (key_size==de.key_size && !memcmp(buffer,key,key_size)){
                if (data){
                    *data=malloc(de.value_size);
                    fread(*data,de.value_size,1,d->runtime.file);
                    if (data_size) *data_size=de.value_size;
                }
                return;
            }

        }

        perturb>>=DICTIONNARY_PERTURB_SHIFT;
        index*=5;
        index++;
        index+=perturb;
        index&=d->header.mask;

    }

    if (data) *data=NULL;
    if (data_size) *data_size=0;
}