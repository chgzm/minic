#ifndef GENERATOR_H
#define GENERATOR_H

#include "parser.h"
#include "util.h"

enum VarType {
    VAR_VOID,
    VAR_CHAR,
    VAR_INT,
    VAR_DOUBLE,
    VAR_PTR,
    VAR_STRUCT,
};

typedef struct FieldInfo FieldInfo;
typedef struct StructInfo StructInfo;
typedef struct Type Type;
typedef struct LocalVar LocalVar;
typedef struct GlobalVar GlobalVar;

struct FieldInfo {
    Type* type;
    int   offset;
};

struct StructInfo {
    StrPtrMap* field_info_map; // field-name => field-info
    int        size;
};

struct Type {
    int         base_type;
    int         type_size;
    int         size;
    int         array_size;
    int         ptr_count;
    StructInfo* struct_info;
};

struct LocalVar {
    Type* type; 
    char* name;
    int   name_len;
    int   offset;
};

struct GlobalVar {
    Type* type;
    char* name;
    int   name_len;
};

void gen(const TransUnitNode* node);

#endif
