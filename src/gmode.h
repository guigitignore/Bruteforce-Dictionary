#ifndef GMODE_H_INCLUDED
#define GMODE_H_INCLUDED
#include <openssl/evp.h>

int gmode(int argc,char* argv[],const EVP_MD* algo);

#endif