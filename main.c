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
    dictionaryGenerateHashTable(d);
    fclose(finput);
    return d;
}

dictionary* readFromDictFile(char* dictfilename,char* inputfile){
    FILE* f;
    dictionary* d;
    char buffer[256];
    char hash[32];
    size_t len;
    char* result;

    if (inputfile){
        f=fopen(inputfile,"r");
        if (!f) return NULL;
    }else{
        f=stdin;
    }

    d=dictionaryOpenExisting(dictfilename);

    while (fgets(buffer,256,f)){
        len=strlen(buffer)-1;
        if (buffer[len]=='\n') buffer[len]='\0';
        else len++;

        len>>=1;
        parseHexa(buffer,hash,len);

        dictionaryGet(d,hash,len,(void**)&result,NULL);
        if (result){
            printf("%s\n",result);
            free(result);
        }else{
            printf("<unknown>\n");
        }
    }

    fclose(f);

    return d;
}

void exportDictKeysCallback(void* key,unsigned key_len,void* value,unsigned value_len,void* stream){
    for (int i=0;i<key_len;i++){
        unsigned char c=*(unsigned char*)(key+i);
        fprintf(stream,"%hhx%hhx",c>>4,c&0xF);
    }    
    
    fputc('\n',stream);
    free(key);
    free(value);
}

dictionary* exportDictFileKeys(char* dictfilename,char* outputfile){
    FILE* f;
    dictionary* d;

    if (outputfile){
        f=fopen(outputfile,"w");
        if (!f) return NULL;
    }else{
        f=stdout;
    }

    d=dictionaryOpenExisting(dictfilename);

    dictionaryForEach(d,exportDictKeysCallback,f);
    fclose(f);
    return d;
}


int main(int argc,char* argv[]){
    dictionary* d;
    char* inputfile;
    char* outputfile;
    size_t len;

    if (argc==1){
        puts("Expected syntax:");
        puts("-G <input file>");
        return EXIT_FAILURE;
    }

    if (argc>=3){
        if (!strcmp("-G",argv[1])){
            inputfile=argv[2];

            len=strlen(inputfile);

            outputfile=malloc(len+6);
            sprintf(outputfile,"%s.dict",inputfile);
            d=generateDictFile(inputfile,outputfile);
            free(outputfile);

            dictionaryClose(d);
        }

        if (!strcmp("-L",argv[1])){
            inputfile=argv[2];

            d=readFromDictFile(inputfile,NULL);
            dictionaryClose(d);
        }

        if (!strcmp("-E",argv[1])){
            inputfile=argv[2];

            len=strlen(inputfile);

            outputfile=malloc(len+6);
            sprintf(outputfile,"%s.keys",inputfile);
            d=exportDictFileKeys(inputfile,outputfile);
            free(outputfile);

            dictionaryClose(d);
        }
    }


    return EXIT_SUCCESS;
}