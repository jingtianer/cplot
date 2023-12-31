#define _XOPEN_SOURCE 500
#define _POSIX_C_SOURCE 200809L
#define _C99_SOURCE
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <float.h>

#include "../../svpng/svpng.inc"
#include "../logger/logger.h"
#include "include/cplot.h"
#undef _CPLOT_SETTERS_H_
#include "include/setters.h"

bool plot_y_axis = false, plot_x_axis = false;
number_t y_scale_len = 0, x_scale_len = 0;
unsigned int y_scale_color = 0, x_scale_color = 0;
number_t y_interval = 1, x_interval = 1;
#define SET_AXIS_IMPL(axis) SET_AXIS(axis) { \
    plot_##axis##_axis = enable; \
    axis##_scale_len = len; \
    axis##_scale_color = color; \
    axis##_interval = interval; \
    logger(DEBUG_LOG, "set_" #axis "_axis: enable=%s, len=%Lf, interval = %Lf", (enable ? "true" : "false"), len, interval); \
}
SET_AXIS_IMPL(x);
SET_AXIS_IMPL(y);

static number_t x1, x2, _y1, _y2, sy, sx;
static number_t deltaX, deltaY;
static number_t max_try = 100;


static int brush_size = 0;
static unsigned int bg_color = 0xFFFFFFFF;
static unsigned int brush_color = 0x000000FF;
static bool fast_mode = false;
static FILE* output_file = NULL;
void enable_fastmode(bool enable) {
    fast_mode = enable;
    logger(DEBUG_LOG, "enable fast mode = %s", (enable ? "true" : "false"));
}

#define SETImpl(x, type, formatter) SET(x, type) { \
    x = n; \
    logger(DEBUG_LOG, "set " #x " = " formatter, x); \
}

SETImpl(max_try, number_t, "%Lf");

#define SET_R(color, val) ((color) |= (val) << (3 << 3))
#define SET_G(color, val) ((color) |= (val) << (2 << 3))
#define SET_B(color, val) ((color) |= (val) << (1 << 3))
#define SET_A(color, val) ((color) |= (val) << (0 << 3))
#define GET_R(color) ((color >> (3 << 3)) & 0xff)
#define GET_G(color) ((color >> (2 << 3)) & 0xff)
#define GET_B(color) ((color >> (1 << 3)) & 0xff)
#define GET_A(color) ((color >> (0 << 3)) & 0xff)
#define SETCOLORImpl(x, writeto, color) SET(x, unsigned char) { SET_##color(writeto, n); logger(DEBUG_LOG, "set " #x " = %x", n); }
SETImpl(brush_size, int, "%d");
SETImpl(output_file, FILE*, "%p");
SETCOLORImpl(R, brush_color, R)
SETCOLORImpl(G, brush_color, G)
SETCOLORImpl(B, brush_color, B)
SETCOLORImpl(A, brush_color, A)
SETCOLORImpl(BG_R, bg_color, R)
SETCOLORImpl(BG_G, bg_color, G)
SETCOLORImpl(BG_B, bg_color, B)
SETCOLORImpl(BG_A, bg_color, A)
SETImpl(brush_color, u_int32_t, "%u")
SETImpl(bg_color, u_int32_t, "%u")
int LEFT_MARGIN, RIGHT_MARGIN, LEFT_PADDING, RIGHT_PADDING,
TOP_MARGIN, END_MARGIN, TOP_PADDING, END_PADDING;
int padding, margin;

SET(margin, int) {
    margin = LEFT_MARGIN = RIGHT_MARGIN = TOP_MARGIN = END_MARGIN = n;
    logger(DEBUG_LOG, "set margin = %u", margin);
}

SET(padding, int) {
    padding = LEFT_PADDING = RIGHT_PADDING = TOP_PADDING = END_PADDING = n;
    logger(DEBUG_LOG, "set padding = %u", padding);
}
SETImpl(LEFT_MARGIN, int, "%d");
SETImpl(RIGHT_MARGIN, int, "%d");
SETImpl(TOP_MARGIN, int, "%d");
SETImpl(END_MARGIN, int, "%d");

SETImpl(LEFT_PADDING, int, "%d");
SETImpl(RIGHT_PADDING, int, "%d");
SETImpl(TOP_PADDING, int, "%d");
SETImpl(END_PADDING, int, "%d");

void set_logger_log_level(log_level_t level) {
    set_log_level(level);
}

#define PUSH(s, n) (s[s##_ptr++] = (n))
#define POP(s, n) (n = s[--s##_ptr])
#define TOP(s, n) (n = s[s##_ptr-1])
#define EMPTY(s) (s##_ptr == 0)
typedef enum unary_ops {
    op_acos = 48,
    op_asin,
    op_atan,
    op_cos,
    op_cosh,
    op_sin,
    op_sinh,
    op_tan,
    op_tanh,
    op_exp,
    op_log,
    op_floor,
    op_round,
    op_sqrt,
    op_fabs,
    op_ceil
} unary_ops;
const static unary_ops op_min = op_acos;
const static unary_ops op_max = op_ceil;

