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
            PUSH(stack, x);
            break;
        case 'y':
        case 'Y':
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
                    if (n > 1 || n < -1) return 0.0/0.0;
                    op_func = acosl;
                    break;
                case op_asin:
                    if (n > 1 || n < -1) return 0.0/0.0;
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
                    if (n < 0) return 0.0/0.0;
                    op_func = logl;
                    break;
                case op_sqrt:
                    if (n < 0) return 0.0/0.0;
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
        if (!eval_cmp(y, x, start, z) || fpclassify(*z) == FP_NAN) {
            ret = false;
            break;
        }
        start = end + 1;
    }
    if(ret) {
        if (!eval_cmp(y, x, start, z) || fpclassify(*z) == FP_NAN) {
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
            if(
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

bool get_slop(number_t i, number_t j, number_t dy, number_t dx, const char *expr, number_t z0, number_t *dzy, number_t *dzx, number_t *max_dz) {
    if(!dzx || !dzy || !max_dz) {
        logger(ERR_LOG, "get_slop, null");
    }
    *max_dz = 0;
    int off[] = { 1, -1, -1, 1, 1 };
    number_t zx, zy;
    *dzx = *dzy = 0;
    bool ok = false;
    for (int offi = 0; offi < 2; offi++) {
        eval(-dy * (i - TOP_PADDING) + _y2,
            dx * (j - LEFT_PADDING - off[offi]) + x1, expr, &zx);
        
        number_t dzxi = off[offi]*fabsl(zx-z0)/(dx);
        *max_dz = max(*max_dz, fabsl(z0 - zx));
        if((z0 > 0 && zx < 0) || (z0 < 0 && zx> 0)) {
            ok = (fpclassify(z0) != FP_NAN && fpclassify(zx) != FP_NAN);
        }
    }
    for (int offi = 0; offi < 2; offi++) {
        eval(-dy * (i - TOP_PADDING - off[offi]) + _y2,
            dx * (j - LEFT_PADDING ) + x1, expr, &zy);
        number_t dzyi = off[offi]*fabsl(zy-z0)/(dy);
        *max_dz = max(*max_dz, fabsl(zy-z0));
        if((z0 > 0 && zy < 0) || (z0 < 0 && zy > 0)) {
            ok = (fpclassify(z0) != FP_NAN && fpclassify(zy) != FP_NAN);
        }
    }
    return ok;
}
void plot_buffer(char** argv, unsigned char *rgba, int h, int w, number_t dx, number_t dy, unsigned int color) {
    //    int expr_cnt = 0;
    //    for(char **expr = argv; *expr; expr++) expr_cnt++;
    //    number_t *z_cache = malloc(
    //            sizeof(number_t) * (w + LEFT_PADDING + RIGHT_PADDING + 2) * //上下左右多算一行/列
    //            (h + TOP_PADDING + END_PADDING + 2) * expr_cnt), *z_cache_ptr;
    //    z_cache_ptr = z_cache;
    //    for (int i = -1; i < h + TOP_PADDING + END_PADDING + 1; i++) {
    //        for (int j = -1; j < w + LEFT_PADDING + RIGHT_PADDING + 1; j++) {
    //            for(char **expr = argv; *expr; expr++, z_cache_ptr++)
    //                eval(-dy * (i - TOP_PADDING) + _y2, dx * (j - LEFT_PADDING) + x1, *expr, z_cache_ptr, false);
    //        }
    //    }
    unsigned char* p = rgba;
    accu = max(dx, dy);
    //    z_cache_ptr = z_cache;
    //    z_cache_ptr += (w + LEFT_PADDING + RIGHT_PADDING + 2) * expr_cnt;
    for (int i = 0; i < h + TOP_PADDING + END_PADDING; i++) {
        //        z_cache_ptr += expr_cnt;
        for (int j = 0; j < w + LEFT_PADDING + RIGHT_PADDING; j++) {
            logger(DEBUG_LOG, "x = %lld, y = %lld", j, i);
            // if(*rgba == GET_R(brush_color)) continue;
            bool ok = false;
            number_t z0, max_dz;
            number_t dzx = 0, dzy = 0;
            //            number_t *zptr = z_cache_ptr;
            //            z_cache_ptr += expr_cnt;
            for (char** expr = argv; *expr; expr++) {
                //                z0 = *zptr++;
                if (!fast_mode) {
                    eval(-dy * (i - TOP_PADDING + dzy) + _y2,
                         dx * (j - LEFT_PADDING + dzx) + x1, *expr, &z0);
                    if(fpclassify(z0) == FP_NAN) continue;
                    ok = get_slop(i, j, dy, dx, *expr, z0, &dzy, &dzx, &max_dz);
                    if(ok) goto draw;
                    if(fpclassify(max_dz) == FP_NAN) continue;
                    accu = min(max(dy, dx), max_dz);
                }
                ok = eval(-dy * (i - TOP_PADDING) + _y2,
                          dx * (j - LEFT_PADDING) + x1, *expr, &z0);
                if (ok) goto draw;
            }
        draw:
            if (ok) {
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
        //        z_cache_ptr += expr_cnt;
    }
    //    z_cache_ptr += (w + LEFT_PADDING + RIGHT_PADDING + 2) * expr_cnt;
    //    free(z_cache);
}

void plot_png(char** argv) {
    //    int expr_cnt = 0;
    //    for(char **expr = argv; *expr; expr++) expr_cnt++;
    int h = (int)ceill(sy * (deltaY > deltaX ? (deltaY / deltaX) : 1));
    int w = (int)ceill(sx * (deltaY > deltaX ? 1 : (deltaX / deltaY)));
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
    plot_buffer(argv, rgba, h, w, dx, dy, brush_color);
    if(plot_x_axis) {
        char x_axis[] = "y=0";
        char *x_scale = NULL;
        alloc_sprintf(&x_scale, "x=CEIL(X/%Lf)*%Lf,y>0,y<%Lf", x_interval, x_interval, x_scale_len);
        char *x_axis_args[] = {x_axis, x_scale, NULL};
        plot_buffer(x_axis_args, rgba, h, w, dx, dy, x_scale_color);
    }
    if(plot_y_axis) {
        char x_axis[] = "x=0";
        char *x_scale = NULL;
        alloc_sprintf(&x_scale, "y=CEIL(Y/%Lf)*%Lf,x>0,x<%Lf", y_interval, y_interval, y_scale_len);
        char *x_axis_args[] = {x_axis, x_scale, NULL};
        plot_buffer(x_axis_args, rgba, h, w, dx, dy, y_scale_color);
    }
    if(output_file == NULL) {
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

char inner_char = '+', outer_char = ' ';

SETImpl(inner_char, char, "%c")
SETImpl(outer_char, char, "%c")
void plot_console(char **argv) {
    if(output_file == NULL) {
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