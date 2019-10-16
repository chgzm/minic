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

#define error(fmt, ...) \
    _error_print_tmsp(); \
    fprintf(stderr, "%s:%d %s] ", __FILE__, __LINE__, __func__); \
    _error(fmt, ## __VA_ARGS__); \
    fflush(stderr)

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

typedef struct Vector Vector;
struct Vector {
    void** elements;
    int    size;
    int    capacity;
};

Vector* create_vector();
void vector_push_back(Vector* vec, void* e);

//
// Vector for int
//

typedef struct IntVector IntVector;
struct IntVector {
    int* elements;
    int  size;
    int  capacity;
};

IntVector* create_int_vector();
void int_vector_push_back(IntVector* vec, int e);

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
