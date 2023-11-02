#include <stdio.h>
#include <cplot.h>

void init_cplot() {
    char y1[] = "-1";
    char y2[] = "1";
    char sy[] = "300";
    char x1[] = "-1";
    char x2[] = "1";
    char sx[] = "300";
    char t1[] = "-pi/4";
    char t2[] = "7*pi/4";
    char st[] = "300";
    char* args[] = { y1, y2, sy, x1, x2, sx, t1, t2, st, NULL };
    init_parametric_cplot(args);
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
    FILE* output = fopen("parametric.png", "wb");
    init_cplot();
    // init_cplot_by_args(); // or use command line args
    set_cplot_attrs(output);
    char expr[] = "SQRT(COS(2*t))*SIN(t),SQRT(COS(2*t))*COS(t)";
    char* plot_args[] = { expr, NULL };
    plot_parametric(plot_args);
    fclose(output);
    return 0;
}