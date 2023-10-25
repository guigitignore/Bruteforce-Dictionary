#include "gmode.h"
#include "lmode.h"
#include "tmode.h"
#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>

static const EVP_MD* getDigestByName(char* name){
    if (*name=='\0'){
        fputs("Using MD5 digest by default...\n",stderr);
        return EVP_md5();
    }

    const EVP_MD* algo=EVP_get_digestbyname(name);
    //default algorithm is md5
    if (algo) return algo;
    
    fprintf(stderr,"Unknown digest algorithm %s\n",name);
    exit(EXIT_FAILURE);
}


int main(int argc,char* argv[]){
    initTimer();

    if (argc>=2){
        if (*argv[1]=='G') return gmode(argc-2,argv+2,getDigestByName(argv[1]+1));
        if (*argv[1]=='L') return lmode(argc-2,argv+2);
        if (*argv[1]=='T') return tmode(getDigestByName(argv[1]+1));
    }

    fputs("Expected syntax:\n",stderr);
    fputs("G <input file>\n",stderr);
    fputs("G[digest] <input file>\n",stderr);
    fputs("L <input dictfile>\n",stderr);
    fputs("T\n",stderr);
    fputs("T[digest]\n",stderr);
    return EXIT_FAILURE;
}