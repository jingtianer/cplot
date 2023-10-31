#include <stdio.h>
#include <cplot.h>
#define PI 3.1415926

void init_cplot() {
    // use init_* api
    init__y1(-5);
    init__y2(2);
    init_x1(-3.4);
    init_x2(2 * PI);
    init_deltaX(2 * PI + 3.4);
    init_deltaY(7);
    init_sy(300);
    init_sx(300);
    init_t1(0);
    init_t2(2 * PI);
    init_st(300);
}

void init_cplot_by_args() {
    // use command line args
    char y1[] = "-5";
    char y2[] = "2";
    char x1[] = "-3.4";
    char x2[] = "2*pi";
    char t1[] = "0";
    char t2[] = "2*pi";
    char sx[] = "300";
    char sy[] = "300";
    char st[] = "300";
    char* args[] = { y1, y2, sy, x1, x2, sx, t1, t2, st, NULL };
    init_polar_cplot(args);
}

void set_cplot_attrs(FILE* output) {
    set_brush_size(1); // px
    set_brush_color(0xffff00e0); // equivalent to 
    // set_R(0xff); set_G(0xff); set_B(0x00); set_A(0xe0);
    set_bg_color(0x000000ff); // equivalent to 
    // set_BG_R(0x00); set_BG_B(0x00); set_BG_B(0x00); set_BG_A(0xff);
    enable_fastmode(false);
    set_margin(15); // equivalent to 
    //set_LEFT_MARGIN(10); set_RIGHT_MARGIN(10); set_TOP_MARGIN(10); set_END_MARGIN(10);
    set_padding(10); // equivalent to 
    //set_LEFT_PADDING(10); set_RIGHT_PADDING(10); set_TOP_PADDING(10); set_END_PADDING(10);
    set_output_file(output);
    // set_logger_log_level(INFO_LOG);
    // set_x_axis(true, 0.1, 0xffffffff, 0.5); // enable x-axis, len of scale is 0.1, color is 0xffffffff, interval of scale is 0.5
    // set_y_axis(true, 0.1, 0xffffffff, 1); // enable y-axis, len of scale is 0.1, color is 0xffffffff, interval of scale is 1
}

int main(int argc, char** argv) {
    FILE* output = fopen("polar.png", "wb");
    init_cplot();
    // init_cplot_by_args(); // or use command line args
    set_cplot_attrs(output);
    char expr0[] = "t";
    char expr1[] = "1";
    char* plot_args[] = { expr0, expr1, NULL };
    plot_polar(plot_args);
    fclose(output);
    return 0;
}