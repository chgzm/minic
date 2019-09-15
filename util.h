#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define error(fmt, ...) \
    _print_tmsp(); \
    fprintf(stderr, "%s:%d %s] ", __FILE__, __LINE__, __func__); \
    _error(fmt, ## __VA_ARGS__); 

void _print_tmsp();
void _error(const char* fmt, ...);

void* mmap_readonly(const char* file_path);

#endif
