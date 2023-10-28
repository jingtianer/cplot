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
static number_t x1, x2, _y1, _y2, s1, s2;
static number_t deltaX, deltaY;

static unsigned int brush_size = 0;
static unsigned int bg_color = 0xFFFFFFFF;
static unsigned int brush_color = 0x000000FF;
static unsigned char R = 0x00, G = 0x00, B = 0x00, A = 0x00;
static unsigned char BG_R = 0xFF, BG_G = 0xFF, BG_B = 0xFF, BG_A = 0xFF;
static bool fast_mode = false;
static FILE* output_file = NULL;
void enable_fastmode(bool enable) {
    fast_mode = enable;
    logger(DEBUG_LOG, "enable fast mode = %s", (enable ? "true" : "false"));
}
#define SET(x, type) void set_##x(type n)
#define SETImpl(x, type, formatter) SET(x, type) { x = n; logger(DEBUG_LOG, "set " #x " = " formatter, x); }
SETImpl(brush_size, u_int32_t, "%u");
SETImpl(output_file, FILE*, "%p");
SETImpl(R, u_int8_t, "%u");
SETImpl(G, u_int8_t, "%u");
SETImpl(B, u_int8_t, "%u");
SETImpl(A, u_int8_t, "%u");
SETImpl(BG_R, u_int8_t, "%u");
SETImpl(BG_G, u_int8_t, "%u");
SETImpl(BG_B, u_int8_t, "%u");
SETImpl(BG_A, u_int8_t, "%u");
SET(brush_color, u_int32_t) {
    brush_color = n;
    R = (n >> (3 << 3)) & 0xff;
    G = (n >> (2 << 3)) & 0xff;
    B = (n >> (1 << 3)) & 0xff;
    A = (n >> (0 << 3)) & 0xff;
    logger(DEBUG_LOG, "set brush_color = %u", brush_color);
}
SET(bg_color, u_int32_t) {
    bg_color = n;
    BG_R = (n >> (3 << 3)) & 0xff;
    BG_G = (n >> (2 << 3)) & 0xff;
    BG_B = (n >> (1 << 3)) & 0xff;
    BG_A = (n >> (0 << 3)) & 0xff;
    logger(DEBUG_LOG, "set bg_color = %u", bg_color);
}
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
                    if (n > 1 || n < -1) return DBL_MAX;
                    op_func = acosl;
                    break;
                case op_asin:
                    if (n > 1 || n < -1) return DBL_MAX;
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
                    if (n < 0) return DBL_MAX;
                    op_func = logl;
                    break;
                case op_sqrt:
                    if (n < 0) return DBL_MAX;
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
        if (!eval_cmp(y, x, start, z)) {
            ret = false;
            break;
        }
        start = end + 1;
    }
    if ((ret) && !eval_cmp(y, x, start, z)) {
        ret = false;
    }
    free(expr);
    return ret;
}

void init(char** argv) {
    _y1 = eval_value(0, 0, *argv++);
    _y2 = eval_value(0, 0, *argv++);
    s1 = eval_value(0, 0, *argv++);
    x1 = eval_value(0, 0, *argv++);
    x2 = eval_value(0, 0, *argv++);
    s2 = eval_value(0, 0, *argv++);
    deltaX = x2 - x1;
    deltaY = _y2 - _y1;
    logger(DEBUG_LOG, "init: %Le, %Le, %Le, %Le, %Le, %Le\n", _y1, _y2, s1, x1, x2, s2);
}
#define INIT(x) void init_##x(number_t n)   
#define INIT_IMPL(x) INIT(x) {x = n; logger(DEBUG_LOG, #x " = %lE", n);}
INIT_IMPL(_y1)
INIT_IMPL(_y2)
INIT_IMPL(s1)
INIT_IMPL(x1)
INIT_IMPL(x2)
INIT_IMPL(s2)
INIT_IMPL(deltaX)
INIT_IMPL(deltaY)
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

void static draw(unsigned char* rgba, int i, int j, int w, int h, int radius) {
    for (int y = max(-radius, -i); y <= min(radius, h - i - 1); y++) {
        for (int x = max(-radius, -j); x <= min(radius, w - j - 1); x++) {
            if (sqrt(x * x + y * y) > radius) continue;
            unsigned char* p = rgba + 4 * w * (i + y) + 4 * (j + x);
            *p++ = R;
            *p++ = G;
            *p++ = B;
            *p++ = A;
        }
    }
}

