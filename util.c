#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//
// error
//

#ifdef MINIC_DEV
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
#else

typedef struct __va_list va_list;
struct __va_list {
  int   gp_offset;
  int   fp_offset;
  void* overflow_arg_area;
  void* reg_save_area;
};

void error(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
}

#endif

//
// fmt
//

char* fmt(const char* fmt, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    return strdup(buf);
}

//
// read file
//

char* read_file(const char* file_path) {
    FILE* fp = fopen(file_path, "r");
    if (fp == NULL) {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    int fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* addr = calloc(fsize, sizeof(char) + 1);
    fread(addr, sizeof(char), fsize, fp);
    fclose(fp);

    return addr;
}

/*
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
*/

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

void vector_push_back(Vector* vec, void* e) {
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

Stack* create_stack() {
    Stack* stack = malloc(sizeof(Stack));
    stack->elements = malloc(sizeof(void*) * 16);
    stack->capacity = 16;
    stack->top      = -1;

    return stack;
}

void stack_push(Stack* stack, void* e) {
    ++(stack->top);
    if (stack->top == stack->capacity) {
        stack->capacity *= 2;
        stack->elements = realloc(stack->elements, sizeof(void*) * stack->capacity);
    }

    stack->elements[stack->top] = e;
}

void* stack_top(Stack* stack) {
    if (stack->top < 0) {
        return NULL;
    }

    return stack->elements[stack->top];
}

void stack_pop(Stack* stack) {
    --(stack->top);
}

//
// Stack for Integer
//

IntStack* create_intstack() {
    IntStack* stack = malloc(sizeof(Stack));
    stack->elements = malloc(sizeof(int) * 16);
    stack->capacity = 16;
    stack->top      = -1;

    return stack;
}

void intstack_push(IntStack* stack, int e) {
    ++(stack->top);
    if (stack->top == stack->capacity) {
        stack->capacity *= 2;
        stack->elements = realloc(stack->elements, sizeof(int) * stack->capacity);
    }

    stack->elements[stack->top] = e;
}

int intstack_top(IntStack* stack) {
    if (stack->top < 0) {
        fprintf(stderr, "stack is empty.\n");
        exit(-1);
    }

    return stack->elements[stack->top];
}

void intstack_pop(IntStack* stack) {
    --(stack->top);
}

//
// Hashmap of char* => int
//

StrPtrMap* create_strptrmap(int capacity) {
    StrPtrMap* map = (StrPtrMap*)(malloc(sizeof(StrPtrMap)));
    map->size     = 0;
    map->capacity = capacity;
    map->entries  = (StrPtrMapEntry**)(malloc(sizeof(StrPtrMapEntry*) * capacity));

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

void strptrmap_put(StrPtrMap* map, const char* key, void* val) {
    const int hash  = calc_hash(key);
    const int index = hash % map->capacity;
    if (map->entries[index] == NULL) {
        // map->entries[index] = (StrPtrMapEntry*)(malloc(sizeof(StrPtrMapEntry)));
        map->entries[index] = calloc(1, sizeof(StrPtrMapEntry));
        map->entries[index]->key  = strdup(key);
        map->entries[index]->val  = val;
        map->entries[index]->next = NULL;
    }
    else {
        StrPtrMapEntry* current = map->entries[index];
        while (current->next != NULL) {
            current = current->next;
        }
        // current->next    = (StrPtrMapEntry*)(malloc(sizeof(StrPtrMapEntry)));
        current->next       = calloc(1, sizeof(StrPtrMapEntry));
        current->next->key  = strdup(key);
        current->next->val  = val;
        current->next->next = NULL;
    }

    ++(map->size);
}

bool strptrmap_contains(StrPtrMap* map, const char* key) {
    const int hash  = calc_hash(key);
    const int index = hash % map->capacity;

    if (map->entries[index] == NULL) {
        return false;
    }
    else {
        StrPtrMapEntry* current = map->entries[index];
        while (current != NULL) {
            if (strcmp(current->key, key) == 0) {
                return true;
            }
            current = current->next;
        }

        return false;
    }
}

void* strptrmap_get(StrPtrMap* map, const char* key) {
    const int hash  = calc_hash(key);
    const int index = hash % map->capacity;
    if (map->entries[index] == NULL) {
        return NULL;
    }
    else {
        StrPtrMapEntry* current = map->entries[index];
        while (current != NULL) {
            if (strcmp(current->key, key) == 0) {
                return current->val;
            }

            current = current->next;
        }

        return NULL;
    }
}

//
// Hashmap of char* => int
//

StrIntMap* create_strintmap(int capacity) {
    StrIntMap* map = (StrIntMap*)(malloc(sizeof(StrIntMap)));
    map->size      = 0;
    map->capacity  = capacity;
    map->entries   = (StrIntMapEntry**)(malloc(sizeof(StrIntMapEntry*) * capacity));

    for (int i = 0; i < map->capacity; ++i) {
        map->entries[i] = NULL;
    }

    return map;
}

void strintmap_put(StrIntMap* map, const char* key, int val) {
    const int hash  = calc_hash(key);
    const int index = hash % map->capacity;
    if (map->entries[index] == NULL) {
        map->entries[index] = (StrIntMapEntry*)(malloc(sizeof(StrIntMapEntry)));
        map->entries[index]->key  = strdup(key);
        map->entries[index]->val  = val;
        map->entries[index]->next = NULL;
    }
    else {
        StrIntMapEntry* current = map->entries[index];
        while (current->next != NULL) {
            current = current->next;
        }
        current->next       = (StrIntMapEntry*)(malloc(sizeof(StrIntMapEntry)));
        current->key        = strdup(key);
        current->val        = val;
        current->next->next = NULL;
    }

    ++(map->size);
}

bool strintmap_contains(StrIntMap* map, const char* key) {
    const int hash  = calc_hash(key);
    const int index = hash % map->capacity;

    if (map->entries[index] == NULL) {
        return false;
    }
    else {
        StrIntMapEntry* current = map->entries[index];
        while (current != NULL) {
            if (strcmp(current->key, key) == 0) {
                return true;
            }
            current = current->next;
        }
        return false;
    }
}

int strintmap_get(StrIntMap* map, const char* key) {
    const int hash  = calc_hash(key);
    const int index = hash % map->capacity;
    if (map->entries[index] == NULL) {
        fprintf(stderr, "no key=\"%s\"\n", key);
        exit(-1);
    }
    else {
        StrIntMapEntry* current = map->entries[index];
        while (current != NULL) {
            if (strcmp(current->key, key) == 0) {
                return current->val;
            }

            current = current->next;
        }

        fprintf(stderr, "no key=\"%s\"\n", key);
        exit(-1);
    }
}
