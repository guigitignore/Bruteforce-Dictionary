#include "timer.h"
#include <stdio.h>
#include <sys/time.h>

static struct timeval initialTime;

void initTimer(){
    gettimeofday(&initialTime,NULL);
}

int vfprintft(FILE* restrict stream,const char* restrict format,va_list va){
    struct timeval currentTime;
    double elapsedTime;

    gettimeofday(&currentTime, NULL);
    elapsedTime = (currentTime.tv_sec - initialTime.tv_sec); 
    elapsedTime += (currentTime.tv_usec - initialTime.tv_usec) / 1000000.0;   // us to s
    
    fprintf(stream,"[%lf] ",elapsedTime);
    return vfprintf(stream,format,va);
}

int vprintft(const char* restrict format,va_list va){
    return vfprintft(stdout,format,va);
}

int fprintft(FILE* restrict stream,const char* restrict format,...){
    va_list va;
    va_start(va,format);
    int result=vfprintft(stream,format,va);
    va_end(va);
    return result;
}

int printft(const char* restrict format,...){
    va_list va;
    va_start(va,format);
    int result=vprintft(format,va);
    va_end(va);
    return result;
}
