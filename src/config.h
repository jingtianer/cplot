#ifndef _CONFIG_H_
#define _CONFIG_H_

// ***examples***
// #define EXEC_EXAMPLES // execute examples
// #define OPEN_EXAMPLES // open the examples



// ***display mode***
#define USE_PNG
// #define USE_CONSOLE // uncomment it if you want draw on console


// ***configs for console***

// *****console display char*****
#define OUTER_CHAR ' '// char to be printed when a dot is outside of the region
#define INNER_CHAR '+' // char to be printed when a dot is inside of the region


// ***configs for png***

// *****colors*****
// #define BG_COLOR 0xffffffff // background color, rgba
// #define BRUSH_COLOR 0x000000ff // brush color, rgba

// *****preset color schemes*****
#define BLUE_PINK // preset color scheme blue_pink
// #define BLACK_WHITE // preset color scheme blue_pink

// *****brush size (px)*****
#define BRUSH_SIZE 1

// *****margins*****
#define MARGIN 10 // margin of png
#ifndef MARGIN // or define margins of 4 direction
#define LEFT_MARGIN     10
#define RIGHT_MARGIN    10
#define TOP_MARGIN      10
#define END_MARGIN      10
#endif

// *****paddings*****
#define PADDING 10 // padding of png
#ifndef PADDING // or define paddings of 4 direction
#define LEFT_PADDING    10
#define RIGHT_PADDING   10
#define TOP_PADDING     10
#define END_PADDING     10
#endif

// *****axis*****
// #define ENABLE_Y_AXIS
#define Y_SCALE_LENGTH 0.1
#define Y_SCALE_COLOR 0x000000ff
#define Y_SCALE_INTERVAL 1

// #define ENABLE_X_AXIS
#define X_SCALE_LENGTH 0.1
#define X_SCALE_COLOR 0x000000ff
#define X_SCALE_INTERVAL 0.5

// *****output file*****
// #define OUTPUT_FILE "./out.png"


// ***other configs***

// *****debug levels*****
/*
DEBUG_LOG 
INFO_LOG 
ERR_LOG 
*/
#ifndef LOG_LEVEL
#define LOG_LEVEL INFO_LOG // set log level
#endif

// *****fast-mode*****

// #define FAST_MODE

// *****max-try*****
#define MAX_TRY 80 // max try, increase it to reduce the number of nonsequence posistion， decrease to gain better performance

// ***default values***
// don't edit code below

#if defined(USE_CONSOLE) + defined(USE_PNG) != 1
#error "you must define exactly one display mode."
#endif

#if defined BLUE_PINK || defined BLACK_WHITE
#if defined(BLUE_PINK) + defined(BLACK_WHITE) > 1
#error "you can only select one color scheme"
#endif
#ifdef BG_COLOR
#error "can not use BG_COLOR and preset color scheme at the same time."
#endif
#ifdef BRUSH_COLOR
#error "can not use BRUSH_COLOR and preset color scheme at the same time."
#endif
#endif

#if defined BLUE_PINK
#define BG_COLOR 0xFFEAE1FF // background color, rgba
#define BRUSH_COLOR 0x00B2EEFF // brush color, rgba
#elif defined BLACK_WHITE
#define BG_COLOR 0xFFFFFFFF // background color, rgba
#define BRUSH_COLOR 0x000000FF // brush color, rgba
#endif


#define RSH(b, n) ((unsigned char)(((b) >> ((n) << 2))&0x000000FF))
#define GET_R(b) RSH(b, 6)
#define GET_G(b) RSH(b, 4)
#define GET_B(b) RSH(b, 2)
#define GET_A(b) RSH(b, 0)
#define R GET_R(BRUSH_COLOR)
#define G GET_G(BRUSH_COLOR)
#define B GET_B(BRUSH_COLOR)
#define A GET_A(BRUSH_COLOR)
#define BG_R GET_R(BG_COLOR)
#define BG_G GET_G(BG_COLOR)
#define BG_B GET_B(BG_COLOR)
#define BG_A GET_A(BG_COLOR)

// #undef LSH
// #undef GET_R
// #undef GET_G
// #undef GET_B
// #undef GET_A

#ifndef LOG_LEVEL
#define LOG_LEVEL INFO_LOG
#endif

#ifdef USE_CONSOLE
#ifndef OUTER_CHAR
#define OUTER_CHAR ' '
#endif
#ifndef INNER_CHAR
#define INNER_CHAR '+'
#endif
#endif

#ifdef PADDING
#ifndef LEFT_PADDING
#define LEFT_PADDING    PADDING
#endif
#ifndef RIGHT_PADDING
#define RIGHT_PADDING   PADDING
#endif
#ifndef TOP_PADDING
#define TOP_PADDING     PADDING
#endif
#ifndef END_PADDING
#define END_PADDING     PADDING
#endif
#else
#ifdef LEFT_PADDING
#define LEFT_PADDING 10
#endif
#ifdef RIGHT_PADDING
#define RIGHT_PADDING 10
#endif
#ifdef TOP_PADDING
#define TOP_PADDING 10
#endif
#ifdef END_MARGIN
#define END_PADDING 10
#endif
#endif

#ifdef MARGIN
#define LEFT_MARGIN     MARGIN
#define RIGHT_MARGIN    MARGIN
#define TOP_MARGIN      MARGIN
#define END_MARGIN      MARGIN
#else
#ifdef LEFT_MARGIN
#define LEFT_MARGIN 10
#endif
#ifdef RIGHT_MARGIN
#define RIGHT_MARGIN 10
#endif
#ifdef TOP_MARGIN
#define TOP_MARGIN 10
#endif
#ifdef END_MARGIN
#define END_MARGIN 10
#endif
#endif
#if defined OPEN_EXAMPLES && !defined EXEC_EXAMPLES
#error "#define EXEC_EXAMPLES please"
#endif

#if defined(Y_SCALE_COLOR) + defined(Y_SCALE_LENGTH) + defined(Y_SCALE_INTERVAL) != 3 && defined(ENABLE_Y_AXIS)
error "you should define Y_SCALE_COLOR and Y_SCALE_LENGTH if you ENABLE_Y_AXIS"
#endif

#if defined(X_SCALE_COLOR) + defined(X_SCALE_LENGTH) + defined(X_SCALE_INTERVAL) != 3 && defined(ENABLE_X_AXIS)
error "you should define X_SCALE_COLOR and X_SCALE_LENGTH if you ENABLE_X_AXIS"
#endif
#endif