#ifndef TMODE_H_INCLUDED
#define TMODE_H_INCLUDED

#include <stdio.h>

int tmode(void (*tmode_callback)(char* line,size_t len,FILE* output));

void tmodeMD5Callback(char* line,size_t len,FILE* output);

void tmodeSHA256Callback(char* line,size_t len,FILE* output);

#endif