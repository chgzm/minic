#include "tokenizer.h"
#include "preprocessor.h"
#include "parser.h"
#include "generator.h"
#include "util.h"
#include "debug.h"

#include <stdio.h>
#include <string.h>

bool debug_flag = false;

static void usage() {
    printf("Usage: minic [OPTION] file\n   -d, --debug    output debug-log.\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        usage();
        return -1;
    }

    int arg_index = 1;
    if (strncmp("-d", argv[1], 2) == 0 || strncmp("--debug", argv[1], 7) == 0) {
        debug_flag = true;
        arg_index = 2;
    } 

    void* addr = mmap_readonly(argv[arg_index]);
    if (addr == NULL) {
        error("Failed to load file.\n"); 
        return -1;
    }

    const TokenVec* vec = tokenize(addr);
    if (vec == NULL) {
        error("Failed to tokenize.\n");
        return -1;
    }

    if (debug_flag) {
        dump_tokens(vec);
    }

    const TokenVec* processed_vec = preprocess(vec); 
    if (processed_vec == NULL) {
        error("Failed to preprocess.\n");
        return -1;
    }

    if (debug_flag) {
        dump_tokens(processed_vec);
    }

    const TransUnitNode* node = parse(processed_vec);
    if (node == NULL) {
        error("Failed to parse.\n");
        return -1;
    }

    if (debug_flag) {
        dump_nodes(node);
    }

    gen(node);

    return 0;
}
