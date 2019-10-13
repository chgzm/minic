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

Vector* create_ptr_vector();
void ptr_vector_push_back(Vector* vec, void* e);

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

Stack* create_ptr_stack();
void ptr_stack_push(Stack* stack, void* e);
void* ptr_stack_top(Stack* stack);
void ptr_stack_pop(Stack* stack);

//
// Hashmap for string => pointer
//

typedef struct StrHashMapEntry StrHashMapEntry;
struct StrHashMapEntry {
    char*            key;
    void*            val;
    StrHashMapEntry* next;
};

typedef struct StrHashMap StrHashMap;
struct StrHashMap {
    StrHashMapEntry** entries;
    int               size;
    int               capacity;
};

StrHashMap* create_strhashmap(int capacity);
void strhashmap_put(StrHashMap* map, const char* key, void* val);
bool strhashmap_contains(StrHashMap* map, const char* key);
void* strhashmap_get(StrHashMap* map, const char* key);

#endif
