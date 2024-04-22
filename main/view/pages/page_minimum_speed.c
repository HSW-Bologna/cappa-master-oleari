#include <stdlib.h>
#include "lvgl.h"
#include "model/model.h"
#include "view/view.h"
#include "view/common.h"
#include "view/view_types.h"
#include "view/theme/style.h"
#include "view/intl/intl.h"
#include "gel/pagemanager/page_manager.h"
#include "config/app_config.h"


LV_IMG_DECLARE(img_ventola);
LV_IMG_DECLARE(img_luce);
LV_IMG_DECLARE(img_luce_sm);
LV_IMG_DECLARE(img_settings);
LV_IMG_DECLARE(img_connection);


enum {
    FAN_BTN_ID,
    FAN_SPEED_ARC_ID,
    FAN_ENABLE_BTN_ID,
    BACK_BTN_ID,
};


struct page_data {
    lv_obj_t *btn_fans[MAX_FANS];
    lv_obj_t *btn_enable_fan;
    lv_obj_t *arc_speed;
    lv_obj_t *img_communication;
    lv_anim_t anim_fans[MAX_FANS];

    uint8_t anim_state[MAX_FANS];

    size_t fan_index;
};


static lv_obj_t *fan_button_create(lv_obj_t *root, const char *text);
static void      update_page(model_t *pmodel, struct page_data *pdata);
static lv_anim_t fan_animation(lv_obj_t *img, uint32_t period);
static lv_obj_t *fan_enable_button_create(lv_obj_t *root);
static uint16_t  anim_period_from_speed(uint16_t speed);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));

    pdata->anim_state[0] = 0;
    pdata->anim_state[1] = 0;
    pdata->anim_state[2] = 0;
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;

    uint16_t main_area_width = LV_HOR_RES_MAX;

    if (pmodel->configuration.num_fans == 1) {
        main_area_width  = LV_HOR_RES_MAX;
        pdata->fan_index = 0;

        for (size_t i = 0; i < MAX_FANS; i++) {
            pdata->btn_fans[i] = NULL;
            lv_anim_init(&pdata->anim_fans[i]);
        }
    } else {
        lv_obj_t *left_tab = lv_obj_create(lv_scr_act());
        lv_obj_set_size(left_tab, 103, LV_PCT(100));
        lv_obj_add_style(left_tab, (lv_style_t *)&style_transparent_cont, LV_STATE_DEFAULT);
        lv_obj_set_flex_flow(left_tab, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(left_tab, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(left_tab, 0, LV_STATE_DEFAULT);
        lv_obj_set_style_pad_gap(left_tab, 0, LV_STATE_DEFAULT);

        main_area_width = LV_HOR_RES_MAX - 100;
        if (pdata->fan_index >= pmodel->configuration.num_fans) {
            pdata->fan_index = 0;
        }

        pdata->btn_fans[0] = fan_button_create(left_tab, model_get_fan_name(pmodel, 0));
        lv_obj_align(pdata->btn_fans[0], LV_ALIGN_BOTTOM_LEFT, 2, -2);
        view_register_object_default_callback_with_number(pdata->btn_fans[0], FAN_BTN_ID, 0);

        if (pmodel->configuration.num_fans > 1) {
            pdata->btn_fans[1] = fan_button_create(left_tab, model_get_fan_name(pmodel, 1));
            lv_obj_align(pdata->btn_fans[1], LV_ALIGN_LEFT_MID, 2, 0);
            view_register_object_default_callback_with_number(pdata->btn_fans[1], FAN_BTN_ID, 1);
        } else {
            pdata->btn_fans[1] = NULL;
        }

        if (pmodel->configuration.num_fans > 2) {
            pdata->btn_fans[2] = fan_button_create(left_tab, model_get_fan_name(pmodel, 2));
            lv_obj_align(pdata->btn_fans[2], LV_ALIGN_TOP_LEFT, 2, 2);
            view_register_object_default_callback_with_number(pdata->btn_fans[2], FAN_BTN_ID, 2);
        } else {
            pdata->btn_fans[2] = NULL;
        }

        for (size_t i = 0; i < MAX_FANS; i++) {
            if (i >= pmodel->configuration.num_fans) {
                lv_anim_init(&pdata->anim_fans[i]);
            } else {
                pdata->anim_fans[i] = fan_animation(lv_obj_get_child(pdata->btn_fans[i], 0), 1000);

                if (model_get_fan_on(pmodel, i) && model_get_fan_speed(pmodel, i) > 0) {
                    uint16_t period = anim_period_from_speed(model_get_fan_speed(pmodel, pdata->fan_index));
                    lv_anim_set_time(&pdata->anim_fans[i], period);
                    lv_anim_start(&pdata->anim_fans[i]);
                }
            }
        }
    }

    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, main_area_width, LV_PCT(100));
    lv_obj_align(cont, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_pad_all(cont, 4, LV_STATE_DEFAULT);

    if (pmodel->configuration.num_fans == 1) {
        lv_obj_t *img = lv_img_create(cont);
        lv_img_set_src(img, &img_ventola);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 8, 8);

        pdata->anim_fans[0] = fan_animation(img, 1000);
    }


    lv_obj_t *arc = lv_arc_create(cont);
    lv_obj_set_size(arc, 272, 272);
    lv_arc_set_rotation(arc, 0);
    lv_arc_set_bg_angles(arc, 0, 270);
    lv_obj_align(arc, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(arc, (lv_style_t *)&style_arc, LV_STATE_DEFAULT);
    lv_obj_add_style(arc, (lv_style_t *)&style_arc_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(arc, (lv_style_t *)&style_arc_knob, LV_PART_KNOB);
    lv_obj_set_style_arc_color(arc, lv_color_black(), LV_PART_MAIN);
    view_register_object_default_callback(arc, FAN_SPEED_ARC_ID);
    lv_obj_set_style_opa(arc, LV_OPA_60, LV_PART_KNOB | LV_STATE_DISABLED);
    lv_obj_set_style_opa(arc, LV_OPA_60, LV_PART_INDICATOR | LV_STATE_DISABLED);
    pdata->arc_speed = arc;

    pdata->btn_enable_fan = fan_enable_button_create(cont);
    lv_obj_align_to(pdata->btn_enable_fan, arc, LV_ALIGN_CENTER, 0, 0);
    view_register_object_default_callback(pdata->btn_enable_fan, FAN_ENABLE_BTN_ID);

    lv_obj_t *img = lv_img_create(cont);
    lv_img_set_src(img, &img_connection);
    lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    pdata->img_communication = img;


    lv_obj_t *btn_back = view_common_back_btn_create(lv_scr_act());
    view_register_object_default_callback(btn_back, BACK_BTN_ID);
    lv_obj_align(btn_back, LV_ALIGN_TOP_RIGHT, -8, 8);

    model_set_fan_speed(pmodel, pdata->fan_index, model_get_minimum_speed(pmodel, pdata->fan_index));

    update_page(pmodel, pdata);
}


static view_message_t page_event(model_t *pmodel, void *args, view_event_t event) {
    view_message_t    msg   = VIEW_NULL_MESSAGE;
    struct page_data *pdata = args;

    switch (event.code) {
        case VIEW_EVENT_CODE_UPDATE:
            update_page(pmodel, pdata);
            break;

        case VIEW_EVENT_CODE_TIMER: {
            if (model_get_fan_on(pmodel, 0) || model_get_fan_on(pmodel, 1) || model_get_fan_on(pmodel, 2)) {
                // Do nothing
            } else {
                msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                msg.vmsg.extra = (void *)(uintptr_t)1;
                msg.vmsg.page  = (void *)&page_splash;
            }
            break;
        }

        case VIEW_EVENT_CODE_LVGL: {
            switch (event.event) {
                case LV_EVENT_VALUE_CHANGED: {
                    switch (event.data.id) {
                        case FAN_BTN_ID:
                            model_turn_fan_off(pmodel, pdata->fan_index);
                            pdata->fan_index = event.data.number;
                            model_set_fan_speed(pmodel, pdata->fan_index,
                                                model_get_minimum_speed(pmodel, pdata->fan_index));
                            update_page(pmodel, pdata);
                            break;

                        case FAN_ENABLE_BTN_ID:
                            model_toggle_fan_on(pmodel, pdata->fan_index);
                            update_page(pmodel, pdata);

                            if (model_get_fan_on(pmodel, pdata->fan_index) &&
                                model_get_fan_speed(pmodel, pdata->fan_index) > 0) {
                                uint16_t period = anim_period_from_speed(model_get_fan_speed(pmodel, pdata->fan_index));
                                lv_anim_set_time(&pdata->anim_fans[pdata->fan_index], period);
                                if (pdata->anim_state[pdata->fan_index] == 0) {
                                    lv_anim_start(&pdata->anim_fans[pdata->fan_index]);
                                    pdata->anim_state[pdata->fan_index] = 1;
                                }
                            } else {
                                lv_anim_custom_del(&pdata->anim_fans[pdata->fan_index], NULL);
                                pdata->anim_state[pdata->fan_index] = 0;
                            }

                            break;

                        case FAN_SPEED_ARC_ID: {
                            uint16_t speed = APP_CONFIG_MIN_MINIMUM_SPEED + lv_arc_get_value(pdata->arc_speed);
                            if (speed > APP_CONFIG_MAX_MINIMUM_SPEED) {
                                speed = APP_CONFIG_MAX_MINIMUM_SPEED;
                            }

                            model_set_fan_speed(pmodel, pdata->fan_index, speed);
                            model_set_minimum_speed(pmodel, pdata->fan_index, speed);
                            update_page(pmodel, pdata);

                            if (model_get_fan_on(pmodel, pdata->fan_index) &&
                                model_get_fan_speed(pmodel, pdata->fan_index) > 0) {
                                uint16_t period = anim_period_from_speed(model_get_fan_speed(pmodel, pdata->fan_index));
                                lv_anim_set_time(&pdata->anim_fans[pdata->fan_index], period);
                                if (pdata->anim_state[pdata->fan_index] == 0) {
                                    lv_anim_start(&pdata->anim_fans[pdata->fan_index]);
                                    pdata->anim_state[pdata->fan_index] = 1;
                                }
                            } else {
                                lv_anim_custom_del(&pdata->anim_fans[pdata->fan_index], NULL);
                                pdata->anim_state[pdata->fan_index] = 0;
                            }
                            break;
                        }
                    }
                    break;
                }

                case LV_EVENT_RELEASED: {
                    switch (event.data.id) {
                        case FAN_SPEED_ARC_ID:
                            if (pdata->anim_state[pdata->fan_index] == 1) {
                                lv_anim_start(&pdata->anim_fans[pdata->fan_index]);
                            }
                            break;
                    }
                    break;
                }

                case LV_EVENT_CLICKED: {
                    switch (event.data.id) {
                        case BACK_BTN_ID:
                            model_turn_fan_off(pmodel, pdata->fan_index);
                            model_check_config(pmodel);
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
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


static void update_page(model_t *pmodel, struct page_data *pdata) {
    view_common_set_hidden(pdata->img_communication, !model_get_communication_error(pmodel));

    for (size_t i = 0; i < MAX_FANS; i++) {
        if (pdata->btn_fans[i] != NULL) {
            view_common_set_checked(pdata->btn_fans[i], i == pdata->fan_index);
            view_common_set_hidden(lv_obj_get_child(pdata->btn_fans[i], 1), !model_get_light_on(pmodel, i));
        }
    }

    uint16_t minimum_speed = APP_CONFIG_MIN_MINIMUM_SPEED;
    lv_arc_set_range(pdata->arc_speed, 0, (APP_CONFIG_MAX_MINIMUM_SPEED - minimum_speed));
    lv_arc_set_value(pdata->arc_speed, (model_get_fan_speed(pmodel, pdata->fan_index) - minimum_speed));

    view_common_set_checked(pdata->btn_enable_fan, model_get_fan_on(pmodel, pdata->fan_index));
    lv_label_set_text(lv_obj_get_child(pdata->btn_enable_fan, 0),
                      model_get_fan_on(pmodel, pdata->fan_index) ? "ON" : "OFF");
}


static lv_obj_t *fan_enable_button_create(lv_obj_t *root) {
    lv_obj_t *btn = lv_btn_create(root);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_size(btn, 80, 64);
    lv_obj_set_style_bg_color(btn, lv_color_make(0, 255, 0), LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(btn, lv_color_make(255, 0, 0), LV_STATE_DEFAULT);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_obj_center(lbl);

    return btn;
}


static lv_obj_t *fan_button_create(lv_obj_t *root, const char *text) {
    lv_obj_t *btn = lv_btn_create(root);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_size(btn, 96, 103);

    lv_obj_t *img = lv_img_create(btn);
    lv_img_set_src(img, &img_ventola);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, -8);

    img = lv_img_create(btn);
    lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor(img, lv_color_make(0xa1, 0xa1, 0x1d), LV_STATE_DEFAULT);
    lv_img_set_src(img, &img_luce_sm);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_16, LV_STATE_DEFAULT);
    lv_label_set_text(lbl, text);
    lv_obj_align(lbl, LV_ALIGN_BOTTOM_MID, 0, 4);

    return btn;
}


static lv_anim_t fan_animation(lv_obj_t *img, uint32_t period) {
    lv_anim_t a;
    lv_anim_init(&a);
    /*Set the "animator" function*/
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)(void *)lv_img_set_angle);
    /*Set target of the animation*/
    lv_anim_set_var(&a, img);
    /*Length of the animation [ms]*/
    lv_anim_set_time(&a, period);
    /*Set start and end values. E.g. 0, 150*/
    lv_anim_set_values(&a, 0, 3600);
    /* OPTIONAL SETTINGS
     *------------------*/
    /*Time to wait before starting the animation [ms]*/
    lv_anim_set_delay(&a, 0);
    /*Number of repetitions. Default is 1. LV_ANIM_REPEAT_INFINITE for infinite repetition*/
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    /* START THE ANIMATION
     *------------------*/
    return a;
}


static uint16_t anim_period_from_speed(uint16_t speed) {
    const uint16_t periods[] = {1600, 1500, 1300, 1100, 1000, 950, 900, 850, 800, 750, 700,
                                660,  620,  580,  540,  490,  450, 410, 370, 330, 300};
    if (speed > 0) {
        return periods[speed / 5];
    } else {
        return 10000;
    }
}


static void close_page(void *args) {
    struct page_data *pdata = args;
    (void)pdata;
    lv_obj_clean(lv_scr_act());
}


static void destroy_page(void *args, void *extra) {
    (void)extra;
    struct page_data *pdata = args;
    lv_mem_free(pdata);
}


const pman_page_t page_minimum_speed = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};
