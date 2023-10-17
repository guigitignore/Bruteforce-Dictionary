#ifndef HASH_H_INCLUDED
#define HASH_H_INCLUDED
#include <stddef.h>

typedef struct{unsigned char buffer[16];} md5;

typedef struct{unsigned char buffer[32];} sha256;

void hashMD5(char* password,size_t len,md5* hash);

void hashSHA256(char* password,size_t len,sha256* hash);

void printMD5(md5* hash);

#endif