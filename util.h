#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//
// error
//

#ifdef MINIC_DEV
#define error(fmt, ...) \
    _error_print_tmsp(); \
    fprintf(stderr, "%s:%d %s] ", __FILE__, __LINE__, __func__); \
    _error(fmt, ## __VA_ARGS__); \
    fflush(stderr)

void _error_print_tmsp();
void _error(const char* fmt, ...);

#else
void error(const char* fmt, ...);

#define NULL  0
#define bool  int
#define false 0
#define true  1
#define O_RDONLY 0
#define FILE void

typedef struct __va_list va_list;
struct __va_list {
  int   gp_offset;
  int   fp_offset;
  void* overflow_arg_area;
  void* reg_save_area;
};

#endif

//
// fmt
//

char* fmt(const char* fmt, ...);

//
// read file
//

char* read_file(const char* file_path);
// void* mmap_readonly(const char* file_path);

//
// Vector for Pointers
//

typedef struct Vector Vector;
struct Vector {
    void** elements;
    int    size;
    int    capacity;
};

Vector* create_vector();
void vector_push_back(Vector* vec, void* e);

//
// Stack for Pointers
//

typedef struct Stack Stack;
struct Stack {
    void** elements;
    int    top;
    int    capacity;
};

Stack* create_stack();
void stack_push(Stack* stack, void* e);
void* stack_top(Stack* stack);
void stack_pop(Stack* stack);

//
// Stack for Integer
//

typedef struct IntStack IntStack;
struct IntStack {
    int* elements;
    int  top;
    int  capacity;
};

IntStack* create_intstack();
void intstack_push(IntStack* stack, int e);
int intstack_top(IntStack* stack);
void intstack_pop(IntStack* stack);

//
// Hashmap for string => pointer
//

typedef struct StrPtrMapEntry StrPtrMapEntry;
struct StrPtrMapEntry {
    char*            key;
    void*            val;
    StrPtrMapEntry* next;
};

typedef struct StrPtrMap StrPtrMap;
struct StrPtrMap {
    StrPtrMapEntry** entries;
    int               size;
    int               capacity;
};

StrPtrMap* create_strptrmap(int capacity);
void strptrmap_put(StrPtrMap* map, const char* key, void* val);
bool strptrmap_contains(StrPtrMap* map, const char* key);
void* strptrmap_get(StrPtrMap* map, const char* key);

//
// Hashmap for string => int
//

typedef struct StrIntMapEntry StrIntMapEntry;
struct StrIntMapEntry {
    char*           key;
    int             val;
    StrIntMapEntry* next;
};

typedef struct StrIntMap StrIntMap;
struct StrIntMap {
    StrIntMapEntry** entries;
    int              size;
    int              capacity;
};

StrIntMap* create_strintmap(int capacity);
void strintmap_put(StrIntMap* map, const char* key, int val);
bool strintmap_contains(StrIntMap* map, const char* key);
int strintmap_get(StrIntMap* map, const char* key);

#endif
