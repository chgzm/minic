#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//
// error 
//

#define error(fmt, ...) \
    _print_tmsp(); \
    fprintf(stderr, "%s:%d %s] ", __FILE__, __LINE__, __func__); \
    _error(fmt, ## __VA_ARGS__); 

void _print_tmsp();
void _error(const char* fmt, ...);

//
// fmt
//

const char* fmt(const char* fmt, ...);

//
// mmap
//

void* mmap_readonly(const char* file_path);

//
// Vector for Pointers
//

typedef struct PtrVector PtrVector;
struct PtrVector {
    void** elements;
    int    size;
    int    capacity;
};

PtrVector* create_ptr_vector();
void ptr_vector_push_back(PtrVector* vec, void* e);

#endif
