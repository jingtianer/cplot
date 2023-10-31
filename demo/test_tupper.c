

#include <stdio.h>
#include <cplot.h>
// #include <setters.h>
#define PI 3.1415926

void init_cplot() {
    number_t k = eval_value(0, 0, "960939379918958884971672962127852754715004339660129306651505519271702802395266424689642842174350718121267153782770623355993237280874144307891325963941337723487857735749823926629715517173716995165232890538221612403238855866184013235585136048828693337902491454229288667081096184496091705183454067827731551705405381627380967602565625016981482083418783163849115590225610003652351370343874461848378737238198224849863465033159410054974700593138339226497249461751545728366702369745461014655997933798537483143786841806593422227898388722980000748404719"
    );
    // use init_* api
    init__y1(k);
    init__y2(k+17);
    init_x1(0);
    init_x2(107);
    init_deltaX(107);
    init_deltaY(17);
    init_sy(3000);
    init_sx(3000);
}

void set_cplot_attrs(FILE *output) {
    set_brush_size(2); // px
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
    set_max_try(1000); // max try, increase it to reduce the number of nonsequence posistion， decrease to gain better performance
}

int main(int argc, char** argv) {
    FILE *output = fopen("tupper.png", "wb");
    init_cplot();
    // init_cplot_by_args(); // or use command line args
    set_cplot_attrs(output);
    char expr[] = "1/2<FLOOR((FLOOR(y/17)*2^(-17*FLOOR(X)-FLOOR(y)%17))%2)";
    char* plot_args[] = { expr, NULL};
    plot_png(plot_args);
    fclose(output);
    return 0;
}