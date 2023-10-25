#include "timer.h"
#include <stdio.h>
#include <sys/time.h>

static struct timeval initialTime;

void initTimer(){
    gettimeofday(&initialTime,NULL);
}

int vprintft(const char* restrict format,va_list va){
    struct timeval currentTime;
    double elapsedTime;

    gettimeofday(&currentTime, NULL);
    elapsedTime = (currentTime.tv_sec - initialTime.tv_sec); 
    elapsedTime += (currentTime.tv_usec - initialTime.tv_usec) / 1000000.0;   // us to s
    
    printf("[%lf] ",elapsedTime);
    return vprintf(format,va);
}

int printft(const char* restrict format,...){
    va_list va;
    va_start(va,format);
    int result=vprintft(format,va);
    va_end(va);
    return result;
}
