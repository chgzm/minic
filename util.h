#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>

#define error(fmt, ...) \
    _print_tmsp(); \
    fprintf(stderr, "%s:%d %s] ", __FILE__, __LINE__, __func__); \
    _error(fmt, ## __VA_ARGS__); 

void _print_tmsp();
void _error(char* fmt, ...);

#endif
