#define _XOPEN_SOURCE 500
#define _POSIX_C_SOURCE 200809L
#define _C99_SOURCE
#include "config.h"
#include "svpng/svpng.inc"
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <float.h>

typedef long double number_t;

void logger(int level, const char* format, ...) {
    if (level > LOG_LEVEL) return;
    va_list args;
    va_start(args, format);
    int len = vsnprintf(NULL, 0, format, args) + 1;
    va_end(args);
    va_start(args, format);
    char* str = malloc(len);
    vsprintf(str, format, args);
    va_end(args);
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

typedef enum platform_t {
    platform_apple = 1,
    platform_win32,
    platform_win64,
    platform_android,
    platform_linux,
    platform_unix,
    platform_posix,
    platform_other
} platform_t;

platform_t platform() {
#ifdef _WIN32
#ifdef _WIN64
    return platform_win64;
#else
    return platform_win32;
#endif
#elif __APPLE__
    return platform_apple;
#elif __ANDROID__
    return platform_android;
#elif __linux__
    return platform_linux;
#elif __unix__ // all unices not caught above
    return platform_unix;
#elif defined(_POSIX_VERSION)
    return platform_posix;
#else
    return platform_other;
#endif
}

int example(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int len = vsnprintf(NULL, 0, format, args) + 1;
    va_end(args);
    va_start(args, format);
    char* str = malloc(len);
    vsprintf(str, format, args);
    va_end(args);
#ifdef EXEC_EXAMPLES
    system(str);
#endif
    logger(INFO_LOG, "\t%s", str);
#ifdef OPEN_EXAMPLES
    if (platform() == platform_apple || platform() == platform_linux) {
        char* show;
        alloc_sprintf(&show, "open ./%s", strrchr(str, '>') + 1);
        system(show);
        free(show);
    } else if (platform() == platform_win32 || platform() == platform_win64) {
        char* show;
        alloc_sprintf(&show, "./%s", strrchr(str, '>') + 1);
        system(show);
        free(show);
    }
#endif
    free(str);
    return len;
}

number_t x1, x2, _y1, _y2, s1, s2;
number_t deltaX, deltaY;

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

void logStack() {
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

void biCheck() {
    if (EMPTY(stack)) {
        logger(ERR_LOG, "empty stack!");
        exit(1);
    }
}

void pushOP(char cur_op) {
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

int len_strncmp(const char* a, const char* b) {
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

void INIT(char** argv) {
    int i = 0;
    _y1 = eval_value(0, 0, argv[i++]);
    _y2 = eval_value(0, 0, argv[i++]);
    s1 = eval_value(0, 0, argv[i++]);
    x1 = eval_value(0, 0, argv[i++]);
    x2 = eval_value(0, 0, argv[i++]);
    s2 = eval_value(0, 0, argv[i++]);
    deltaX = x2 - x1;
    deltaY = _y2 - _y1;
    logger(DEBUG_LOG, "%Le, %Le, %Le, %Le, %Le, %Le\n", _y1, _y2, s1, x1, x2, s2);
}

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

void draw(unsigned char* rgba, int i, int j, int w, int h, int radius) {
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
    accu = 2 * max(dx, dy);
    //    z_cache_ptr = z_cache;
    //    z_cache_ptr += (w + LEFT_PADDING + RIGHT_PADDING + 2) * expr_cnt;
    for (int i = 0; i < h + TOP_PADDING + END_PADDING; i++) {
        //        z_cache_ptr += expr_cnt;
        for (int j = 0; j < w + LEFT_PADDING + RIGHT_PADDING; j++) {
            logger(DEBUG_LOG, "x = %lld, y = %lld", j, i);
            bool ok = false;
            number_t z0, zx, zy;
            //            number_t *zptr = z_cache_ptr;
            //            z_cache_ptr += expr_cnt;
            for (char** expr = argv; *expr; expr++) {
                //                z0 = *zptr++;
#ifndef FAST_MODE
                eval(-dy * (i - TOP_PADDING) + _y2,
                    dx * (j - LEFT_PADDING) + x1, *expr, &z0);
                number_t dzx = 0;
                number_t dzy = 0;
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
#endif
                ok = eval(-dy * (i - TOP_PADDING) + _y2,
                    dx * (j - LEFT_PADDING) + x1, *expr, &z0);
                if (ok) goto draw;
#ifndef FAST_MODE
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
#endif
            }
        draw:
            if (ok) {
                draw(
                    rgba,
                    i + TOP_MARGIN, j + LEFT_MARGIN,
                    w + LEFT_MARGIN + RIGHT_MARGIN + LEFT_PADDING + RIGHT_PADDING,
                    h + TOP_MARGIN + END_MARGIN + TOP_PADDING + END_PADDING,
                    BRUSH_SIZE
                );
            }
        }
        //        z_cache_ptr += expr_cnt;
    }
    //    z_cache_ptr += (w + LEFT_PADDING + RIGHT_PADDING + 2) * expr_cnt;

    svpng(
        stdout,
        w + LEFT_MARGIN + RIGHT_MARGIN + LEFT_PADDING + RIGHT_PADDING,
        h + TOP_MARGIN + END_MARGIN + TOP_PADDING + END_PADDING,
        rgba,
        1
    );
    free(rgba);
    //    free(z_cache);
}

#ifdef tupper

int main(int argc, char** argv) {
    if (argc != 3) {
        char* str;
        logger(INFO_LOG, "Usage: %s sy sx expression\nexamples:", argv[0]);
        example("%s 800 800 2>errs.log 1>tupper.png", argv[0]);
        exit(0);
    }
    char* expr = strdup("1/2<FLOOR((FLOOR(y/17)*2^(-17*FLOOR(X)-FLOOR(y)%17))%2)");
    number_t k = eval_value(0, 0, "4858450636189713423582095962494202044581400587983244549483093085061934704708809928450644769865524364849997247024915119110411605739177407856919754326571855442057210445735883681829823754139634338225199452191651284348332905131193199953502413758765239264874613394906870130562295813219481113685339535565290850023875092856892694555974281546386510730049106723058933586052544096664351265349363643957125565695936815184334857605266940161251266951421550539554519153785457525756590740540157929001765967965480064427829131488548259914721248506352686630476300");
    _y1 = k;
    _y2 = k + 17;
    x1 = 0;
    x2 = 107;
    deltaX = 107;
    deltaY = 17;
    s1 = eval_value(0, 0, argv[1]);
    s2 = eval_value(0, 0, argv[2]);
    char* plot_args[] = { expr, NULL };
    plot_png(plot_args);
    free(expr);
    return 0;
}
#else

#ifndef USE_CONSOLE

int main(int argc, char** argv) {
    if (argc < 8) {
        char* str;
        logger(INFO_LOG, "Usage: %s y1 y2 sy x1 x2 sx expression\nexamples:", argv[0]);
        example("%s \"-1\" 1 300 -1 1 300 \"x*x+y*y-1=0\" 2>errs.log 1>out1.png", argv[0]);
        example("%s \"-pi/2\" \"pi/2\" 300 \"-3*pi\" \"2*pi\" 300 \"y^2-SIN(x+y)^2=0\" 2>errs.log 1>out2.png", argv[0]);
        example("%s \"-pi/2\" \"pi/2\" 300 \"-3*pi\" \"2*pi\" 300 \"y^2-SIN(x)^2=0\" 2>errs.log 1>out3.png", argv[0]);
        example("%s \"-2\" \"ACOS(1/2)-pi/4\" 300 \"-pi/2\" \"pi/2\" 300 \"y*y+x*x+y-SQRT(y*y+x*x)=0\" 2>errs.log 1>out4.png",
            argv[0]);
        example("%s \"-pi\" \"1\" 300 \"-2\" \"2\" 300 \"(ACOS(1-FABS(x))-pi)-y=0\" \"y-SQRT(1-(FABS(x)-1)^2)=0\" 2>errs.log 1>out5.png",
            argv[0]);
        example("%s \"-1\" \"pi/2\" 300 \"-1\" \"1\" 300 \"x*x+(y-FABS(x)^(2/3.0))^2-1=0\" 2>errs.log 1>out6.png",
            argv[0]);
        example("%s \"-4\" \"4\" \"300\" \"0\" \"2*pi\" \"300\" \"y-5*EXP(-x)*SIN(6*x)=0\" 2>errs.log 1>out7.png",
            argv[0]);
        example("%s \"0\" \"3\" 300 \"0\" \"9\" 300 \"y-SQRT(9-x)=0\" 2>errs.log 1>out8.png", argv[0]);
        example("%s \"0\" \"1\" 300 \"0\" \"1\" 300 \"y-X=0\" 2>errs.log 1>out9.png", argv[0]);
        example("%s \"-1.5*pi\" \"4.5*pi\" 300 \"-1.5*pi\" \"4.5*pi\" 300 \"SIN(X)+SIN(Y)=0\" 2>errs.log 1>out10.png",
            argv[0]);
        example("%s \"-1.5*pi\" \"4.5*pi\" 300 \"-1.5*pi\" \"4.5*pi\" 300 \"SIN(X)*SIN(Y)=0\" 2>errs.log 1>out11.png",
            argv[0]);
        example("%s \"-8*pi\" \"8*pi\" 300 \"-8*pi\" \"8*pi\" 300 \"COS(x+SIN(y))-TAN(y)=0\" 2>errs.log 1>out12.png",
            argv[0]);
        example("%s \"-pi\" \"1\" 300 \"-2\" \"2\" 300 \"(ACOS(1-FABS(x))-pi)-y<=0,y-SQRT(1-(FABS(x)-1)^2)<=0\" 2>errs.log 1>out13.png",
            argv[0]);
        example("%s \"-1\" \"2\" 300 \"-1\" \"4\" 300 \"y-x=0,y-SQRT(x)=0\" 2>errs.log 1>out14.png",
            argv[0]); // 求交点的情况 and
        example("%s \"-1\" \"2\" 300 \"-1\" \"4\" 300 \"y-x=0\" \"y-SQRT(x)=0\" 2>errs.log 1>out15.png",
            argv[0]); // 求交点的情况 or
        example("%s \"-2*pi\" \"2*pi\" \"800\" \"-2*pi\" \"2*pi\" \"800\" \"SIN(X*x+Y*y)-SIN(X)-SIN(Y)=0\" 2>errs.log 1>out16.png",
            argv[0]);
        example("%s \"-pi\" \"pi\" \"800\" \"-pi\" \"pi\" \"800\" \"SIN(X*x+Y*y)-COS(X*Y)=0\" 2>errs.log 1>out17.png",
            argv[0]);
        example("%s \"-pi\" \"pi\" \"800\" \"-pi\" \"pi\" \"800\" \"SIN(X*x+Y*y)-COS(X-Y)=0\" 2>errs.log 1>out18.png",
            argv[0]);
        example("%s \"-4\" \"4\" \"300\" \"-4\" \"4\" \"300\" \"FLOOR(X)-y=0\" 2>errs.log 1>out19.png", argv[0]);
        example("%s \"-4*pi\" \"4*pi\" \"800\" \"-4*pi\" \"4*pi\" \"800\" \"SIN(SIN(X*Y))=0\" 2>errs.log 1>out20.png",
            argv[0]);
        example("%s \"-1\" 1 300 -8 8 300 \"(COS(pi*X)+COS(pi*X^2))/2=y\" 2>errs.log 1>out21.png", argv[0]);
        example("%s \"-1\" 1 300 -2.5 2.5 300 \"(COS(pi*X)+COS(pi*X^2)+COS(pi*X^3))/3=y\" 2>errs.log 1>out22.png", argv[0]);
        example("%s \"-3*pi/2\" \"3*pi/2\" 300 \"-3*pi/2\" \"3*pi/2\" 300 \"SIN(x*x)+SIN(y*y)=1\"  2>errs.log 1>out23.png",
            argv[0]);
        example("%s \"-10\" \"10\" 2000 \"-10\" \"10\" 2000 \"Y=X^X\"  2>errs.log 1>out24.png", argv[0]);
        example("%s \"0\" \"10\" 300 \"-8\" \"8\" 300 \"Y=10/(1+EXP(-X))\" 2>errs.log 1>out25.png", argv[0]);
        example("%s \"-1\" \"1\" 300 \"-2*pi\" \"2*pi\" 300 \"Y=SIN(1/X)\" 2>errs.log 1>out26.png", argv[0]);
        exit(0);
    }
    INIT(argv + 1);
    plot_png(argv + 7);
    return 0;
}

#else
int main(int argc, char** argv) {
    if (argc < 8) {
        logger(INFO_LOG, "Usage: %s y1 y2 sy x1 x2 sy expression\nexamples:", argv[0]);
        example("%s \"-1\" 1 0.125 -1 1 0.0625 \"x*x+y*y-1>0\" 2>errs.log 1>out1", argv[0]);
        example("%s \"-pi/2\" \"pi/2\" 0.25 \"-3*pi\" \"2*pi\" 0.125 \"y^2-SIN(x+y)^2<0\" 2>errs.log 1>out2", argv[0]);
        example("%s \"-pi/2\" \"pi/2\" 0.25 \"-3*pi\" \"2*pi\" 0.125 \"y^2-SIN(x)^2<0\" 2>errs.log 1>out3", argv[0]);
        example("%s \"-2\" \"ACOS(1/2)-pi/4\" 0.125 \"-pi/2\" \"pi/2\" 0.0625 \"y*y+x*x+y-SQRT(y*y+x*x)>0\" 2>errs.log 1>out4", argv[0]);
        example("%s \"-pi\" \"1\" 0.125 \"-2\" \"2\" 0.0625 \"(ACOS(1-FABS(x))-pi)-y<0,y-SQRT(1-(FABS(x)-1)^2)<0\" 2>errs.log 1>out5", argv[0]);
        example("%s \"-1\" \"pi/2\" 0.125 \"-1\" \"1\" 0.0625 \"x*x+(y-FABS(x)^(2.0/3.0))^2-1<0\" 2>errs.log 1>out6", argv[0]);
        example("%s \"-4\" \"4\" \"8/32\" \"0\" \"2*pi\" \"2*pi/64\" \"y-5*EXP(-x)*SIN(6*x)<0\" 2>errs.log 1>out7", argv[0]);
        example("%s \"0\" \"3\" \"3/32\" \"0\" \"9\" \"9/32\" \"y-SQRT(X)>0\" 2>errs.log 1>out8", argv[0]);
        example("%s \"0\" \"1\" \"1/32\" \"0\" \"1\" \"1/32\" \"y-X<0\" 2>errs.log 1>out9", argv[0]);
        example("%s \"-1.5*pi\" \"4.5*pi\" \"6*pi/32\" \"-1.5*pi\" \"4.5*pi\" \"6*pi/32\" \"SIN(X)+SIN(Y)>0\" 2>errs.log 1>out10", argv[0]);
        example("%s \"-1.5*pi\" \"4.5*pi\" \"6*pi/32\" \"-1.5*pi\" \"4.5*pi\" \"6*pi/64\" \"SIN(X)*SIN(Y)>0\" 2>errs.log 1>out11", argv[0]);
        exit(0);
    }
    INIT(argv + 1);
    accu = max(s1, s2);
    for (number_t i = _y2; i >= _y1; ) {
        for (number_t j = x1; j <= x2; ) {
            logger(DEBUG_LOG, "x = %Le, y = %Le", j, i);
            bool ok = false;
            for (char** expr = argv + 7; *expr; expr++) {
                ok = eval(i, j, *expr, NULL);
                if (ok) {
                    break;
                }
            }
            if (ok) {
                printf("%c", INNER_CHAR);
            } else {
                printf("%c", OUTER_CHAR);
            }
            j += s2;
        }
        printf("\n");
        i -= s1;
    }
    return 0;
}
#endif
#endif