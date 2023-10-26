#ifndef __CPLOT_LOGGER_H__
#define __CPLOT_LOGGER_H__
void logger(int level, const char* format, ...);
int alloc_sprintf(char** level, const char* format, ...);

#define DEBUG_LOG 7
#define INFO_LOG 5
#define ERR_LOG 4
#endif