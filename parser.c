#include "parser.h"
#include <stdlib.h>
#include <string.h>

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