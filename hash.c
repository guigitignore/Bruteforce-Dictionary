#include "hash.h"
#include <openssl/evp.h>

void hashMD5(char* password,size_t len,md5* hash){
    EVP_Digest (password, len, (unsigned char*)hash, NULL, EVP_md5(), NULL);
}

void hashSHA256(char* password,size_t len,sha256* hash){
    EVP_Digest (password, len, (unsigned char*)hash, NULL, EVP_sha256(), NULL);
}
