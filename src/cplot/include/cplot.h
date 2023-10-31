#ifndef __CPLOT_CPLOT_H__
#define __CPLOT_CPLOT_H__
#define _XOPEN_SOURCE 500
#include <math.h>
#include <float.h>
#include <stdbool.h>
#include <sys/types.h>
#include "./log_levels.h"
#include "./setters.h"
void set_logger_log_level(log_level_t level);
typedef long double number_t;
number_t eval_value(number_t y, number_t x, const char* expr);
bool eval_cmp(number_t y, number_t x, char* expr, number_t* z);
bool eval(number_t y, number_t x, const char* _expr, number_t* z);
void plot_png(char** argv);

PLOT(normal);

PLOT(polar);

PLOT(parametric);

PLOT(function);

INIT_CPLOT(normal);

INIT_CPLOT(polar);

INIT_CPLOT(parametric);

INIT_CPLOT(function);

void plot_console(char** argv);
void init(char** argv);
SET(inner_char, char);
SET(outer_char, char);
INIT(_y1);
INIT(_y2);
INIT(x1);
INIT(x2);
INIT(sy);
INIT(sx);
INIT(deltaX);
INIT(deltaY);
INIT(t1);
INIT(t2);
INIT(st);
SET(max_try, number_t);
SET(brush_size, int);
SET(output_file, FILE*);
SET(R, u_int8_t);
SET(G, u_int8_t);
SET(B, u_int8_t);
SET(A, u_int8_t);
SET(BG_R, u_int8_t);
SET(BG_G, u_int8_t);
SET(BG_B, u_int8_t);
SET(BG_A, u_int8_t);
SET(brush_color, u_int32_t);
SET(bg_color, u_int32_t);

SET(margin, int);

SET(padding, int);
SET(LEFT_MARGIN, int);
SET(RIGHT_MARGIN, int);
SET(TOP_MARGIN, int);
SET(END_MARGIN, int);

SET(LEFT_PADDING, int);
SET(RIGHT_PADDING, int);
SET(TOP_PADDING, int);
SET(END_PADDING, int);
void enable_fastmode(bool enable);

SET(continuous_only, bool);

SET_AXIS(x);
SET_AXIS(y);

#undef SET
#undef SET_AXIS
#undef INIT
#endif