#include "lmode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dictionary.h"
#include "timer.h"
#include "array.h"
#include "util.h"

typedef struct{
    unsigned buffer_max_len;
    unsigned buffer_actual_len;
    char* buffer;
    char* result;
    array* dicts;
}lmode_callback;


static void readlineCallback(dictionary** d,lmode_callback* lcb){
    if (!lcb->result) dictionaryGet(*d,lcb->buffer,lcb->buffer_actual_len,(void**)&lcb->result,NULL);
}

static void readStdinCallback(char* line,size_t len,lmode_callback* lcb){
    lcb->result=NULL;
    lcb->buffer_actual_len=len>>1;

    if (len>lcb->buffer_max_len){
        lcb->buffer=realloc(lcb->buffer,len);
        lcb->buffer_max_len=len;
    }

    parseHexa(line,lcb->buffer,lcb->buffer_actual_len);

    arrayForEach(lcb->dicts,(void*)readlineCallback,lcb);

    if (lcb->result){
        fputs(lcb->result,stdout);
        free(lcb->result);
    }
    fputc('\n',stdout);
}

static void dictsFreeCallback(dictionary** dict,void* userdata){
    dictionaryClose(*dict);
}

int lmode(int argc,char* argv[]){
    dictionary* d;
    lmode_callback lcb;

    if (argc==0){
        printft("Expected inputfile\n");
        return EXIT_FAILURE;
    }

    lcb.dicts=arrayNew(sizeof(dictionary*));
    lcb.buffer=NULL;
    lcb.buffer_max_len=0;


    for (int i=0;i<argc;i++){
        d=dictionaryOpenExisting(argv[i]);
        if (d) arrayPush(lcb.dicts,&d);
    }

    fileForEachLine(stdin,NULL,(void*)readStdinCallback,&lcb);

    arrayForEach(lcb.dicts,(void*)dictsFreeCallback,NULL);
    arrayFree(lcb.dicts);
    free(lcb.buffer);

    return EXIT_SUCCESS;
}