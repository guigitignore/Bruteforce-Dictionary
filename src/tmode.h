#ifndef TMODE_H_INCLUDED
#define TMODE_H_INCLUDED
#include <openssl/evp.h>

int tmode(const EVP_MD* algo);

#endif