static number_t stack[1024];
static char op_stack[1024];
static int priv[128] = {
        [0 ... 36] = 0,
        ['%'] = 3,
        [38 ... 39] = 0,
        ['('] = 1,
        [')'] = 2,
        ['*'] = 3,
        ['+'] = 2,
        [44] = 0,
        ['-'] = 2,
        [46] = 0,
        ['/'] = 3,
        [48 ... 93] = 0,
        ['^'] = 4,
        [95 ... 127] = 0
};
static int stack_ptr = 0;
static int op_stack_ptr = 0;

void static logStack() {
    logger(DEBUG_LOG, "op_stack: ");
    for (int cnt = 0; cnt < op_stack_ptr; cnt++) {
        logger(DEBUG_LOG, "%c ", op_stack[cnt]);
    }
    logger(DEBUG_LOG, "stack: ");
    for (int cnt = 0; cnt < stack_ptr; cnt++) {
        logger(DEBUG_LOG, "%Le ", stack[cnt]);
    }
    logger(DEBUG_LOG, "");
}

void static biCheck() {
    if (EMPTY(stack)) {
        logger(ERR_LOG, "empty stack!");
        exit(1);
    }
}

void static pushOP(char cur_op) {
    number_t n1, n2;
    char op;
    while (!EMPTY(op_stack) && priv[TOP(op_stack, op)] >= priv[cur_op]) {
        POP(op_stack, op);
        number_t res = 0;
        POP(stack, n2);
        if (!EMPTY(stack))POP(stack, n1);
        else n1 = 0;
        switch (op) {
        case '+':
            res = n1 + n2;
            break;
        case '-':
            res = n1 - n2;
            break;
        case '*':
            res = n1 * n2;
            break;
        case '/':
            if (n2 == 0) {
                logger(INFO_LOG, "divisor is zeor!");
                //                    exit(1);
            }
            res = n1 / n2;
            break;
        case '%':
            if (n2 == 0) {
                logger(INFO_LOG, "divisor is zeor!");
                //                    exit(1);
            }
            res = fmodl(n1, n2);
            break;
        case '^':
            res = powl(n1, n2);
            break;
        default:
            break;
        }
        PUSH(stack, res);
    }
}

int static len_strncmp(const char* a, const char* b) {
    return strncmp(a, b, strlen(b));
}

