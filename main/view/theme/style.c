#include "lvgl.h"
#include "style.h"


static const lv_style_const_prop_t style_transparent_cont_props[] = {
    LV_STYLE_CONST_PAD_BOTTOM(0), LV_STYLE_CONST_PAD_TOP(0),      LV_STYLE_CONST_PAD_LEFT(0),
    LV_STYLE_CONST_PAD_RIGHT(0),  LV_STYLE_CONST_BORDER_WIDTH(0), LV_STYLE_CONST_BG_OPA(LV_OPA_TRANSP),
};
LV_STYLE_CONST_INIT(style_transparent_cont, style_transparent_cont_props);

static const lv_style_const_prop_t style_arc_props[] = {
    LV_STYLE_CONST_ARC_WIDTH(24),
};
LV_STYLE_CONST_INIT(style_arc, style_arc_props);

static const lv_style_const_prop_t style_arc_indicator_props[] = {
    LV_STYLE_CONST_ARC_WIDTH(24),
};
LV_STYLE_CONST_INIT(style_arc_indicator, style_arc_indicator_props);

static const lv_style_const_prop_t style_arc_knob_props[] = {
    LV_STYLE_CONST_PAD_BOTTOM(16), LV_STYLE_CONST_PAD_TOP(16),     LV_STYLE_CONST_PAD_LEFT(16),
    LV_STYLE_CONST_PAD_RIGHT(16),  LV_STYLE_CONST_BORDER_WIDTH(2),
};
LV_STYLE_CONST_INIT(style_arc_knob, style_arc_knob_props);

static const lv_style_const_prop_t style_toast_props[] = {
    LV_STYLE_CONST_PAD_BOTTOM(8),
    LV_STYLE_CONST_PAD_TOP(8),
    LV_STYLE_CONST_PAD_LEFT(8),
    LV_STYLE_CONST_PAD_RIGHT(8),
    LV_STYLE_CONST_BORDER_COLOR(STYLE_BLACK),
    LV_STYLE_CONST_BORDER_WIDTH(2),
};
LV_STYLE_CONST_INIT(style_toast, style_toast_props);


void style_init(void) {}