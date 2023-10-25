#include "gmode.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include "dictionary.h"
#include "util.h"
#include "timer.h"

#define GENERATION_THREAD_NUMBER 4

typedef struct{
    FILE* file;
    pthread_mutex_t mutex;
    dictionary* dict;
    bool sha256;
    bool md5;
}gmode_callback;

void dictionaryGenerateFileThreadCallback(char* line,size_t len, gmode_callback* dgt){
    md5 md5Hash;
    sha256 sha256Hash;

    if (dgt->md5){
        hashMD5(line,len++,&md5Hash);
        dictionarySafeWrite(dgt->dict,&md5Hash,sizeof(md5),line,len);
    }
    if (dgt->sha256){
        hashSHA256(line,len++,&sha256Hash);
        dictionarySafeWrite(dgt->dict,&sha256Hash,sizeof(sha256),line,len);
    }
}

void dictionaryGenerateFileThread(gmode_callback* dgt){
    fileForEachLine(dgt->file,&dgt->mutex,(void*)dictionaryGenerateFileThreadCallback,dgt);
}

void generateDictFile(char* filename,bool md5,bool sha256){
    pthread_t threads[GENERATION_THREAD_NUMBER];

    FILE* finput=fopen(filename,"r");
    
    if (!finput) return;

    size_t len=strlen(filename);

    char* outputfile=malloc(len+6);
    sprintf(outputfile,"%s.dict",filename);
    
    printft("Creating new dictionary...\n");

    dictionary* d=dictionaryNew(outputfile);
    gmode_callback dgt;

    dgt.dict=d;
    dgt.file=finput;
    dgt.md5=md5;
    dgt.sha256=sha256;

    printft("Hashing passwords in %d threads\n",GENERATION_THREAD_NUMBER);

    pthread_mutex_init(&dgt.mutex,NULL);

    for (int i=0;i<GENERATION_THREAD_NUMBER;i++){
        pthread_create(&threads[i],NULL,(void*)dictionaryGenerateFileThread,&dgt);
    }

    for (int i=0;i<GENERATION_THREAD_NUMBER;i++){
        pthread_join(threads[i],NULL);
    }
    
    
    pthread_mutex_destroy(&dgt.mutex);
    
    printft("%d entries has been added in dictionary\n",dictionaryGetSize(d));
    printft("Generating hash table...\n");
    dictionaryGenerateHashTable(d);

    printft("Finished!\n");
    dictionaryClose(d);

    fclose(finput);
    

    free(outputfile);
}


int gmode(int argc,char* argv[]){
    bool sha256=false;
    bool md5=false;

    if (argc==0){
        printft("Expected input file\n");
        return EXIT_FAILURE;
    }

    char **files=malloc(sizeof(char*)*argc);
    size_t nbfiles=0;

    for (int i=0;i<argc;i++){
        if (*argv[i]=='-'){
            if (!strcmp("md5",argv[i]+1)){
                md5=true;
                printft("Using MD5 hash...\n");
                continue;
            }
            if (!strcmp("sha256",argv[i]+1)){
                sha256=true;
                printft("Using SHA256 hash...\n");
                continue;
            }
            printft("Unsupported hash algorithm '%s' ! \n",argv[i]+1);
        }else{
            files[nbfiles++]=argv[i];
        }

    }

    if (!md5 && !sha256){
        md5=true;
        printft("Using MD5 hash by default...\n");
    }

    for (int i=0;i<nbfiles;i++){
        generateDictFile(files[i],md5,sha256);
    }

    free(files);

    return EXIT_SUCCESS;
}