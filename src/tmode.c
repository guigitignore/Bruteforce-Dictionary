#include "tmode.h"
#include "timer.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct{
    const EVP_MD* algo;
    FILE* output;
    unsigned char* buffer;
    unsigned buffer_len;

}tmode_callback;

static void tmodeCallback(char* line,size_t len,tmode_callback* tcb){
    EVP_Digest(line,len,tcb->buffer,&tcb->buffer_len,tcb->algo,NULL);
    printHexa(tcb->buffer,tcb->buffer_len,tcb->output);
}

int tmode(const EVP_MD* algo){
    tmode_callback tcb;

    tcb.algo=algo;
    tcb.buffer_len=EVP_MD_get_size(algo);
    tcb.buffer=malloc(tcb.buffer_len);
    tcb.output=stdout;

    fileForEachLine(stdin,NULL,(void*)tmodeCallback,&tcb);

    free(tcb.buffer);
    return EXIT_SUCCESS;
}