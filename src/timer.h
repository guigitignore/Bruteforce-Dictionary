#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include <stdarg.h>
#include <stdio.h>

void initTimer();

int vfprintft(FILE* restrict stream,const char* restrict format,va_list va);

int vprintft(const char* restrict format,va_list va);

int fprintft(FILE* restrict stream,const char* restrict format,...);

int printft(const char* restrict format,...);

#endif