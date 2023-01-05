#ifndef STYLE_H_INCLUDED
#define STYLE_H_INCLUDED

#include "lvgl.h"


#define STYLE_RED ((lv_color_t)LV_COLOR_MAKE(0xFF, 0, 0))
#define STYLE_BLACK ((lv_color_t)LV_COLOR_MAKE(0, 0, 0))


void style_init(void);


extern const lv_style_t style_transparent_cont;
extern const lv_style_t style_arc;
extern const lv_style_t style_arc_indicator;
extern const lv_style_t style_arc_knob;
extern const lv_style_t style_toast;


#endif