#include "dictionary.h"
#include "hash.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define GENERATION_THREAD_NUMBER 8

typedef struct{
    FILE* file;
    pthread_mutex_t mutex;
    dictionary* dict;
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
    md5 hash;

    while (safe_fgets(buffer,256,dgt->file,&dgt->mutex)){
        len=strlen(buffer)-1;
        if (buffer[len]=='\n') buffer[len]='\0';
        else len++;

        hashMD5(buffer,len,&hash);

        dictionarySafeWrite(dgt->dict,&hash,sizeof(md5),buffer,len);

        //len=dictionaryGetSize(dgt->dict);
        //if (!(len%1000000)) printf("Writing %ld passwords\n",len);

    }
}

dictionary* generateDictFile(char* inputfile,char* outputfile){

    pthread_t threads[GENERATION_THREAD_NUMBER];

    FILE* finput=fopen(inputfile,"r");
    
    if (!finput) return NULL;

    dictionary* d=dictionaryNew(outputfile);
    dictionary_generation_thread dgt;

    dgt.dict=d;
    dgt.file=finput;
    pthread_mutex_init(&dgt.mutex,NULL);

    for (int i=0;i<GENERATION_THREAD_NUMBER;i++){
        pthread_create(&threads[i],NULL,(void*)dictionaryGenerateFileThread,&dgt);
    }

    for (int i=0;i<GENERATION_THREAD_NUMBER;i++){
        pthread_join(threads[i],NULL);
    }
    

    pthread_mutex_destroy(&dgt.mutex);
    
    printf("hash %d passwords\n",dictionaryGetSize(d));
    fclose(finput);
    return d;
}


int main(int argc,char* argv[]){
    dictionary* d;
    char* inputfile;
    char buffer[256];

    if (argc==1){
        puts("Expected syntax:");
        puts("-G <input file>");
        return EXIT_FAILURE;
    }

    if (argc>=3){
        if (!strcmp("-G",argv[1])){
            inputfile=argv[2];

            size_t len=strlen(inputfile);

            char* outputfile=malloc(len+6);
            sprintf(outputfile,"%s.dict",inputfile);
            d=generateDictFile(inputfile,outputfile);
            free(outputfile);

            dictionaryClose(d);
        }

        if (!strcmp("-L",argv[1])){
            inputfile=argv[2];

            d=dictionaryOpenExisting(inputfile);

            fgets(buffer,256,stdin);
            md5 h;
            char* result;

            parseHexa(buffer,&h,sizeof(md5));
            printMD5(&h);
            dictionaryGet(d,&h,sizeof(md5),(void**)&result,NULL);
            printf("result=%s\n",result);
            dictionaryClose(d);
        }
    }


    return EXIT_SUCCESS;
}