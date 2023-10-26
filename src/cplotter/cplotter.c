#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "../logger/logger.h"
#include "../cplot/include/cplot.h"
#include "../config.h"

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
    set_bg_color(BG_COLOR);
    set_brush_size(BRUSH_SIZE);
    set_brush_color(BRUSH_COLOR);
    set_LEFT_MARGIN(LEFT_MARGIN);
    set_LEFT_PADDING(LEFT_PADDING);
    set_RIGHT_MARGIN(RIGHT_MARGIN);
    set_RIGHT_PADDING(RIGHT_PADDING);
    set_TOP_MARGIN(TOP_MARGIN);
    set_TOP_PADDING(TOP_PADDING);
    set_END_MARGIN(END_MARGIN);
    set_END_PADDING(END_PADDING);
    init(argv + 1);
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