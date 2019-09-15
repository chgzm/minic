#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "tokenizer.h"
#include "parser.h"
#include "generator.h"
#include "util.h"

static void* mmap_readonly(const char* file_path) {
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
