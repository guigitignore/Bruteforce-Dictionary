#include "dictionary.h"
#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void generateDictFile(char* inputfile,char* outputfile){
    char buffer[256];

    FILE* finput=fopen(inputfile,"r");
    
    if (!finput) return;

    while (fgets(buffer,256,finput)){
        
    }

    

    fclose(finput);
}


int main(int argc,char* argv[]){
    if (argc==1){
        puts("Expected syntax:");
        puts("-G <input file>");
        return EXIT_FAILURE;
    }

    if (argc>=3){
        if (!strcmp("-G",argv[1])){
            char* inputfile=argv[2];

            size_t len=strlen(inputfile);

            char* outputfile=malloc(len+6);
            sprintf(outputfile,"%s.dict",inputfile);


            free(outputfile);
        }
    }

    return EXIT_SUCCESS;
}