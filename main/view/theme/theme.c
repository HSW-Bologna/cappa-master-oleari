#include "theme.h"
#include "style.h"


static void theme_apply_cb(lv_theme_t *th, lv_obj_t *obj);


void theme_init(lv_disp_t *disp) {
    // lv_theme_t *th = lv_theme_basic_init(disp);
    lv_theme_t *th = lv_theme_default_init(disp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), 1,
                                           &lv_font_montserrat_48);

    /*Initialize the new theme from the current theme*/
    static lv_theme_t th_new;
    th_new = *th;

    /*Set the parent theme and the style apply callback for the new theme*/
    lv_theme_set_parent(&th_new, th);
    lv_theme_set_apply_cb(&th_new, theme_apply_cb);

    /*Assign the new theme the the current display*/
    lv_disp_set_theme(disp, &th_new);
}


static void theme_apply_cb(lv_theme_t *th, lv_obj_t *obj) {}