number_t eval_value(number_t y, number_t x, const char* expr) {
    logger(DEBUG_LOG, "eval_value, expr = %s\n", expr);

    size_t len = strlen(expr);
    size_t i = 0;
    stack_ptr = 0;
    op_stack_ptr = 0;
    while (i < len) {
        switch (expr[i]) {
        case '^':
            biCheck();
            pushOP(expr[i]);
            // if(EMPTY(stack)) PUSH(stack, 0);
            PUSH(op_stack, expr[i]);
            break;
        case 'x':
        case 'X':
        case 't':
            PUSH(stack, x);
            break;
        case 'y':
        case 'Y':
        case 'r':
            PUSH(stack, y);
            break;
        case '(':
            PUSH(op_stack, '(');
            break;
        case ')': {
            number_t n1, n2;
            char op;
            if (EMPTY(op_stack)) {
                logger(ERR_LOG, "no match ')'");
                exit(1);
            }
            pushOP(')');
            POP(op_stack, op);
            if (TOP(op_stack, op) >= op_min && TOP(op_stack, op) <= op_max) {
                POP(op_stack, op);
                number_t n;
                if (!EMPTY(stack)) {
                    POP(stack, n);
                } else {
                    logger(ERR_LOG, "Error : no opvalue");
                    exit(1);
                }
                number_t(*op_func)(number_t);
                switch (op) {
                case op_acos:
                    if (n > 1 || n < -1) return 0.0 / 0.0;
                    op_func = acosl;
                    break;
                case op_asin:
                    if (n > 1 || n < -1) return 0.0 / 0.0;
                    op_func = asinl;
                    break;
                case op_atan:
                    op_func = atanl;
                    break;
                case op_cos:
                    op_func = cosl;
                    break;
                case op_cosh:
                    op_func = coshl;
                    break;
                case op_sin:
                    op_func = sinl;
                    break;
                case op_sinh:
                    op_func = sinhl;
                    break;
                case op_tan:
                    op_func = tanl;
                    break;
                case op_tanh:
                    op_func = tanhl;
                    break;
                case op_exp:
                    op_func = expl;
                    break;
                case op_log:
                    if (n < 0) return 0.0 / 0.0;
                    op_func = logl;
                    break;
                case op_sqrt:
                    if (n < 0) return 0.0 / 0.0;
                    op_func = sqrtl;
                    break;
                case op_fabs:
                    op_func = fabsl;
                    break;
                case op_ceil:
                    op_func = ceill;
                    break;
                case op_floor:
                    op_func = floorl;
                    break;
                case op_round:
                    op_func = roundl;
                    break;
                }
                PUSH(stack, op_func(n));
            }
        }
                break;
        case '+':
        case '-': {
            if ((i > 0 && expr[i - 1] != '(')) { // fix： a-(-b)
                // if stack is empty or last op is '(', ‘-’ is an Unary operator
                // else it's a Binary operator
                biCheck();
                pushOP(expr[i]);
            } else {
                PUSH(stack, 0);
            }
            PUSH(op_stack, expr[i]);
        }
                break;
        case '*':
        case '/':
        case '%':
            biCheck();
            pushOP(expr[i]);
            // if(EMPTY(stack)) PUSH(stack, 0);
            PUSH(op_stack, expr[i]);
            break;
        case 'p': // p1 = 3.14
            if (i + 1 < len && expr[i + 1] == 'i') {
                PUSH(stack, M_PI);
                i++;
            } else {
                logger(ERR_LOG, "Error 'pi': unknown char(%c)", expr[i]);
                exit(1);
            }
            break;
        case 'e': // e = 2.7
            PUSH(stack, M_E);
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '.': {

            logger(DEBUG_LOG, "before: expr+i = %s", expr + i);
            char* end = NULL;
            errno = 0;
            number_t number = strtold(expr + i, &end);
            if (errno != 0) {
                logger(ERR_LOG, "error occured while parsing number: %s, expr+i = %s, n = %lE", strerror(errno),
                    expr + i, number);
                errno = 0;
            }
            if (end == expr + i) {
                logger(ERR_LOG, "incredible, no number converted!, *end=%c, expr[i]=%c", *end, expr[i]);
            }
            i = end - expr;
            logger(DEBUG_LOG, "after: expr+i = %s, n = %lE", expr + i, number);
            PUSH(stack, number);
            i--;
        }
                break;
        default:
            if (!len_strncmp(expr + i, "ACOS")) {
                PUSH(op_stack, op_acos);
                i += 3;
            } else if (!len_strncmp(expr + i, "ASIN")) {
                PUSH(op_stack, op_asin);
                i += 3;
            } else if (!len_strncmp(expr + i, "ATAN")) {
                PUSH(op_stack, op_atan);
                i += 3;
            } else if (!len_strncmp(expr + i, "COS")) {
                if (expr[i + 3] == 'H') {
                    PUSH(op_stack, op_cosh);
                    i += 3;
                } else {
                    PUSH(op_stack, op_cos);
                    i += 2;
                }
            } else if (!len_strncmp(expr + i, "SIN")) {
                if (expr[i + 3] == 'H') {
                    PUSH(op_stack, op_sinh);
                    i += 3;
                } else {
                    PUSH(op_stack, op_sin);
                    i += 2;
                }
            } else if (!len_strncmp(expr + i, "TAN")) {
                if (expr[i + 3] == 'H') {
                    PUSH(op_stack, op_tanh);
                    i += 3;
                } else {
                    PUSH(op_stack, op_tan);
                    i += 2;
                }
            } else if (!len_strncmp(expr + i, "EXP")) {
                PUSH(op_stack, op_exp);
                i += 2;
            } else if (!len_strncmp(expr + i, "LOG")) {
                PUSH(op_stack, op_log);
                i += 2;
            } else if (!len_strncmp(expr + i, "SQRT")) {
                PUSH(op_stack, op_sqrt);
                i += 3;
            } else if (!len_strncmp(expr + i, "FABS")) {
                PUSH(op_stack, op_fabs);
                i += 3;
            } else if (!len_strncmp(expr + i, "CEIL")) {
                PUSH(op_stack, op_ceil);
                i += 3;
            } else if (!len_strncmp(expr + i, "FLOOR")) {
                PUSH(op_stack, op_floor);
                i += 4;
            } else if (!len_strncmp(expr + i, "ROUND")) {
                PUSH(op_stack, op_round);
                i += 4;
            } else {
                logger(ERR_LOG, "Error: unknown char(%c)", expr[i]);
                exit(1);
            }
            break;
        }
        i++;
        logStack();
    }
    biCheck();
    pushOP(0);
    logStack();
    return stack[0];
}

number_t accu;

