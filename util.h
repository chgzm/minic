#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//
// debug, error 
//

extern bool debug_flag;

#define debug(fmt, ...) \
    _debug_print_tmsp(); \
    if (debug_flag) fprintf(stdout, "%s:%d %s] ", __FILE__, __LINE__, __func__); \
    _debug(fmt, ## __VA_ARGS__); 

#define error(fmt, ...) \
    _error_print_tmsp(); \
    fprintf(stderr, "%s:%d %s] ", __FILE__, __LINE__, __func__); \
    _error(fmt, ## __VA_ARGS__); 

void _debug_print_tmsp();
void _debug(const char* fmt, ...);
void _error_print_tmsp();
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

//
// Stack for Pointers
//

typedef struct PtrStack PtrStack;
struct PtrStack {
    void** elements;
    int    top;
    int    capacity;
};

PtrStack* create_ptr_stack();
void ptr_stack_push(PtrStack* stack, void* e);
void* ptr_stack_top(PtrStack* stack);
void ptr_stack_pop(PtrStack* stack);


#endif
