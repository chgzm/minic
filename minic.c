#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "tokenizer.h"

static void* mmap_readonly(const char* file_path) {
    const int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "open failed.\n");
        return NULL;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        fprintf(stderr, "fstat failed.\n");
        return NULL;
    }
    int fsize = sb.st_size;  

    void* addr = mmap(NULL, fsize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
        fprintf(stderr, "mmap failed\n");
        return NULL;
    }

    if (close(fd) == -1) {
        fprintf(stderr, "close failed.\n");
        return NULL;
    }

    return addr;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: minic file\n");
        return -1;
    }

    void* addr = mmap_readonly(argv[1]);
    if (addr == NULL) {
        fprintf(stderr, "Failed to load file.\n"); 
        return -1;
    }

    Token* tokens = tokenize(addr);
    if (tokens == NULL) {
        fprintf(stderr, "Failed to parse.\n");
        return -1;
    }

    return 0;
}
