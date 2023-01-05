#include <stdlib.h>
#include "lvgl.h"
#include "model/model.h"
#include "view/view.h"
#include "view/view_types.h"
#include "view/theme/style.h"
#include "gel/pagemanager/page_manager.h"


enum {
    BACK_BTN_ID,
    SLIDER_ID,
};


struct page_data {
    lv_obj_t *slider_min_speed;
};


static void update_page(model_t *pmodel, struct page_data *pdata);
static void slider_event_cb(lv_event_t *e);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = lv_mem_alloc(sizeof(struct page_data));
    assert(pdata != NULL);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;

    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);


    lv_obj_t *slider = lv_slider_create(cont);
    lv_obj_set_size(slider, 360, 32);
    lv_obj_align(slider, LV_ALIGN_CENTER, 0, 60);
    lv_slider_set_range(slider, 0, 50 / 5);
    lv_obj_add_style(slider, (lv_style_t *)&style_arc, LV_STATE_DEFAULT);
    lv_obj_add_style(slider, (lv_style_t *)&style_arc_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(slider, (lv_style_t *)&style_arc_knob, LV_PART_KNOB);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_refresh_ext_draw_size(slider);
    view_register_object_default_callback(slider, SLIDER_ID);
    pdata->slider_min_speed = slider;


    lv_obj_t *btn = lv_btn_create(cont);
    lv_obj_t *lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_48, LV_STATE_DEFAULT);
    lv_label_set_text(lbl, LV_SYMBOL_CLOSE);
    lv_obj_center(lbl);
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, 0, 0);
    view_register_object_default_callback(btn, BACK_BTN_ID);

    update_page(pmodel, pdata);
}


static view_message_t page_event(model_t *pmodel, void *args, view_event_t event) {
    view_message_t    msg   = VIEW_NULL_MESSAGE;
    struct page_data *pdata = args;

    switch (event.code) {
        case VIEW_EVENT_CODE_TIMER:
            break;

        case VIEW_EVENT_CODE_LVGL: {
            switch (event.event) {
                case LV_EVENT_CLICKED: {
                    switch (event.data.id) {
                        case BACK_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
                            break;
                    }
                    break;
                }

                case LV_EVENT_VALUE_CHANGED: {
                    switch (event.data.id) {
                        case SLIDER_ID:
                            model_set_minimum_speed(pmodel, lv_slider_get_value(pdata->slider_min_speed) * 5);
                            update_page(pmodel, pdata);
                            break;
                    }
                    break;
                }

                default:
                    break;
            }
            break;
        }

        default:
            break;
    }

    return msg;
}


static void close_page(void *args) {
    struct page_data *pdata = args;
    lv_obj_clean(lv_scr_act());
}


static void destroy_page(void *args, void *extra) {
    (void)extra;
    struct page_data *pdata = args;
    lv_mem_free(pdata);
}


static void update_page(model_t *pmodel, struct page_data *pdata) {
    lv_slider_set_value(pdata->slider_min_speed, model_get_minimum_speed(pmodel) / 5, LV_ANIM_OFF);
}


static void slider_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t       *obj  = lv_event_get_target(e);

    /*Provide some extra space for the value*/
    if (code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
        lv_event_set_ext_draw_size(e, 50);
    } else if (code == LV_EVENT_DRAW_PART_END) {
        lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);
        if (dsc->part == LV_PART_INDICATOR) {
            char buf[16];
            lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(obj) * 5);

            lv_point_t label_size;
            lv_txt_get_size(&label_size, buf, LV_FONT_DEFAULT, 0, 0, LV_COORD_MAX, 0);
            lv_area_t label_area;
            label_area.x1 = lv_area_get_width(dsc->draw_area) + label_size.x / 2;
            label_area.x2 = label_area.x1 + label_size.x;
            label_area.y2 = dsc->draw_area->y1 - 20;
            label_area.y1 = label_area.y2 - label_size.y;

            lv_draw_label_dsc_t label_draw_dsc;
            lv_draw_label_dsc_init(&label_draw_dsc);
            label_draw_dsc.color = lv_color_hex3(0xfff);
            lv_draw_label(dsc->draw_ctx, &label_draw_dsc, &label_area, buf, NULL);
        }
    }
}


const pman_page_t page_minimum_speed = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};