#include "parser.h"
#include <stdlib.h>
#include <string.h>

void parseHexa(char* buffer,void* output,unsigned output_len){
    char number[3];
    number[2]=0;
    int i=0;
    long unsigned result;
    memset(output,0,output_len);

    while ((number[0]=*buffer++) && (number[1]=*buffer++)){
        i++;
        result=strtoul(number,NULL,16);
        if (i>output_len) break;
        *(char*)output++=result&0xFF;
    }
}