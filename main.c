#include "gmode.h"
#include "lmode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc,char* argv[]){
    if (argc>=2){
        if (!strcmp("G",argv[1])) return gmode(argc-2,argv+2);
        if (!strcmp("L",argv[1])) return lmode(argc-2,argv+2);
    }

    puts("Expected syntax:");
    puts("G <input file>");
    puts("L <input dictfile>");
    return EXIT_FAILURE;
}