bool eval_cmp(number_t y, number_t x, char* expr, number_t* z) {
    int state = 0; // 1 eq, 2 gt , 0 st
    size_t len = strlen(expr);
    size_t i = 0;
    while (i < len && !(expr[i] >= '<' && expr[i] <= '>')) {
        i++;
    }
    while (i < len && (expr[i] >= '<' && expr[i] <= '>')) {
        state |= 1 << (expr[i] - '<');
        expr[i] = 0;
        i++;
    }
    if (state == 0) {
        logger(ERR_LOG, "invalid expression, comparision operators not found!");
    }
    if (i == len) {
        logger(ERR_LOG, "invalid expression, comparision operators are binary operator, second operand not found!");
    }
    if (state >= 7) {
        logger(ERR_LOG, "invalid expression, invalid comparision operators state: %d", state);
    }
    logger(DEBUG_LOG, "compare state: %04x", state);
    number_t n1 = eval_value(y, x, expr), n2 = eval_value(y, x, expr + i);
    if (z) *z = n1 - n2;
    switch (state) {
    case 1: // <
        return n1 < n2 && !(fabsl(n1 - n2) < accu);
        break;
    case 2: // =
        return fabsl(n1 - n2) < accu;
        break;
    case 4: // >
        return n1 > n2 > 0 && !(fabsl(n1 - n2) < accu);
        break;
    case 3: // <=
        return n1 < n2 || fabsl(n1 - n2) < accu;
        break;
    case 5: // !=
        return fabsl(n1 - n2) > accu;
        break;
    case 6: // >=
        return n1 > n2 > 0 || fabsl(n1 - n2) < accu;
        break;
    default:
        logger(ERR_LOG, "unreachable");
        break;
    }
    logger(ERR_LOG, "unreachable1");
    return false;
}

bool eval(number_t y, number_t x, const char* _expr, number_t* z) {
    char* expr = strdup(_expr);
    bool ret = true;
    char* start = expr;
    char* end = NULL;
    //    if(z)*z = LDBL_MIN;
    //    number_t zi = LDBL_MIN;
    while ((end = strchr(start, ',')) != NULL) {
        *end = 0;
        if (!eval_cmp(y, x, start, z) || (z && fpclassify(*z) == FP_NAN)) {
            ret = false;
            break;
        }
        start = end + 1;
    }
    if (ret) {
        if (!eval_cmp(y, x, start, z) || (z && fpclassify(*z) == FP_NAN)) {
            ret = false;
        }
    }
    free(expr);
    return ret;
}

void init(char** argv) {
    _y1 = eval_value(0, 0, *argv++);
    _y2 = eval_value(0, 0, *argv++);
    sy = eval_value(0, 0, *argv++);
    x1 = eval_value(0, 0, *argv++);
    x2 = eval_value(0, 0, *argv++);
    sx = eval_value(0, 0, *argv++);
    deltaX = x2 - x1;
    deltaY = _y2 - _y1;
    logger(DEBUG_LOG, "init: %Le, %Le, %Le, %Le, %Le, %Le\n", _y1, _y2, sy, x1, x2, sx);
}
#define INIT(x) void init_##x(number_t n)   
#define INIT_IMPL(x) INIT(x) {x = n; logger(DEBUG_LOG, #x " = %lE", n);}
INIT_IMPL(_y1)
INIT_IMPL(_y2)
INIT_IMPL(sy)
INIT_IMPL(x1)
INIT_IMPL(x2)
INIT_IMPL(sx)
INIT_IMPL(deltaX)
INIT_IMPL(deltaY)
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

    void static draw(unsigned char* rgba, int i, int j, int w, int h, int radius, unsigned int color) {
    for (int y = max(-radius, -i); y <= min(radius, h - i - 1); y++) {
        for (int x = max(-radius, -j); x <= min(radius, w - j - 1); x++) {
            if (sqrt(x * x + y * y) > radius) continue;
            unsigned char* p = rgba + 4 * w * (i + y) + 4 * (j + x);
            *p++ = GET_R(color);
            *p++ = GET_G(color);
            *p++ = GET_B(color);
            *p++ = GET_A(color);
        }
    }
}

