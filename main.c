#define _XOPEN_SOURCE 500
#define _C99_SOURCE
#include "svpng/svpng.inc"
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <float.h>
//#define LOG_LEVEL DEBUG_LOG
#define DEBUG_LOG 7
#define INFO_LOG 5
#define ERR_LOG 4
#ifndef LOG_LEVEL
#define LOG_LEVEL INFO_LOG
#endif
#ifndef GREATER_CHAR
#define GREATER_CHAR ' '
#endif
#ifndef SMALLER_CHAR
#define SMALLER_CHAR '+'
#endif
void logger(int level, const char *format, ...) {
    if(level > LOG_LEVEL) return;
    va_list args;
    va_start(args, format);
    int len = vsnprintf(NULL, 0, format, args)+1;
    va_end(args);
    va_start(args, format);
    char *str = malloc(len);
    vsprintf(str, format, args);
    va_end(args);
    fprintf(stderr, "%s\n", str);
    free(str);
}
double x1, x2, _y1, _y2, s1, s2;

#define PUSH(s, n) (s[s##_ptr++] = (n))
#define POP(s, n) (n = s[--s##_ptr])
#define TOP(s, n) (n = s[s##_ptr-1])
#define EMPTY(s) (s##_ptr == 0)
enum {
    op_acos     = 48,
    op_asin         ,
    op_atan         ,
    op_cos          ,
    op_cosh         ,
    op_sin          ,
    op_sinh         ,
    op_tan          ,
    op_tanh         ,
    op_exp          ,
    op_log          ,
    op_floor        ,
    op_sqrt         ,
    op_fabs         ,
    op_ceil
};
const static int op_min = op_acos;
const static int op_max = op_ceil;

