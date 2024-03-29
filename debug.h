#ifndef DEBUG_H
#define DEBUG_H

#include "tokenizer.h"
#include "parser.h"
#include "util.h"

//
// debug
//
#ifdef MINIC_DEV
#define debug(fmt, ...) \
    _debug_print_tmsp(); \
    fprintf(stdout, "%s:%d %s] ", __FILE__, __LINE__, __func__); \
    _debug(fmt, ## __VA_ARGS__); \
    fflush(stdout)

void _debug_print_tmsp();
void _debug(const char* fmt, ...);
#else
void debug(const char* fmt, ...);
#endif

//
// token
//

void dump_tokens(const Vector* vec);

//
// parser
//

void dump_nodes(const TransUnitNode* node);

#endif
