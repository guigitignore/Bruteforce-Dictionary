#include "dictionary.h"
#include "hash.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

dictionary* generateDictFile(char* inputfile,char* outputfile){
    char buffer[256];
    size_t len;
    md5 hash;
    int i=0;

    FILE* finput=fopen(inputfile,"r");
    
    if (!finput) return NULL;

    dictionary* d=dictionaryOpen(outputfile);

    while (fgets(buffer,256,finput)){
        len=strlen(buffer)-1;
        if (buffer[len]=='\n') buffer[len]='\0';
        else len++;

        hashMD5(buffer,len,&hash);

        dictionaryWrite(d,&hash,sizeof(md5),buffer,len);

        i++;
        if (!(i%1000000)) printf("Writing %d passwords\n",i);
    }

    printf("Generating hash table\n");
    dictionaryGenerateHashTable(d);

    
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

            d=dictionaryOpen(inputfile);

            fgets(buffer,256,stdin);
            md5 h;
            char* result;

            parseHexa(buffer,&h,sizeof(md5));
            printMD5(&h);
            dictionaryGet(d,&h,sizeof(md5),&result,NULL);
            printf("result=%s\n",result);
            dictionaryClose(d);
        }
    }

    return EXIT_SUCCESS;
}