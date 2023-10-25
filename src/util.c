#include "util.h"
#include <openssl/evp.h>
#include <string.h>

void hashMD5(char* password,size_t len,md5* hash){
    EVP_Digest (password, len, (unsigned char*)hash, NULL, EVP_md5(), NULL);
}

void hashSHA256(char* password,size_t len,sha256* hash){
    EVP_Digest (password, len, (unsigned char*)hash, NULL, EVP_sha256(), NULL);
}

char* safe_fgets(char* restrict s, int n, FILE* restrict stream,pthread_mutex_t* mutex) {
    char* result;

    if (mutex){
        pthread_mutex_lock(mutex);
        result=fgets(s,n,stream);
        pthread_mutex_unlock(mutex);
    }else{
        result=fgets(s,n,stream);
    }
    return result;
}

void fileForEachLine(FILE* f,pthread_mutex_t* mutex,void (*callback)(char* line,size_t len,void* userdata),void* userdata){
    char buffer[256];
    size_t len;

    while (safe_fgets(buffer,256,f,mutex)){
        len=strlen(buffer)-1;
        if (buffer[len]=='\n') buffer[len]='\0';
        else len++;

        callback(buffer,len,userdata);
    }
}

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

void printHexa(unsigned char* buffer,unsigned len,FILE* stream){
    for (unsigned i=0,j;i<len;i++){
        j=buffer[i];
        fprintf(stream,"%hhx%hhx",j>>4,j&0xF);
    }
    fputc('\n',stream);
}