#include "tmode.h"
#include "timer.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

void tmodeMD5Callback(char* line,size_t len,FILE* output){
    md5 md5Hash;

    hashMD5(line,len,&md5Hash);
    printHexa((unsigned char*)&md5Hash,sizeof(md5),output);
}

void tmodeSHA256Callback(char* line,size_t len,FILE* output){
    sha256 sha256Hash;

    hashSHA256(line,len,&sha256Hash);
    printHexa((unsigned char*)&sha256Hash,sizeof(sha256),output);
}

int tmode(void (*tmode_callback)(char* line,size_t len,FILE* output)){
    fileForEachLine(stdin,NULL,(void*)tmodeMD5Callback,stdout);
    return EXIT_SUCCESS;
}