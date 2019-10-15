#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//
// error
//

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

Vector* create_vector() {
    Vector* vec = malloc(sizeof(Vector));
    vec->elements  = malloc(sizeof(void*) * 16);
    vec->capacity  = 16;
    vec->size      = 0;

    return vec;
}

void ptr_vector_push_back(Vector* vec, void* e) {
    if (vec->size == vec->capacity) {
        vec->capacity *= 2;
        vec->elements = realloc(vec->elements, sizeof(void*) * vec->capacity);
    }

    vec->elements[vec->size] = e;
    ++(vec->size);
}

//
// Vector for int
//

IntVector* create_int_vector() {
    IntVector* vec = malloc(sizeof(IntVector));
    vec->elements  = malloc(sizeof(int) * 16);
    vec->capacity  = 16;
    vec->size      = 0;

    return vec;
}

void int_vector_push_back(IntVector* vec, int e) {
    if (vec->size == vec->capacity) {
        vec->capacity *= 2;
        vec->elements = realloc(vec->elements, sizeof(int) * vec->capacity);
    }

    vec->elements[vec->size] = e;
    ++(vec->size);
}

//
// Stack for Pointers
//

Stack* create_stack() {
    Stack* stack = malloc(sizeof(Stack));
    stack->elements = malloc(sizeof(void*) * 16);
    stack->capacity = 16;
    stack->top      = -1;

    return stack;
}

void ptr_stack_push(Stack* stack, void* e) {
    ++(stack->top);
    if (stack->top == stack->capacity) {
        stack->capacity *= 2;
        stack->elements = realloc(stack->elements, sizeof(void*) * stack->capacity);
    }

    stack->elements[stack->top] = e;
}

void* ptr_stack_top(Stack* stack) {
    if (stack->top < 0) {
        return NULL;
    }

    return stack->elements[stack->top];
}

void ptr_stack_pop(Stack* stack) {
    --(stack->top);
}

//
// Hashmap of char* => int
//

StrHashMap* create_strhashmap(int capacity) {
    StrHashMap* map = (StrHashMap*)(malloc(sizeof(StrHashMap)));
    map->size     = 0;
    map->capacity = capacity;
    map->entries  = (StrHashMapEntry**)(malloc(sizeof(StrHashMapEntry*) * capacity));

    for (int i = 0; i < map->capacity; ++i) {
        map->entries[i] = NULL;
    } 

    return map;
}

static int calc_hash(const char* str) {
    int h = 0, pos = 0;
    while (str[pos] != '\0') {
        h += str[pos];
        ++pos;
    }

    return h;
}

void strhashmap_put(StrHashMap* map, const char* key, void* val) {
    const int hash  = calc_hash(key);
    const int index = hash % map->capacity;
    if (map->entries[index] == NULL) {
        map->entries[index] = (StrHashMapEntry*)(malloc(sizeof(StrHashMapEntry)));
        map->entries[index]->key  = strdup(key);
        map->entries[index]->val  = val;
        map->entries[index]->next = NULL;
    }
    else {
        StrHashMapEntry* current = map->entries[index];
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = (StrHashMapEntry*)(malloc(sizeof(StrHashMapEntry)));
        map->entries[index]->key = strdup(key);
        map->entries[index]->val = val;
        current->next->next      = NULL;
    }

    ++(map->size);
}

bool strhashmap_contains(StrHashMap* map, const char* key) {
    const int hash  = calc_hash(key);
    const int index = hash % map->capacity;
    return (map->entries[index] != NULL);
}

void* strhashmap_get(StrHashMap* map, const char* key) {
    const int hash  = calc_hash(key);
    const int index = hash % map->capacity;
    if (map->entries[index] == NULL) {
        return NULL;
    }
    else {
        StrHashMapEntry* current = map->entries[index];
        while (current != NULL) {
            if (strcmp(current->key, key) == 0) {
                return current->val;
            }
             
            current = current->next;
        }

        return NULL;
    }

}
