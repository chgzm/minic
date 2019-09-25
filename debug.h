#ifndef DEBUG_H
#define DEBUG_H

#include "tokenizer.h"
#include "parser.h"

//
// token
//

void dump_tokens(const TokenVec* vec);

//
// parser
//

void dump_nodes(const TransUnitNode* node);

#endif
