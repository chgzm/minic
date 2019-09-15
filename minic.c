#include "tokenizer.h"
#include "parser.h"
#include "generator.h"
#include "util.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        error("Usage: minic file\n");
        return -1;
    }

    void* addr = mmap_readonly(argv[1]);
    if (addr == NULL) {
        error("Failed to load file.\n"); 
        return -1;
    }

    TokenVec* vec = tokenize(addr);
    if (vec == NULL) {
        error("Failed to tokenize.\n");
        return -1;
    }

    TransUnitNode* node = parse(vec);
    if (node == NULL) {
        error("Failed to parse.\n");
        return -1;
    }

    gen(node);

    return 0;
}
