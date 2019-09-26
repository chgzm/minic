#include "tokenizer.h"
#include "parser.h"
#include "generator.h"
#include "util.h"

#include <unistd.h>
#include <getopt.h>

bool debug_flag = false;

static struct option longopts[] = {
    { "debug", no_argument, NULL, 'd' },
};

static void usage() {
    printf(
        "Usage: minic [OPTION] file\n"
        "   -d, --debug    output debug-log.\n"
    );        
}

static char* parse_command_line(int argc, char** argv) {
    int opt = 0, index = 0;
    while ((opt = getopt_long(argc, argv, ":d", longopts, &index)) != -1) {
        switch (opt) {
        case 'd': { 
            debug_flag = true; 
            break; 
        }
        default:  { 
            fprintf(stderr, "Invalid option '%c'\n", opt);
            return NULL;
        }
        }
    }    

    if (argc <= optind) {
        return NULL;
    } 

    return argv[optind];
}

int main(int argc, char* argv[]) {
    const char* file_path = parse_command_line(argc, argv);
    if (file_path == NULL) {
        usage();
        return -1;
    }

    void* addr = mmap_readonly(file_path);
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
