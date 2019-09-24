#ifndef GENERATOR_H
#define GENERATOR_H

#include "parser.h"

typedef struct LocalVar LocalVar;
struct LocalVar {
    char* name;
    int   name_len;
    int   offset;
};

void gen(const TransUnitNode* node);

#endif