bool static drawable(unsigned char* rgba, int i, int j, int w, int h, int radius, unsigned int color) {
    for (int y = max(-radius, -i); y <= min(radius, h - i - 1); y++) {
        for (int x = max(-radius, -j); x <= min(radius, w - j - 1); x++) {
            if (sqrt(x * x + y * y) > radius) continue;
            unsigned char* p = rgba + 4 * w * (i + y) + 4 * (j + x);
            if (
                *p++ == GET_R(color) &&
                *p++ == GET_G(color) &&
                *p++ == GET_B(color) &&
                *p++ == GET_A(color)) {
                return false;
            }
        }
    }
    return true;
}
bool continuous_only = false;
SETImpl(continuous_only, bool, "%d")
bool get_slop(number_t i, number_t j, number_t dy, number_t dx, const char* expr, number_t z0, number_t* dzy, number_t* dzx, number_t* max_dz) {
    if (!dzx || !dzy || !max_dz) {
        logger(ERR_LOG, "get_slop, null");
    }
    *max_dz = 0;
    int off[] = { 1, -1, -1, 1, 1 };
    number_t zx, zy;
    *dzx = *dzy = 0;
    bool ok = false;
    for (int offi = 0; offi < (continuous_only ? 1 : 1); offi++) {
        eval(-dy * (i - TOP_PADDING) + _y2,
            dx * (j - LEFT_PADDING - off[offi]) + x1, expr, &zx);

        number_t dzxi = off[offi] * fabsl(zx - z0) / (dx);
        *max_dz = max(*max_dz, fabsl(z0 - zx));
        if ((z0 > 0 && zx < 0) || (z0 < 0 && zx> 0)) {
            ok = (fpclassify(z0) != FP_NAN && fpclassify(zx) != FP_NAN);
        }
        if ((z0 > 0 && z0 > zx) || (z0 < 0 && z0 < zx)) {
            *dzx = fabsl(*dzx) > fabsl(dzxi) ? *dzx : dzxi;
        }
    }
    for (int offi = 0; offi < (continuous_only ? 1 : 1); offi++) {
        eval(-dy * (i - TOP_PADDING - off[offi]) + _y2,
            dx * (j - LEFT_PADDING) + x1, expr, &zy);
        number_t dzyi = off[offi] * fabsl(zy - z0) / (dy);
        *max_dz = max(*max_dz, fabsl(zy - z0));
        if ((z0 > 0 && zy < 0) || (z0 < 0 && zy > 0)) {
            ok = (fpclassify(z0) != FP_NAN && fpclassify(zy) != FP_NAN);
        }
        if ((z0 > 0 && z0 > zy) || (z0 < 0 && z0 < zy)) {
            *dzy = fabsl(*dzy) > fabsl(dzyi) ? *dzy : dzyi;
        }
    }
    return ok;
}

void plot_buffer(char** argv, unsigned char* rgba, int h, int w, number_t dx, number_t dy, unsigned int color, bool is_polar) {
    unsigned char* p = rgba;
    accu = max(dx, dy);
    int end_padding = is_polar ? 0 : END_PADDING;
    int top_padding = is_polar ? 0 : TOP_PADDING;
    int left_padding = is_polar ? 0 : LEFT_PADDING;
    int right_padding = is_polar ? 0 : RIGHT_PADDING;

    number_t ymin = _y1 - end_padding * dy, ymax = _y2 + top_padding * dy;
    number_t xmin = x1 - left_padding * dx, xmax = x2 + right_padding * dx;
    for (int i = 0; i < h + top_padding + end_padding; i++) {
        for (int j = 0; j < w + left_padding + right_padding; j++) {
            number_t y = -dy * (i - top_padding) + _y2, x = dx * (j - left_padding) + x1;
            logger(DEBUG_LOG, "y = %Lf, x = %Lf", y, x);
            bool ok = false;
            number_t z0, max_dz = LDBL_MAX;
            number_t dzx = 0, dzy = 0;
            for (char** expr = argv; *expr; expr++) {
                if (!fast_mode) {
                    eval(y, x, *expr, &z0);
                    if (fpclassify(z0) == FP_NAN) continue;
                    ok = get_slop(i, j, dy, dx, *expr, z0, &dzy, &dzx, &max_dz) && continuous_only;
                    if (ok) goto draw;
                    if (fpclassify(max_dz) == FP_NAN) continue;
                    accu = min(max(dx, dy), max_dz);
                }
                ok = eval(y, x, *expr, &z0);
                if (ok) goto draw;
                if (!fast_mode && !continuous_only) {
                    logger(DEBUG_LOG, "dzx = %Le, dzy = %Le", dzx, dzy);
                    number_t maxd = max(fabsl(dzx), fabsl(dzy));
                    if (maxd <= 0) continue;
                    dzx = dzx > 0 ? -maxd : maxd;
                    dzy = dzy > 0 ? -maxd : maxd;
                    int max_try_time = min(max_try, maxd);
                    for (int divx = max_try_time - 1; divx > 0; divx--) {
                        ok = eval(
                            -dy * (i - top_padding + divx / dzy) + _y2,
                            dx * (j - left_padding + divx / dzx) + x1, *expr, NULL);
                        if (ok) goto draw;
                    }
                }
            }
        draw:

            if (ok) {
                int draw_i, draw_j;
                if (is_polar) {
                    number_t polar_y = y * sinl(x);
                    number_t polar_x = y * cosl(x);
                    draw_i = (_y2 - polar_y) / (_y2 + _y2) * h;
                    draw_j = (polar_x + _y2) / (_y2 + _y2) * w;
                    logger(DEBUG_LOG, "r = %Lf, t = %Lf, y = %Lf, x = %Lf, i = %d, j = %d", y, x, polar_y, polar_x, draw_i, draw_j);
                } else {
                    draw_i = i;
                    draw_j = j;
                }
                draw(
                    rgba,
                    draw_i + TOP_MARGIN + TOP_PADDING - top_padding, draw_j + LEFT_MARGIN + LEFT_PADDING - left_padding,
                    w + LEFT_MARGIN + RIGHT_MARGIN + LEFT_PADDING + RIGHT_PADDING,
                    h + TOP_MARGIN + END_MARGIN + TOP_PADDING + END_PADDING,
                    brush_size,
                    color
                );
            }
        }
        //        z_cache_ptr += expr_cnt;
    }
    //    z_cache_ptr += (w + LEFT_PADDING + RIGHT_PADDING + 2) * expr_cnt;
    //    free(z_cache);
}