static double stack[1024];
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
    for(int cnt = 0; cnt < op_stack_ptr; cnt++) {
        logger(DEBUG_LOG, "%c ", op_stack[cnt]);
    }
    logger(DEBUG_LOG, "stack: ");
    for(int cnt = 0; cnt < stack_ptr; cnt++) {
        logger(DEBUG_LOG, "%lf ", stack[cnt]);
    }
    logger(DEBUG_LOG, "");
}
void biCheck() {
    if(EMPTY(stack)) {
        logger(ERR_LOG, "empty stack!");
        exit(1);
    }
}
void pushOP(char cur_op) {
    double n1, n2;
    char op;
    while(!EMPTY(op_stack) && priv[TOP(op_stack, op)] >= priv[cur_op]) {
        POP(op_stack, op);
        double res = 0;
        POP(stack, n2);
        if(!EMPTY(stack))POP(stack, n1);
        else n1 = 0;
        switch(op) {
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
                if(n2 == 0) {
                    logger(ERR_LOG, "divisor is zeor!");
//                    exit(1);
                }
                res = n1 / n2;
                break;
            case '%':
                if(n2 == 0) {
                    logger(ERR_LOG, "divisor is zeor!");
//                    exit(1);
                }
                res = fmod(n1 , n2);
                break;
            case '^':
                res = pow(n1, n2);
                break;
            default:
                break;
        }
        PUSH(stack, res);
    }
}
int len_strncmp(const char *a, const char *b) {
    return strncmp(a, b, strlen(b));
}
double eval(double y, double x, const char *expr) {
    int len = strlen(expr);
    int i = 0;
    stack_ptr = 0;
    op_stack_ptr = 0;
    while(i < len) {
        switch(expr[i]) {
            case '^':
                biCheck();
                pushOP(expr[i]);
                // if(EMPTY(stack)) PUSH(stack, 0);
                PUSH(op_stack, expr[i]);
                break;
            case 'x': case 'X':
                PUSH(stack, x);
                break;
            case 'y': case 'Y':
                PUSH(stack, y);
                break;
            case '(':
                PUSH(op_stack, '(');
                break;
            case ')': {
                double n1, n2;
                char op;
                if(EMPTY(op_stack)) {
                    logger(ERR_LOG, "no match ')'");
                    exit(1);
                }
                pushOP(')');
                POP(op_stack, op);
                if(TOP(op_stack, op) >= op_min && TOP(op_stack, op) <= op_max) {
                    POP(op_stack, op);
                    double n;
                    if(!EMPTY(stack)) {
                        POP(stack, n);
                    } else {
                        logger(ERR_LOG, "Error : no opvalue");
                        exit(1);
                    }
                    double (*op_func)(double);
                    switch(op) {
                        case op_acos:
                            if(n > 1 || n < -1) return DBL_MAX;
                            op_func = acos;
                            break;
                        case op_asin:
                            if(n > 1 || n < -1) return DBL_MAX;
                            op_func = asin;
                            break;
                        case op_atan:
                            op_func = atan;
                            break;
                        case op_cos:
                            op_func = cos;
                            break;
                        case op_cosh:
                            op_func = cosh;
                            break;
                        case op_sin:
                            op_func = sin;
                            break;
                        case op_sinh:
                            op_func = sinh;
                            break;
                        case op_tan:
                            op_func = tan;
                            break;
                        case op_tanh:
                            op_func = tanh;
                            break;
                        case op_exp:
                            op_func = exp;
                            break;
                        case op_log:
                            if(n < 0) return DBL_MAX;
                            op_func = log;
                            break;
                        case op_sqrt:
                            if(n < 0) return DBL_MAX;
                            op_func = sqrt;
                            break;
                        case op_fabs:
                            op_func = fabs;
                            break;
                        case op_ceil:
                            op_func = ceil;
                            break;
                        case op_floor:
                            op_func = floor;
                            break;
                    }
                    PUSH(stack, op_func(n));
                }
            }
                break;
            case '+':case '-': {
                if((i > 0 && expr[i-1] != '(')) { // fix： a-(-b)
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
                if(i + 1 < len && expr[i + 1] == 'i') {
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
            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':case '.':
            {
                double n = 0;
                double n1 = 1;
                while(i < len) {
                    if(expr[i] < '0' || expr[i] > '9') {
                        break;
                    }
                    n *= 10;
                    n += expr[i] - '0';
                    i++;
                }
                if(expr[i] == '.') {
                    i++;
                    while(i < len && expr[i] != '.') {
                        if(expr[i] < '0' || expr[i] > '9') {
                            break;
                        }
                        n1 /= 10.0;
                        n += n1 * (expr[i] - '0');
                        i++;
                    }
                }if(expr[i] == '.') {
                    logger(ERR_LOG, "error : two '.' in one number");
                    exit(1);
                }
                PUSH(stack, n);
                i--;
            }
                break;
            default:
                if(!len_strncmp(expr + i, "ACOS")) {
                    PUSH(op_stack, op_acos);
                    i+=3;
                } else if(!len_strncmp(expr + i, "ASIN")) {
                    PUSH(op_stack, op_asin);
                    i+=3;
                } else if(!len_strncmp(expr + i, "ATAN")) {
                    PUSH(op_stack, op_atan);
                    i+=3;
                } else if(!len_strncmp(expr + i, "COS")) {
                    if(expr[i+3] == 'H') {
                        PUSH(op_stack, op_cosh);
                        i+=3;
                    } else {
                        PUSH(op_stack, op_cos);
                        i+=2;
                    }
                } else if(!len_strncmp(expr + i, "SIN")) {
                    if(expr[i+3] == 'H') {
                        PUSH(op_stack, op_sinh);
                        i+=3;
                    } else {
                        PUSH(op_stack, op_sin);
                        i+=2;
                    }
                } else if(!len_strncmp(expr + i, "TAN")) {
                    if(expr[i+3] == 'H') {
                        PUSH(op_stack, op_tanh);
                        i+=3;
                    } else {
                        PUSH(op_stack, op_tan);
                        i+=2;
                    }
                } else if(!len_strncmp(expr + i, "EXP")) {
                    PUSH(op_stack, op_exp);
                    i+=2;
                } else if(!len_strncmp(expr + i, "LOG")) {
                    PUSH(op_stack, op_log);
                    i+=2;
                } else if(!len_strncmp(expr + i, "SQRT")) {
                    PUSH(op_stack, op_sqrt);
                    i+=3;
                } else if(!len_strncmp(expr + i, "FABS")) {
                    PUSH(op_stack, op_fabs);
                    i+=3;
                } else if(!len_strncmp(expr + i, "CEIL")) {
                    PUSH(op_stack, op_ceil);
                    i+=3;
                } else if(!len_strncmp(expr + i, "FLOOR")) {
                    PUSH(op_stack, op_floor);
                    i+=4;
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
void INIT(char **argv) {
    int i = 0;
    _y1 = eval(0, 0, argv[i++]);
    _y2 = eval(0, 0, argv[i++]);
    s1 = eval(0, 0, argv[i++]);
    x1 = eval(0, 0, argv[i++]);
    x2 = eval(0, 0, argv[i++]);
    s2 = eval(0, 0, argv[i++]);
    logger(DEBUG_LOG, "%lf, %lf, %lf, %lf, %lf, %lf\n", _y1, _y2, s1, x1, x2, s2);
}
#define LEFT_MARGIN 10
#define RIGHT_MARGIN 10
#define TOP_MARGIN 10
#define END_MARGIN 10
#define LEFT_EXTRA_PIXEL 10
#define RIGHT_EXTRA_PIXEL 10
#define TOP_EXTRA_PIXEL 10
#define END_EXTRA_PIXEL 10
#define R 255
#define G 255
#define B 255
#define A 255
#define ACCURACY 0.01
#define X_PAINTERSIZE 10
#define Y_PAINTERSIZE 30
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#ifndef USE_CHAR
int main(int argc, char **argv) {
    if(argc < 8) {
        logger(INFO_LOG, "Usage: %s y1 y2 sy x1 x2 sy expression", argv[0]);
        logger(INFO_LOG, "%s -1 1 0.125 -1 1 0.0625 \"x*x+y*y-1\" 2>errs.log", argv[0]);
        logger(INFO_LOG, "%s \"-pi/2\" \"pi/2\" 0.25 \"-3*pi\" \"2*pi\" 0.125 \"y^2-SIN(x+y)^2\" 2>errs.log", argv[0]);
        logger(INFO_LOG, "%s \"-pi/2\" \"pi/2\" 0.25 \"-3*pi\" \"2*pi\" 0.125 \"y^2-SIN(x)^2\" 2>errs.log", argv[0]);
        logger(INFO_LOG, "%s \"-2\" \"ACOS(1/2)-pi/4\" 0.125 \"-pi/2\" \"pi/2\" 0.0625 \"y*y+x*x+y-SQRT(y*y+x*x)\" 2>errs.log", argv[0]);
        logger(INFO_LOG, "%s \"-pi\" \"1\" 0.125 \"-2\" \"2\" 0.0625 \"(ACOS(1-FABS(x))-pi)-y\" \"y-SQRT(1-(FABS(x)-1)^2)\" 2>errs.log", argv[0]);
        logger(INFO_LOG, "%s \"-1\" \"pi/2\" 0.125 \"-1\" \"1\" 0.0625 \"x*x+(y-FABS(x)^(2.0/3.0))^2-1\" 2>errs.log", argv[0]);

        exit(0);
    }
    INIT(argv + 1);
    int h = (int)ceil(s1);
    int w = (int)ceil(s2);
    // logger(ERR_LOG, "x = %d, y = %d", h, w);
    unsigned char *rgba = malloc(sizeof(unsigned char) * (w+LEFT_MARGIN+RIGHT_MARGIN+LEFT_EXTRA_PIXEL+RIGHT_EXTRA_PIXEL)*(h+TOP_MARGIN+END_MARGIN+TOP_EXTRA_PIXEL+END_EXTRA_PIXEL)*4);
    memset(rgba, 255, sizeof(unsigned char) * (w+LEFT_MARGIN+RIGHT_MARGIN+LEFT_EXTRA_PIXEL+RIGHT_EXTRA_PIXEL)*(h+TOP_MARGIN+END_MARGIN+TOP_EXTRA_PIXEL+END_EXTRA_PIXEL)*4);
    unsigned char *p = rgba;
    p += 4*TOP_MARGIN*(w+LEFT_MARGIN+RIGHT_MARGIN+LEFT_EXTRA_PIXEL+RIGHT_EXTRA_PIXEL);
    for(int i = 0; i < h+TOP_EXTRA_PIXEL+END_EXTRA_PIXEL; i++) {
        p+=4*LEFT_MARGIN;
        for(int j = 0; j < w+LEFT_EXTRA_PIXEL+RIGHT_EXTRA_PIXEL; j++) {
            logger(DEBUG_LOG, "x = %lf, y = %lf", j, i);
            double val = DBL_MAX;
            for(char **expr = argv + 7; *expr; expr++) {
                val = fmin(val, fabs(eval((_y1-_y2)/h*(i-TOP_EXTRA_PIXEL)+_y2, (x2-x1)/w*(j-LEFT_EXTRA_PIXEL)+x1, *expr)));
            }
//            if(val < ACCURACY) {
//                *(p++) = (unsigned char) round(R * 0);
//                *(p++) = (unsigned char) round(G * 0);
//                *(p++) = (unsigned char) round(B * 0);
//                *(p++) = (unsigned char) round(A * 1);
//            } else {
//                p+=4;
//            }
            long cur = (p - rgba);
            long line = (w+LEFT_EXTRA_PIXEL+RIGHT_EXTRA_PIXEL+LEFT_EXTRA_PIXEL+RIGHT_EXTRA_PIXEL);
            long line_cnt = (h+TOP_MARGIN+END_MARGIN+TOP_EXTRA_PIXEL+END_EXTRA_PIXEL);
            int k = Y_PAINTERSIZE;
            if(val < ACCURACY)
                while(k--) {
                    unsigned char *move = p + max(min(+X_PAINTERSIZE * 2 + line * (2*k-Y_PAINTERSIZE) * 2, 4*line_cnt*line - cur), 0);
                    unsigned char  *end = p + max(min(-X_PAINTERSIZE * 2 + line * (2*k-Y_PAINTERSIZE) * 2, 4*line_cnt*line - cur), 0);
                    while(move != end) {
                        *(--move) = (unsigned char)round(A*1);
                        *(--move) = (unsigned char)round(B*0);
                        *(--move) = (unsigned char)round(G*0);
                        *(--move) = (unsigned char)round(R*0);
                    }
                }
            p += 4;
        }
        p+=4*RIGHT_MARGIN;
    }
    p += 4*END_MARGIN*(w+LEFT_MARGIN+RIGHT_MARGIN+LEFT_EXTRA_PIXEL+RIGHT_EXTRA_PIXEL);

    svpng(stdout, w+LEFT_MARGIN+RIGHT_MARGIN+LEFT_EXTRA_PIXEL+RIGHT_EXTRA_PIXEL, h+TOP_MARGIN+END_MARGIN+TOP_EXTRA_PIXEL+END_EXTRA_PIXEL, rgba, 1);
    free(rgba);
    return 0;
}
#else
int main(int argc, char **argv) {
    logger(INFO_LOG, "Char Usage: %s y1 y2 sy x1 x2 sy expression", argv[0]);
    if(argc < 8) {
        logger(INFO_LOG, "Usage: %s y1 y2 sy x1 x2 sy expression", argv[0]);
        logger(INFO_LOG, "%s -1 1 0.125 -1 1 0.0625 \"x*x+y*y-1\" 2>errs.log", argv[0]);
        logger(INFO_LOG, "%s \"-pi/2\" \"pi/2\" 0.25 \"-3*pi\" \"2*pi\" 0.125 \"y^2-SIN(x+y)^2\" 2>errs.log", argv[0]);
        logger(INFO_LOG, "%s \"-pi/2\" \"pi/2\" 0.25 \"-3*pi\" \"2*pi\" 0.125 \"y^2-SIN(x)^2\" 2>errs.log", argv[0]);
        logger(INFO_LOG, "%s \"-2\" \"ACOS(1/2)-pi/4\" 0.125 \"-pi/2\" \"pi/2\" 0.0625 \"y*y+x*x+y-SQRT(y*y+x*x)\" 2>errs.log", argv[0]);
        logger(INFO_LOG, "%s \"-pi\" \"1\" 0.125 \"-2\" \"2\" 0.0625 \"(ACOS(1-FABS(x))-pi)-y\" \"y-SQRT(1-(FABS(x)-1)^2)\" 2>errs.log", argv[0]);
        logger(INFO_LOG, "%s \"-1\" \"pi/2\" 0.125 \"-1\" \"1\" 0.0625 \"x*x+(y-FABS(x)^(2.0/3.0))^2-1\" 2>errs.log", argv[0]);
        exit(0);
    }
    INIT(argv + 1);
    for(double i = _y2; i >= _y1; ) {
        for(double j = x1; j <= x2; ) {
            logger(DEBUG_LOG, "x = %lf, y = %lf", j, i);
            bool ok = true;
            for(char **expr = argv + 7; *expr; expr++) {
                double val = eval(i, j, *expr);
                if(val >= 0) {
                    ok = false;
                    break;
                }
            }
            if(ok) {
                printf("%c", SMALLER_CHAR);
            } else {
                printf("%c", GREATER_CHAR);
            }
            j+=s2;
        }
        printf("\n");
        i-=s1;
    }
    return 0;
}
#endif