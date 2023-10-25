#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED
#include <pthread.h>
#include <stdio.h> 

typedef struct{unsigned char buffer[16];} md5;

typedef struct{unsigned char buffer[32];} sha256;

void hashMD5(char* password,size_t len,md5* hash);

void hashSHA256(char* password,size_t len,sha256* hash);

void fileForEachLine(FILE* f,pthread_mutex_t* mutex,void (*callback)(char* line,size_t len,void* userdata),void* userdata);

void parseHexa(char* buffer,void* output,unsigned output_len);

void printHexa(unsigned char* buffer,unsigned len,FILE* stream);

#endif