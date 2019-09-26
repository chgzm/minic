#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

//
// debug, error
//

void _debug_print_tmsp() {
    if (!debug_flag) {
        return;
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    struct tm tm;
    localtime_r(&(ts.tv_sec), &tm);

    char buf[30];
    snprintf(
        buf
      , 30
      , "%4d/%02d/%02d %02d:%02d:%02d"
      , tm.tm_year + 1900 , tm.tm_mon + 1
      , tm.tm_mday
      , tm.tm_hour
      , tm.tm_min
      , tm.tm_sec
    );

    fprintf(stdout, "[%s ", buf);
}

void _debug(const char *fmt, ...) {
    if (!debug_flag) {
        return;
    }

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
}

void _error_print_tmsp() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    struct tm tm;
    localtime_r(&(ts.tv_sec), &tm);

    char buf[30];
    snprintf(
        buf
      , 30
      , "%4d/%02d/%02d %02d:%02d:%02d"
      , tm.tm_year + 1900 , tm.tm_mon + 1
      , tm.tm_mday
      , tm.tm_hour
      , tm.tm_min
      , tm.tm_sec
    );

    fprintf(stderr, "[%s ", buf);
}

void _error(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
}

//
// fmt
//

const char* fmt(const char* fmt, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    return strdup(buf);
}

//
// mmap
// 

void* mmap_readonly(const char* file_path) {
    const int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        error("open failed.\n");
        return NULL;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        error("fstat failed.\n");
        return NULL;
    }
    int fsize = sb.st_size;  

    void* addr = mmap(NULL, fsize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
        error("mmap failed\n");
        return NULL;
    }

    if (close(fd) == -1) {
        error("close failed.\n");
        return NULL;
    }

    return addr;
}

//
// Vector for Pointers
//

PtrVector* create_ptr_vector() {
    PtrVector* vec = malloc(sizeof(PtrVector));
    vec->elements  = malloc(sizeof(void*) * 16);
    vec->capacity  = 16;
    vec->size      = 0;

    return vec;
}

void ptr_vector_push_back(PtrVector* vec, void* e) {
    if (vec->size == vec->capacity) {
        vec->capacity *= 2;
        vec->elements = realloc(vec->elements, sizeof(void*) * vec->capacity);
    }

    vec->elements[vec->size] = e;
    ++(vec->size);
}

//
// Stack for Pointers
//

PtrStack* create_ptr_stack() {
    PtrStack* stack = malloc(sizeof(PtrStack));
    stack->elements = malloc(sizeof(void*) * 16);
    stack->capacity = 16;
    stack->top      = -1;

    return stack;
}

void ptr_stack_push(PtrStack* stack, void* e) {
    ++(stack->top);
    if (stack->top == stack->capacity) {
        stack->capacity *= 2;
        stack->elements = realloc(stack->elements, sizeof(void*) * stack->capacity);
    }

    stack->elements[stack->top] = e;
}

void* ptr_stack_top(PtrStack* stack) {
    if (stack->top < 0) {
        return NULL;
    }

    return stack->elements[stack->top];
}

void ptr_stack_pop(PtrStack* stack) {
    --(stack->top);
}
