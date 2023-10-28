#ifndef _CPLOT_SETTERS_H_
#define _CPLOT_SETTERS_H_
#else
error "user should not include this hearder."
#endif
#ifndef SET_AXIS
#define SET_AXIS(axis) void set_##axis##_axis(bool enable, number_t len, unsigned int color, number_t interval)
#endif
#ifndef SET
#define SET(x, type) void set_##x(type n)
#endif
#ifndef INIT
#define INIT(x) void init_##x(number_t n)
#endif