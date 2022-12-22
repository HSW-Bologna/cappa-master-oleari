#include <stdlib.h>
#include "lvgl.h"
#include "model/model.h"
#include "view/view.h"
#include "view/common.h"
#include "view/view_types.h"
#include "view/theme/style.h"
#include "view/intl/intl.h"
#include "gel/pagemanager/page_manager.h"


LV_IMG_DECLARE(img_ventola);


enum {
    FAN_BTN_ID,
    FAN_SPEED_ARC_ID,
    FAN_ENABLE_BTN_ID,
};


struct page_data {
    lv_obj_t *btn_fans[NUM_FANS];
    lv_obj_t *btn_enable_fan;
    lv_obj_t *arc_speed;
    lv_anim_t anim_fans[NUM_FANS];

    size_t fan_index;
};


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



static lv_obj_t *fan_button_create(lv_obj_t *root);
static void      update_page(model_t *pmodel, struct page_data *pdata);
static lv_anim_t fan_animation(lv_obj_t *img, uint32_t period);
static lv_obj_t *fan_enable_button_create(lv_obj_t *root);
static uint16_t  anim_period_from_speed(uint16_t speed);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->fan_index        = 1;
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;

    pdata->btn_fans[0] = fan_button_create(lv_scr_act());
    lv_obj_align(pdata->btn_fans[0], LV_ALIGN_BOTTOM_LEFT, 2, -2);
    view_register_object_default_callback_with_number(pdata->btn_fans[0], FAN_BTN_ID, 0);

    pdata->btn_fans[1] = fan_button_create(lv_scr_act());
    lv_obj_align(pdata->btn_fans[1], LV_ALIGN_LEFT_MID, 2, 0);
    view_register_object_default_callback_with_number(pdata->btn_fans[1], FAN_BTN_ID, 1);

    pdata->btn_fans[2] = fan_button_create(lv_scr_act());
    lv_obj_align(pdata->btn_fans[2], LV_ALIGN_TOP_LEFT, 2, 2);
    view_register_object_default_callback_with_number(pdata->btn_fans[2], FAN_BTN_ID, 2);

    for (size_t i = 0; i < NUM_FANS; i++) {
        pdata->anim_fans[i] = fan_animation(lv_obj_get_child(pdata->btn_fans[i], 0), 1000);
    }

    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_HOR_RES_MAX - 100, LV_PCT(100));
    lv_obj_align(cont, LV_ALIGN_RIGHT_MID, 0, 0);

    lv_obj_t *arc = lv_arc_create(cont);
    lv_obj_set_size(arc, 250, 250);
    lv_arc_set_rotation(arc, 135);
    lv_arc_set_bg_angles(arc, 0, 270);
    lv_obj_align(arc, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(arc, (lv_style_t *)&style_arc, LV_STATE_DEFAULT);
    lv_obj_add_style(arc, (lv_style_t *)&style_arc_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(arc, (lv_style_t *)&style_arc_knob, LV_PART_KNOB);
    view_register_object_default_callback(arc, FAN_SPEED_ARC_ID);
    lv_obj_set_style_opa(arc, LV_OPA_60, LV_PART_KNOB | LV_STATE_DISABLED);
    lv_obj_set_style_opa(arc, LV_OPA_60, LV_PART_INDICATOR | LV_STATE_DISABLED);
    pdata->arc_speed = arc;

    pdata->btn_enable_fan = fan_enable_button_create(cont);
    lv_obj_align_to(pdata->btn_enable_fan, arc, LV_ALIGN_BOTTOM_MID, 0, 16);
    view_register_object_default_callback(pdata->btn_enable_fan, FAN_ENABLE_BTN_ID);

    update_page(pmodel, pdata);
}


static view_message_t page_event(model_t *pmodel, void *args, view_event_t event) {
    view_message_t    msg   = VIEW_NULL_MESSAGE;
    struct page_data *pdata = args;

    switch (event.code) {
        case VIEW_EVENT_CODE_LVGL: {
            switch (event.event) {
                case LV_EVENT_VALUE_CHANGED: {
                    switch (event.data.id) {
                        case FAN_BTN_ID:
                            pdata->fan_index = event.data.number;
                            update_page(pmodel, pdata);
                            break;

                        case FAN_ENABLE_BTN_ID:
                            pmodel->run.fan_on[pdata->fan_index] = !pmodel->run.fan_on[pdata->fan_index];
                            update_page(pmodel, pdata);

                            msg.cmsg.code = VIEW_CONTROLLER_MESSAGE_CODE_SET_FAN_SPEED;
                            msg.cmsg.fan  = pdata->fan_index;

                            if (pmodel->run.fan_on[pdata->fan_index] && pmodel->run.fan_speeds[pdata->fan_index] > 0) {
                                uint16_t period = anim_period_from_speed(pmodel->run.fan_speeds[pdata->fan_index]);
                                lv_anim_set_time(&pdata->anim_fans[pdata->fan_index], period);
                                lv_anim_start(&pdata->anim_fans[pdata->fan_index]);

                                msg.cmsg.speed = pmodel->run.fan_speeds[pdata->fan_index];
                            } else {
                                lv_anim_custom_del(&pdata->anim_fans[pdata->fan_index], NULL);

                                msg.cmsg.speed = 0;
                            }

                            break;

                        case FAN_SPEED_ARC_ID:
                            pmodel->run.fan_speeds[pdata->fan_index] = lv_arc_get_value(pdata->arc_speed);
                            update_page(pmodel, pdata);

                            msg.cmsg.code = VIEW_CONTROLLER_MESSAGE_CODE_SET_FAN_SPEED;
                            msg.cmsg.fan  = pdata->fan_index;

                            if (pmodel->run.fan_on[pdata->fan_index] && pmodel->run.fan_speeds[pdata->fan_index] > 0) {
                                uint16_t period = anim_period_from_speed(pmodel->run.fan_speeds[pdata->fan_index]);
                                lv_anim_set_time(&pdata->anim_fans[pdata->fan_index], period);
                                lv_anim_start(&pdata->anim_fans[pdata->fan_index]);

                                msg.cmsg.speed = pmodel->run.fan_speeds[pdata->fan_index];
                            } else {
                                lv_anim_custom_del(&pdata->anim_fans[pdata->fan_index], NULL);

                                msg.cmsg.speed = 0;
                            }
                            break;
                    }
                    break;
                }

                case LV_EVENT_RELEASED: {
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
    for (size_t i = 0; i < NUM_FANS; i++) {
        view_common_set_checked(pdata->btn_fans[i], i == pdata->fan_index);
    }

    lv_arc_set_value(pdata->arc_speed, pmodel->run.fan_speeds[pdata->fan_index]);
    view_common_set_checked(pdata->btn_enable_fan, pmodel->run.fan_on[pdata->fan_index]);
    view_common_set_disabled(pdata->arc_speed, !pmodel->run.fan_on[pdata->fan_index]);
    lv_label_set_text(lv_obj_get_child(pdata->btn_enable_fan, 0), pmodel->run.fan_on[pdata->fan_index] ? "ON" : "OFF");
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


static lv_obj_t *fan_button_create(lv_obj_t *root) {
    lv_obj_t *btn = lv_btn_create(root);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_size(btn, 96, 103);

    lv_obj_t *img = lv_img_create(btn);
    lv_img_set_src(img, &img_ventola);
    lv_obj_center(img);

    return btn;
}


static lv_anim_t fan_animation(lv_obj_t *img, uint32_t period) {
    lv_anim_t a;
    lv_anim_init(&a);
    /*Set the "animator" function*/
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_img_set_angle);
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
    if (speed > 0) {
        return 50000 / speed;
    } else {
        return 10000;
    }
}


const pman_page_t page_main = {
    .create        = create_page,
    .destroy       = view_destroy_all,
    .open          = open_page,
    .close         = view_close_all,
    .process_event = page_event,
};