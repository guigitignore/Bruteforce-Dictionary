#include "lmode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dictionary.h"

void parseHexa(char* buffer,void* output,unsigned output_len){
    char number[3];
    number[2]=0;
    long unsigned result;
    memset(output,0,output_len);

    for (int i=0;(number[0]=*buffer++) && i<output_len;i++){
        number[1]=*buffer++;
        result=strtoul(number,NULL,16);
        *(char*)output++=result;
        if (!number[1]) break;
    }
}

void readFromDictFile(dictionary** ldict,size_t nbdict){
    char buffer[256];
    char hash[32];
    size_t len;
    char* result;



    while (fgets(buffer,256,stdin)){
        len=strlen(buffer)-1;
        if (buffer[len]=='\n') buffer[len]='\0';
        else len++;

        len>>=1;
        parseHexa(buffer,hash,len);

        for (int i=0;i<nbdict;i++){
            dictionaryGet(ldict[i],hash,len,(void**)&result,NULL);
            if (result) break;
        }

        if (result){
            puts(result);
            free(result);
        }else{
            puts("<unknown>");
        }
        
    }

}

int lmode(int argc,char* argv[]){
    if (argc==0){
        puts("Expected inputfile");
        return EXIT_FAILURE;
    }

    dictionary** ldict=malloc(sizeof(dictionary*)*argc);
    size_t nbdict=0;

    for (int i=0;i<argc;i++){
        ldict[i]=dictionaryOpenExisting(argv[i]);
        if (ldict[i]) nbdict++;
    }

    readFromDictFile(ldict,nbdict);

    for (int i=0;i<nbdict;i++){
        dictionaryClose(ldict[i]);
    }
    free(ldict);

    return EXIT_SUCCESS;
}