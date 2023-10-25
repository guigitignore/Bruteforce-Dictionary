#include "gmode.h"
#include "lmode.h"
#include "tmode.h"
#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc,char* argv[]){
    initTimer();

    if (argc>=2){
        if (!strcmp("G",argv[1])) return gmode(argc-2,argv+2);
        if (!strcmp("L",argv[1])) return lmode(argc-2,argv+2);
        if (!strcmp("Tmd5",argv[1])) return tmode(tmodeMD5Callback);
        if (!strcmp("Tsha256",argv[1])) return tmode(tmodeSHA256Callback);
    }

    puts("Expected syntax:");
    puts("G <input file>");
    puts("L <input dictfile>");
    return EXIT_FAILURE;
}