char inner_char = '+', outer_char = ' ';

SETImpl(inner_char, char, "%c")
SETImpl(outer_char, char, "%c")
void plot_console(char** argv) {
    if (output_file == NULL) {
        output_file = stdout;
    }
    accu = max(sy, sx);
    for (number_t i = _y2; i >= _y1; ) {
        for (number_t j = x1; j <= x2; ) {
            logger(DEBUG_LOG, "x = %Le, y = %Le", j, i);
            bool ok = false;
            for (char** expr = argv; *expr; expr++) {
                ok = eval(i, j, *expr, NULL);
                if (ok) {
                    break;
                }
            }
            if (ok) {
                fprintf(output_file, "%c", inner_char);
            } else {
                fprintf(output_file, "%c", outer_char);
            }
            j += sx;
        }
        printf("\n");
        i -= sy;
    }
}

number_t t1, t2, st;
INIT_IMPL(t1)
INIT_IMPL(t2)
INIT_IMPL(st)

void plot_parametric_buffer(char** argv, unsigned char* rgba, int h, int w, number_t dx, number_t dy, unsigned int color) {
    int expr_cnt = 0;
    number_t dt = (t2 - t1) / st;
    number_t ymax = _y2 + TOP_PADDING * dy;
    number_t ymin = _y1 - END_PADDING * dy;
    number_t xmax = x2 + RIGHT_PADDING * dx;
    number_t xmin = x1 - LEFT_PADDING * dx;
    number_t ymid = (ymax + ymin) / 2, xmid = (xmax + xmin) / 2;
    for (char** expr = argv; *expr; expr++) {
        char* common_pos = strchr(*expr, ',');
        if (common_pos == NULL) {
            logger(ERR_LOG, "format of parametric: \"y(t),x(t)\", invalid expr: %s", *expr);
        }
        *common_pos = '\0';
        char* expry = *expr, * exprx = common_pos + 1;
        number_t lasty;
        number_t lastx;
        number_t y = eval_value(0, t1, expry);
        number_t x = eval_value(0, t1, exprx);
        for (int i = 0; i <= st; i++) {
            number_t t = i * dt + t1;
            lasty = y;
            lastx = x;
            y = eval_value(0, t, expry);
            x = eval_value(0, t, exprx);
            if (fpclassify(lasty) == FP_NAN) {
                logger(DEBUG_LOG, "lasty is Nan, y = %Lf", y);
                lasty = eval_value(0, t1, expry);
            }
            if (fpclassify(lastx) == FP_NAN) {
                logger(DEBUG_LOG, "lastx is Nan, x = %Lf", x);
                lastx = eval_value(0, t1, exprx);
            }
            number_t dydt = (y - lasty) / dy, dxdt = (lastx - x) / dx;
            logger(DEBUG_LOG, "dydt=%Lf, dxdt=%Lf, y=%Lf, x=%Lf, t=%Lf", dydt, dxdt, y, x, t);


            // if (dydt == 0 || dxdt == 0) {
            //     continue;
            // }
            // if (lasty > ymax || lasty < ymin) continue;
            // if (lastx > xmax || lastx < xmin) continue;
            number_t maxd = max(fabsl(dydt), fabsl(dxdt));
            // dydt = dydt > 0 ? maxd : -maxd;
            // dxdt = dxdt > 0 ? maxd : -maxd;
            for (int k = 0; k <= (maxd); k++) {
                int i = ((ymid - y) / dy + k / maxd * dydt) + (h + TOP_PADDING + END_PADDING) / 2.0;
                int j = ((x - xmid) / dx + k / maxd * dxdt) + (w + LEFT_PADDING + RIGHT_PADDING) / 2.0;
                logger(DEBUG_LOG, "i=%d, j=%d, t=%Lf", i, j, t);

                if (j > w + LEFT_PADDING + RIGHT_PADDING || j < 0) break;
                if (i > h + TOP_PADDING + TOP_PADDING || i < 0) break;
                draw(
                    rgba,
                    i + TOP_MARGIN, j + LEFT_MARGIN,
                    w + LEFT_MARGIN + RIGHT_MARGIN + LEFT_PADDING + RIGHT_PADDING,
                    h + TOP_MARGIN + END_MARGIN + TOP_PADDING + END_PADDING,
                    brush_size,
                    color
                );
            }
            for (int k = maxd; k >= 0; k--) {
                int i = ((ymid - y) / dy + k / maxd * dydt) + (h + TOP_PADDING + END_PADDING) / 2.0;
                int j = ((x - xmid) / dx + k / maxd * dxdt) + (w + LEFT_PADDING + RIGHT_PADDING) / 2.0;
                logger(DEBUG_LOG, "i=%d, j=%d, t=%Lf", i, j, t);

                if (j > w + LEFT_PADDING + RIGHT_PADDING || j < 0) break;
                if (i > h + TOP_PADDING + TOP_PADDING || i < 0) break;
                draw(
                    rgba,
                    i + TOP_MARGIN, j + LEFT_MARGIN,
                    w + LEFT_MARGIN + RIGHT_MARGIN + LEFT_PADDING + RIGHT_PADDING,
                    h + TOP_MARGIN + END_MARGIN + TOP_PADDING + END_PADDING,
                    brush_size,
                    color
                );
            }
        }
    }
}


