#include "lmode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dictionary.h"
#include "timer.h"
#include "array.h"
#include "util.h"

typedef struct{
    unsigned key_len;
    char key[32];
    char* result;
}lmode_callback;


void readlineCallback(dictionary** d,lmode_callback* lcb){
    if (!lcb->result) dictionaryGet(*d,lcb->key,lcb->key_len,(void**)&lcb->result,NULL);
}

void readStdinCallback(char* line,size_t len,array* dicts){
    lmode_callback lcb;

    lcb.result=NULL;
    lcb.key_len=len>>1;
    parseHexa(line,lcb.key,lcb.key_len);

    arrayForEach(dicts,(void*)readlineCallback,&lcb);

    if (lcb.result){
        fputs(lcb.result,stdout);
        free(lcb.result);
    }
    putchar('\n');
}

void dictsFreeCallback(dictionary** dict,void* userdata){
    dictionaryClose(*dict);
}

int lmode(int argc,char* argv[]){
    dictionary* d;
    array* dicts;

    if (argc==0){
        printft("Expected inputfile\n");
        return EXIT_FAILURE;
    }

    dicts=arrayNew(sizeof(dictionary*));


    for (int i=0;i<argc;i++){
        d=dictionaryOpenExisting(argv[i]);
        if (d) arrayPush(dicts,&d);
    }

    fileForEachLine(stdin,NULL,(void*)readStdinCallback,dicts);

    arrayForEach(dicts,(void*)dictsFreeCallback,NULL);
    arrayFree(dicts);

    return EXIT_SUCCESS;
}