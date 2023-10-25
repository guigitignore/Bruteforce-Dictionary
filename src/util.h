#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED
#include <pthread.h>
#include <stdio.h> 

void fileForEachLine(FILE* f,pthread_mutex_t* mutex,void (*callback)(char* line,size_t len,void* userdata),void* userdata);

void parseHexa(char* buffer,void* output,unsigned output_len);

void printHexa(unsigned char* buffer,unsigned len,FILE* stream);

#endif