void plot_polar_buffer(char** argv, unsigned char* rgba, int h, int w, number_t dx, number_t dy, unsigned int color) {
    int expr_cnt = 0;
    for (char** expr = argv; *expr; expr++) expr_cnt++;
    char** exprs = malloc(sizeof(char*) * (expr_cnt * 2 + 1));
    for (int i = 0; i < expr_cnt * 2; i++) {
        alloc_sprintf(&exprs[i], "(%s)*SIN(t),(%s)*COS(t)", argv[i / 2], argv[i / 2]);
    }
    exprs[expr_cnt * 2] = NULL;
    plot_parametric_buffer(exprs, rgba, h, w, dx, dy, color);
    for (int i = 0; i < expr_cnt * 2; i++) {
        free(exprs[i]);
    }
    free(exprs);
}

void plot_function_buffer(char** argv, unsigned char* rgba, int h, int w, number_t dx, number_t dy, unsigned int color) {
    number_t ymax = _y2 + END_PADDING * dy;
    number_t ymin = _y1 - TOP_PADDING * dy;
    for (char** expr = argv; *expr; expr++) {
        number_t last_y = eval_value(0, dx * (0 - LEFT_PADDING) + x1, *argv);
        for (int j = 1; j < w + LEFT_PADDING + RIGHT_PADDING; j++) {
            number_t y = eval_value(0, dx * (j - LEFT_PADDING) + x1, *expr);
            int i_start = (ymax - y) / dy;
            int i_end = (ymax - last_y) / dy;
            last_y = y;
            if (y < ymin) continue;
            if (i_start > i_end) {
                number_t swapi = i_start;
                i_start = i_end;
                i_end = swapi;
            }
            for (int i = i_end; i >= i_start; i--) {
                if (i < 0) break;
                draw(
                    rgba,
                    i + TOP_MARGIN, j + LEFT_MARGIN,
                    w + LEFT_MARGIN + RIGHT_MARGIN + LEFT_PADDING + RIGHT_PADDING,
                    h + TOP_MARGIN + END_MARGIN + TOP_PADDING + END_PADDING,
                    brush_size,
                    color
                );
            }
        }
    }
}

typedef enum expr_type_t {
    normal,
    function,
    parametric,
    polar
} expr_type_t;

