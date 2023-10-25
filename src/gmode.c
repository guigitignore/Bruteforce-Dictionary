#include "gmode.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include "dictionary.h"
#include "util.h"
#include "timer.h"
#include <openssl/evp.h>

#define GENERATION_THREAD_NUMBER 4
#define HASH_MAX_LEN 256

typedef struct{
    FILE* file;
    dictionary* dict;
    const EVP_MD* algo;
    pthread_mutex_t mutex;
}gmode_callback;

typedef struct{
    dictionary* dict;
    const EVP_MD* algo;
    unsigned char* buffer;
    unsigned  buffer_len;
}gmode_thread_callback;

static void dictionaryGenerateFileThreadCallback(char* line,size_t len, gmode_thread_callback* gtc){
    EVP_Digest(line,len,gtc->buffer,&gtc->buffer_len,gtc->algo,NULL);

    dictionarySafeWrite(gtc->dict,gtc->buffer,gtc->buffer_len,line,++len);
}

static void dictionaryGenerateFileThread(gmode_callback* dgt){
    gmode_thread_callback gtc;

    gtc.algo=dgt->algo;
    gtc.dict=dgt->dict;
    gtc.buffer_len=EVP_MD_get_size(dgt->algo);
    gtc.buffer=malloc(gtc.buffer_len);

    fileForEachLine(dgt->file,&dgt->mutex,(void*)dictionaryGenerateFileThreadCallback,&gtc);

    free(gtc.buffer);
}

static void generateDictFile(char* filename,const EVP_MD* algo){
    pthread_t threads[GENERATION_THREAD_NUMBER];
    FILE* finput;
    char* outputfile;
    dictionary* d;
    size_t len;
    gmode_callback dgt;

    finput=fopen(filename,"r");
    
    if (!finput) return;

    len=strlen(filename)+strlen(EVP_MD_get0_name(algo))+7;
    outputfile=malloc(len);
    snprintf(outputfile,len,"%s.%s.dict",filename,EVP_MD_get0_name(algo));
    
    printft("Creating new dictionary in \"%s\"...\n",outputfile);

    d=dictionaryNew(outputfile);
    
    dgt.dict=d;
    dgt.file=finput;
    dgt.algo=algo;

    printft("Hashing passwords of \"%s\" in %d threads\n",filename,GENERATION_THREAD_NUMBER);

    pthread_mutex_init(&dgt.mutex,NULL);

    for (int i=0;i<GENERATION_THREAD_NUMBER;i++){
        pthread_create(&threads[i],NULL,(void*)dictionaryGenerateFileThread,&dgt);
    }

    for (int i=0;i<GENERATION_THREAD_NUMBER;i++){
        pthread_join(threads[i],NULL);
    }
    
    
    pthread_mutex_destroy(&dgt.mutex);
    
    printft("%d entries has been added in dictionary \"%s\"\n",dictionaryGetSize(d),outputfile);
    printft("Generating hash table of \"%s\"...\n",outputfile);
    dictionaryGenerateHashTable(d);

    printft("Closing \"%s\"...\n",outputfile);
    dictionaryClose(d);

    fclose(finput);
    

    free(outputfile);
}


int gmode(int argc,char* argv[],const EVP_MD* algo){

    if (argc==0){
        printft("Expected input file\n");
        return EXIT_FAILURE;
    }

    printft("Using %s algorithm...\n",EVP_MD_get0_name(algo));

    for (int i=0;i<argc;i++){
        generateDictFile(argv[i],algo);
    }
    

    return EXIT_SUCCESS;
}