void plot_png(char** argv) {
    //    int expr_cnt = 0;
    //    for(char **expr = argv; *expr; expr++) expr_cnt++;
    int h = (int)ceill(s1 * (deltaY > deltaX ? (deltaY / deltaX) : 1));
    int w = (int)ceill(s2 * (deltaY > deltaX ? 1 : (deltaX / deltaY)));
    //    int h = ceill(s1);
    //    int w = ceill(s2);
    number_t dx = deltaX / w;
    number_t dy = deltaY / h;
    logger(DEBUG_LOG, "x = %d, y = %d", h, w);
    unsigned char* rgba = malloc(
        sizeof(unsigned char) * (w + LEFT_MARGIN + RIGHT_MARGIN + LEFT_PADDING + RIGHT_PADDING) *
        (h + TOP_MARGIN + END_MARGIN + TOP_PADDING + END_PADDING) * 4);
    if (rgba == NULL) logger(ERR_LOG, "malloc rgba failed, size = %d, error: %s", sizeof(unsigned char) * (w + LEFT_MARGIN + RIGHT_MARGIN + LEFT_PADDING + RIGHT_PADDING) *
        (h + TOP_MARGIN + END_MARGIN + TOP_PADDING + END_PADDING) * 4, strerror(errno));
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
    for (int i = 0; i < (w + LEFT_MARGIN + RIGHT_MARGIN + LEFT_PADDING + RIGHT_PADDING) *
        (h + TOP_MARGIN + END_MARGIN + TOP_PADDING + END_PADDING); i++) {
        *p++ = BG_R;
        *p++ = BG_G;
        *p++ = BG_B;
        *p++ = BG_A;
    }
    accu = max(dx, dy);
    //    z_cache_ptr = z_cache;
    //    z_cache_ptr += (w + LEFT_PADDING + RIGHT_PADDING + 2) * expr_cnt;
    for (int i = 0; i < h + TOP_PADDING + END_PADDING; i++) {
        //        z_cache_ptr += expr_cnt;
        for (int j = 0; j < w + LEFT_PADDING + RIGHT_PADDING; j++) {
            logger(DEBUG_LOG, "x = %lld, y = %lld", j, i);
            bool ok = false;
            number_t z0, zx, zy;
            number_t dzx = 0;
            number_t dzy = 0;
            //            number_t *zptr = z_cache_ptr;
            //            z_cache_ptr += expr_cnt;
            for (char** expr = argv; *expr; expr++) {
                //                z0 = *zptr++;
                if (!fast_mode) {
                    eval(-dy * (i - TOP_PADDING) + _y2,
                        dx * (j - LEFT_PADDING) + x1, *expr, &z0);
                    int off[] = { 1, -1, -1, 1, 1 };
                    accu = 0;
                    for (int offi = 0; offi <= 1; offi++) {
                        //                    zx = *(zptr + off[offi]*expr_cnt);
                        eval(-dy * (i - TOP_PADDING) + _y2,
                            dx * (j - LEFT_PADDING + off[offi]) + x1, *expr, &zx);
                        dzx = (zx - z0) / dx;
                        accu = max(accu, fabsl(z0 - zx));
                        //                    if((z0 > 0 && dzx < 0) || (z0 < 0 && dzx > 0)) break;
                    }

                    for (int offi = 0; offi <= 1; offi++) {
                        //                    zy = *(zptr + off[offi]*expr_cnt*(w + LEFT_PADDING + RIGHT_PADDING + 2));
                        eval(-dy * (i - TOP_PADDING + off[offi]) + _y2,
                            dx * (j - LEFT_PADDING) + x1, *expr, &zy);
                        dzy = (zy - z0) / dy;
                        accu = max(accu, fabsl(z0 - zy));
                        //                    if((z0 > 0 && dzy < 0) || (z0 < 0 && dzy > 0)) break;
                    }
                    accu = min(accu, max(dx, dy));
                }
                ok = eval(-dy * (i - TOP_PADDING) + _y2,
                    dx * (j - LEFT_PADDING) + x1, *expr, NULL);
                if (ok) goto draw;
                if (!fast_mode) {
                    if (z0 > 10 * max(dx, dy)) continue;
                    logger(DEBUG_LOG, "dzx = %Le", dzx);
                    logger(DEBUG_LOG, "dzy = %Le", dzy);
                    number_t maxd = min(100, floorl(max(fabsl(dzx), fabsl(dzy))));
                    for (int divx = 1; divx < maxd; divx++) {
                        ok = eval(-dy * (i - TOP_PADDING + divx / max(fabsl(dzx), fabsl(dzy))) + _y2,
                            dx * (j - LEFT_PADDING + divx / max(fabsl(dzx), fabsl(dzy))) + x1, *expr, NULL);
                        if (ok) goto draw;
                        ok = eval(-dy * (i - TOP_PADDING - divx / max(fabsl(dzx), fabsl(dzy))) + _y2,
                            dx * (j - LEFT_PADDING - divx / max(fabsl(dzx), fabsl(dzy))) + x1, *expr, NULL);
                        if (ok) goto draw;
                        ok = eval(-dy * (i - TOP_PADDING - divx / max(fabsl(dzx), fabsl(dzy))) + _y2,
                            dx * (j - LEFT_PADDING + divx / max(fabsl(dzx), fabsl(dzy))) + x1, *expr, NULL);
                        if (ok) goto draw;
                        ok = eval(-dy * (i - TOP_PADDING + divx / max(fabsl(dzx), fabsl(dzy))) + _y2,
                            dx * (j - LEFT_PADDING - divx / max(fabsl(dzx), fabsl(dzy))) + x1, *expr, NULL);
                        if (ok) goto draw;
                    }
                }
            }
        draw:
            if (ok) {
                draw(
                    rgba,
                    i + TOP_MARGIN, j + LEFT_MARGIN,
                    w + LEFT_MARGIN + RIGHT_MARGIN + LEFT_PADDING + RIGHT_PADDING,
                    h + TOP_MARGIN + END_MARGIN + TOP_PADDING + END_PADDING,
                    brush_size
                );
            }
        }
        //        z_cache_ptr += expr_cnt;
    }
    //    z_cache_ptr += (w + LEFT_PADDING + RIGHT_PADDING + 2) * expr_cnt;
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