void plot_png_by_type(char** argv, expr_type_t type) {
    bool is_polar = false;
    for (char** expr = argv; *expr; expr++) {
        if (strchr(*expr, 't') != NULL || strchr(*expr, 'r') != NULL) {
            is_polar = true;
            break;
        }
        // y is r
        // x is t
    }
    int h = (int)ceill(sy * (deltaY > deltaX ? (deltaY / deltaX) : 1));
    int w = (int)ceill(sx * (deltaY > deltaX ? 1 : (deltaX / deltaY)));
    if (is_polar && type == normal) {
        // if (x2 - x1 < M_PI * 2) {
        //     h = sy * ((sinl(x2) * sinl(x1) > 0) ? ceill(_y2 * fmaxl(fabsl(sinl(x1)), fabsl(sinl(x2)))) : fabsl(ceill(_y2 * (sinl(x2) - sinl(x1)))));
        //     w = sx * ((cosl(x2) * cosl(x1) > 0) ? ceill(_y2 * fmaxl(fabsl(cosl(x1)), fabsl(cosl(x2)))) : fabsl(ceill(_y2 * (cosl(x2) - cosl(x1)))));
        // } else {
        h = 2 * sy;
        w = 2 * sx;
        // }
        logger(DEBUG_LOG, "h = %d, w = %d", h, w);
    }
    //    int h = ceill(sy);
    //    int w = ceill(sx);
    number_t dx = deltaX / w;
    number_t dy = deltaY / h;
    logger(DEBUG_LOG, "x = %d, y = %d", h, w);
    unsigned char* rgba = malloc(
        sizeof(unsigned char) * (w + LEFT_MARGIN + RIGHT_MARGIN + LEFT_PADDING + RIGHT_PADDING) *
        (h + TOP_MARGIN + END_MARGIN + TOP_PADDING + END_PADDING) * 4);
    if (rgba == NULL) logger(ERR_LOG, "malloc rgba failed, size = %d, error: %s", sizeof(unsigned char) * (w + LEFT_MARGIN + RIGHT_MARGIN + LEFT_PADDING + RIGHT_PADDING) *
        (h + TOP_MARGIN + END_MARGIN + TOP_PADDING + END_PADDING) * 4, strerror(errno));

    unsigned char* p = rgba;
    for (int i = 0; i < (w + LEFT_MARGIN + RIGHT_MARGIN + LEFT_PADDING + RIGHT_PADDING) *
        (h + TOP_MARGIN + END_MARGIN + TOP_PADDING + END_PADDING); i++) {
        *p++ = GET_R(bg_color);
        *p++ = GET_G(bg_color);
        *p++ = GET_B(bg_color);
        *p++ = GET_A(bg_color);
    }
    switch (type) {
    case normal:
        plot_buffer(argv, rgba, h, w, dx, dy, brush_color, is_polar);
        break;
    case parametric:
        plot_parametric_buffer(argv, rgba, h, w, dx, dy, brush_color);
        break;
    case function:
        plot_function_buffer(argv, rgba, h, w, dx, dy, brush_color);
        break;
    case polar:
        plot_polar_buffer(argv, rgba, h, w, dx, dy, brush_color);
        break;
    default:
        logger(ERR_LOG, "plot png by type, unreachable!");
        break;
    }
    if (plot_x_axis) {
        char x_axis[] = "y=0";
        char* x_scale = NULL;
        alloc_sprintf(&x_scale, "x=CEIL(X/%Lf)*%Lf,y>0,y<%Lf", x_interval, x_interval, x_scale_len);
        char* x_axis_args[] = { x_axis, x_scale, NULL };
        plot_buffer(x_axis_args, rgba, h, w, dx, dy, x_scale_color, false);
    }
    if (plot_y_axis) {
        char x_axis[] = "x=0";
        char* x_scale = NULL;
        alloc_sprintf(&x_scale, "y=CEIL(Y/%Lf)*%Lf,x>0,x<%Lf", y_interval, y_interval, y_scale_len);
        char* x_axis_args[] = { x_axis, x_scale, NULL };
        plot_buffer(x_axis_args, rgba, h, w, dx, dy, y_scale_color, false);
    }
    if (output_file == NULL) {
        output_file = stdout;
    }
    svpng(
        output_file,
        w + LEFT_MARGIN + RIGHT_MARGIN + LEFT_PADDING + RIGHT_PADDING,
        h + TOP_MARGIN + END_MARGIN + TOP_PADDING + END_PADDING,
        rgba,
        1
    );
    free(rgba);
    //    free(z_cache);
}

#define PLOT_Impl(x) PLOT(x) { \
    plot_png_by_type(argv, x); \
}

PLOT_Impl(normal)

PLOT_Impl(polar)

PLOT_Impl(parametric)

PLOT_Impl(function)

void plot_png(char** argv) {
    plot_png_by_type(argv, normal);
}

INIT_CPLOT(normal) {
    init(argv);
}


void init_with_t(char** argv) {
    _y1 = eval_value(0, 0, *argv++);
    _y2 = eval_value(0, 0, *argv++);
    sy = eval_value(0, 0, *argv++);
    x1 = eval_value(0, 0, *argv++);
    x2 = eval_value(0, 0, *argv++);
    sx = eval_value(0, 0, *argv++);
    t1 = eval_value(0, 0, *argv++);
    t2 = eval_value(0, 0, *argv++);
    st = eval_value(0, 0, *argv++);
    deltaX = x2 - x1;
    deltaY = _y2 - _y1;
    logger(DEBUG_LOG, "init: %Le, %Le, %Le, %Le, %Le, %Le, %Lf, %Lf, %Lf\n", _y1, _y2, sy, x1, x2, sx, t1, t2, st);
}
INIT_CPLOT(polar) {
    init_with_t(argv);
}
INIT_CPLOT(parametric) {
    init_with_t(argv);
}

INIT_CPLOT(function) {
    init(argv);
}
