#include "gmode.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include "dictionary.h"
#include "hash.h"

#define GENERATION_THREAD_NUMBER 4

typedef struct{
    FILE* file;
    pthread_mutex_t mutex;
    dictionary* dict;
    bool sha256;
    bool md5;
}dictionary_generation_thread;

char* safe_fgets(char* restrict s, int n, FILE* restrict stream,pthread_mutex_t* mutex) {
    char* result;

    pthread_mutex_lock(mutex);
    result=fgets(s,n,stream);
    pthread_mutex_unlock(mutex);
    return result;
}

void dictionaryGenerateFileThread(dictionary_generation_thread* dgt){
    char buffer[256];
    size_t len;
    md5 md5Hash;
    sha256 sha256Hash;

    while (safe_fgets(buffer,256,dgt->file,&dgt->mutex)){
        len=strlen(buffer)-1;
        if (buffer[len]=='\n') buffer[len]='\0';
        else len++;

        if (dgt->md5){
            hashMD5(buffer,len,&md5Hash);
            dictionarySafeWrite(dgt->dict,&md5Hash,sizeof(md5),buffer,len);
        }else{
            hashSHA256(buffer,len,&sha256Hash);
            dictionarySafeWrite(dgt->dict,&sha256Hash,sizeof(sha256),buffer,len);
        }
        
    }
}

void generateDictFile(char* filename,bool md5,bool sha256){
    pthread_t threads[GENERATION_THREAD_NUMBER];

    FILE* finput=fopen(filename,"r");
    
    if (!finput) return;

    size_t len=strlen(filename);

    char* outputfile=malloc(len+6);
    sprintf(outputfile,"%s.dict",filename);
    

    dictionary* d=dictionaryNew(outputfile);
    dictionary_generation_thread dgt;

    dgt.dict=d;
    dgt.file=finput;
    dgt.md5=md5;
    dgt.sha256=sha256;

    pthread_mutex_init(&dgt.mutex,NULL);

    for (int i=0;i<GENERATION_THREAD_NUMBER;i++){
        pthread_create(&threads[i],NULL,(void*)dictionaryGenerateFileThread,&dgt);
    }

    for (int i=0;i<GENERATION_THREAD_NUMBER;i++){
        pthread_join(threads[i],NULL);
    }
    
    
    pthread_mutex_destroy(&dgt.mutex);
    
    printf("hash %d passwords\n",dictionaryGetSize(d));
    dictionaryGenerateHashTable(d);
    dictionaryClose(d);

    fclose(finput);
    

    free(outputfile);
}


int gmode(int argc,char* argv[]){
    bool sha256=false;
    bool md5=false;

    if (argc==0){
        puts("Expected input file");
        return EXIT_FAILURE;
    }

    char **files=malloc(sizeof(char*)*argc);
    size_t nbfiles=0;

    for (int i=0;i<argc;i++){
        if (*argv[i]=='-'){
            if (!strcmp("md5",argv[i]+1)){
                md5=true;
                continue;
            }
            if (!strcmp("sha256",argv[i]+1)){
                sha256=true;
                continue;
            }
            printf("Unsupported hash algorithm '%s' ! \n",argv[i]+1);
        }else{
            files[nbfiles++]=argv[i];
        }

    }

    if (!md5 && !sha256){
        md5=true;
        puts("Using MD5 hash by default");
    }

    for (int i=0;i<nbfiles;i++){
        generateDictFile(files[i],md5,sha256);
    }

    free(files);

    return EXIT_SUCCESS;
}