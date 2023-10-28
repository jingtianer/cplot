#include "logger.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
static log_level_t log_level = INFO_LOG;
void set_log_level(log_level_t level) {
    log_level = level;
}

void logger(int level, const char* format, ...) {
    if (level > log_level) return;
    va_list args;
    va_start(args, format);
    int len = vsnprintf(NULL, 0, format, args) + 1;
    va_end(args);
    va_start(args, format);
    char* str = malloc(len);
    vsprintf(str, format, args);
    va_end(args);
    if (level == ERR_LOG) fprintf(stderr, "err_log: ");
    fprintf(stderr, "%s\n", str);
    free(str);
    if (level == ERR_LOG) {
        exit(1);
    }
}

int alloc_sprintf(char** level, const char* format, ...) {
    if (level == NULL) return 0;
    va_list args;
    va_start(args, format);
    int len = vsnprintf(NULL, 0, format, args) + 1;
    va_end(args);
    va_start(args, format);
    *level = malloc(len);
    vsprintf(*level, format, args);
    va_end(args);
    return len;
}