#ifndef __CPLOT_LOGGER_H__
#define __CPLOT_LOGGER_H__
#include "../cplot/include/log_levels.h"
void logger(int level, const char* format, ...);
int alloc_sprintf(char** level, const char* format, ...);
void set_log_level(log_level_t level);
// #define DEBUG_LOG 7
// #define INFO_LOG 5
// #define ERR_LOG 4
#endif