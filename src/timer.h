#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include <stdarg.h>

void initTimer();

int vprintft(const char* restrict format,va_list va);

int printft(const char* restrict format,...);

#endif