#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

void _print_tmsp() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    struct tm tm;
    localtime_r(&(ts.tv_sec), &tm);

    char buf[30];
    snprintf(
        buf
      , 30
      , "%d/%02d/%02d %02d:%02d:%02d.%09ld"
      , tm.tm_year + 1900
      , tm.tm_mon + 1
      , tm.tm_mday
      , tm.tm_hour
      , tm.tm_min
      , tm.tm_sec
      , ts.tv_nsec
    );

    fprintf(stderr, "[%s ", buf);
}

void _error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
}

void* mmap_readonly(const char* file_path) {
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
