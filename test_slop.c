#define _XOPEN_SOURCE 500
#include <math.h>
#include <stdio.h>
#include <cplot.h>
// #include <setters.h>

void init_cplot(int i) {
    // use init_* api
    init__y1(i);
    init__y2(1+i);
    init_x1(0);
    init_x2(M_PI_2);
    init_deltaX(M_PI_2);
    init_deltaY(1);
    init_sy(300);
    init_sx(300);
}

void set_cplot_attrs(FILE *output) {
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
    // set_logger_log_level(INFO_LOG);
    set_x_axis(true, 0.1, 0xffffffff, 0.5); // enable x-axis, len of scale is 0.1, color is 0xffffffff, interval of scale is 0.5
    set_y_axis(true, 0.1, 0xffffffff, 1); // enable y-axis, len of scale is 0.1, color is 0xffffffff, interval of scale is 1
}

int main(int argc, char** argv) {
    // init_cplot_by_args(); // or use command line args
    char fname[1280] = {0};
    for(int i = 0; i < 100; i++) {
        int n = snprintf(fname, 1280, "slop_%d.png", i);
        FILE *output = fopen(fname, "wb");
        set_output_file(output);
        init_cplot(i);
        char expr0[] = "y=TAN(x)";
        char* plot_args[] = { expr0, NULL};
        plot_png(plot_args);
        fclose(output);
    }
    return 0;
}