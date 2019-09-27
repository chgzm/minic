#ifndef GENERATOR_H
#define GENERATOR_H

#include "parser.h"

extern bool debug_flag;

enum VarType {
    VAR_VOID,
    VAR_CHAR,
    VAR_SHORT,
    VAR_INT,
    VAR_LONG,
    VAR_FLOAT,
    VAR_DOUBLE,
    VAR_SIGNED,
    VAR_UNSIGNED,
    VAR_PTR,
};

typedef struct Type Type;
struct Type {
    int   base_type;
    int   type_size;
    int   array_size;
    Type* ptr;
};

typedef struct LocalVar LocalVar;
struct LocalVar {
    Type* type; 
    char* name;
    int   name_len;
    int   offset;
};

void gen(const TransUnitNode* node);

#endif
