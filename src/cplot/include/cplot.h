#ifndef __CPLOT_CPLOT_H__
#define __CPLOT_CPLOT_H__
#include <stdbool.h>

typedef long double number_t;
number_t eval_value(number_t y, number_t x, const char* expr);
bool eval_cmp(number_t y, number_t x, char* expr, number_t* z);
bool eval(number_t y, number_t x, const char* _expr, number_t* z);
void plot_png(char** argv);
void init(char** argv);
#define INIT(x) void init_##x(number_t n)
INIT(_y1);
INIT(_y2);
INIT(x1);
INIT(x2);
INIT(s1);
INIT(s2);
INIT(deltaX);
INIT(deltaY);
#undef INIT

#define SET(x, type) void set_##x(type n)
SET(brush_size, u_int32_t);
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

SET(margin, u_int32_t);

SET(padding, u_int32_t);
SET(LEFT_MARGIN, u_int32_t);
SET(RIGHT_MARGIN, u_int32_t);
SET(TOP_MARGIN, u_int32_t);
SET(END_MARGIN, u_int32_t);

SET(LEFT_PADDING, u_int32_t);
SET(RIGHT_PADDING, u_int32_t);
SET(TOP_PADDING, u_int32_t);
SET(END_PADDING, u_int32_t);
#undef SET
#endif