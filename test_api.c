
#include <stdio.h>
#include <cplot.h>
#define PI 3.1415926

void init_cplot() {
    init__y1(-1);
    init__y2(1);
    init_x1(-PI);
    init_x2(PI);
    init_deltaX(2*PI);
    init_deltaY(2);
    init_s1(300);
    init_s2(300);
}

void set_cplot_attrs(FILE *output) {
    set_brush_size(1);
    set_brush_color(0xffff00e0);
    set_bg_color(0x000000ff);
    enable_fastmode(false);
    set_margin(15);
    set_padding(10);
    set_output_file(output);
}

int main(int argc, char** argv) {
    FILE *output = fopen("api.png", "wb");
    init_cplot();
    set_cplot_attrs(output);
    char expr0[] = "y=SIN(x)";
    char expr1[] = "y=COS(x)";
    char* plot_args[] = { expr0, expr1, NULL };
    plot_png(plot_args);
    fclose(output);
    return 0;
}