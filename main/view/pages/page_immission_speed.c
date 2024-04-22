#include <stdlib.h>
#include "lvgl.h"
#include "model/model.h"
#include "view/view.h"
#include "view/common.h"
#include "view/view_types.h"
#include "view/theme/style.h"
#include "gel/pagemanager/page_manager.h"


enum {
    BACK_BTN_ID,
    MOTOR_1_MODIFY_BTN_ID,
    MOTOR_2_MODIFY_BTN_ID,
    MOTOR_3_MODIFY_BTN_ID,
};


struct page_data {
    lv_obj_t *slider_min_speed;

    lv_obj_t *lbl_motor_1;
    lv_obj_t *lbl_motor_2;
    lv_obj_t *lbl_motor_3;
};


static void      update_page(model_t *pmodel, struct page_data *pdata);
static lv_obj_t *percentage_editor_create(lv_obj_t *parent, lv_obj_t **lbl, int id);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = lv_mem_alloc(sizeof(struct page_data));
    assert(pdata != NULL);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;

    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);

    view_common_create_title(cont, "Porzione di Immissione", BACK_BTN_ID);

    lv_obj_t *flex = lv_obj_create(lv_scr_act());
    lv_obj_add_flag(flex, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_set_size(flex, LV_PCT(100), LV_VER_RES - 88);
    lv_obj_add_style(flex, (lv_style_t *)&style_transparent_cont, LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(flex, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(flex, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(flex, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_gap(flex, 0, LV_STATE_DEFAULT);
    lv_obj_align(flex, LV_ALIGN_BOTTOM_MID, 0, 0);

    percentage_editor_create(flex, &pdata->lbl_motor_1, MOTOR_1_MODIFY_BTN_ID);
    if (pmodel->configuration.num_fans > 1) {
        percentage_editor_create(flex, &pdata->lbl_motor_2, MOTOR_2_MODIFY_BTN_ID);
    } else {
        pdata->lbl_motor_2 = NULL;
    }
    if (pmodel->configuration.num_fans > 2) {
        percentage_editor_create(flex, &pdata->lbl_motor_3, MOTOR_3_MODIFY_BTN_ID);
    } else {
        pdata->lbl_motor_3 = NULL;
    }

    update_page(pmodel, pdata);
}


static view_message_t page_event(model_t *pmodel, void *args, view_event_t event) {
    struct page_data *pdata = args;
    view_message_t    msg   = VIEW_NULL_MESSAGE;

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

                        case MOTOR_1_MODIFY_BTN_ID:
                            model_modify_immission_percentage(pmodel, 0, 5 * event.data.number);
                            update_page(pmodel, pdata);
                            break;

                        case MOTOR_2_MODIFY_BTN_ID:
                            model_modify_immission_percentage(pmodel, 1, 5 * event.data.number);
                            update_page(pmodel, pdata);
                            break;

                        case MOTOR_3_MODIFY_BTN_ID:
                            model_modify_immission_percentage(pmodel, 2, 5 * event.data.number);
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
    lv_label_set_text_fmt(pdata->lbl_motor_1, "%s:\n%i%%", model_get_fan_name(pmodel, 0),
                          model_get_immission_percentage(pmodel, 0));
    if (pdata->lbl_motor_2 != NULL) {
        lv_label_set_text_fmt(pdata->lbl_motor_2, "%s:\n%i%%", model_get_fan_name(pmodel, 1),
                              model_get_immission_percentage(pmodel, 1));
    }
    if (pdata->lbl_motor_3 != NULL) {
        lv_label_set_text_fmt(pdata->lbl_motor_3, "%s:\n%i%%", model_get_fan_name(pmodel, 2),
                              model_get_immission_percentage(pmodel, 2));
    }
}


static lv_obj_t *percentage_editor_create(lv_obj_t *parent, lv_obj_t **lbl, int id) {
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(cont, LV_HOR_RES - 36, 72);
    lv_obj_add_style(cont, (lv_style_t *)&style_transparent_cont, LV_STATE_DEFAULT);

    *lbl = lv_label_create(cont);
    lv_obj_set_style_text_font(*lbl, STYLE_FONT_MEDIUM, LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(*lbl, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
    lv_obj_center(*lbl);

    lv_obj_t *btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 64, 64);
    lv_obj_t *lbl_symbol = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl_symbol, &lv_font_montserrat_48, LV_STATE_DEFAULT);
    lv_obj_center(lbl_symbol);
    lv_label_set_text(lbl_symbol, LV_SYMBOL_MINUS);
    view_register_object_default_callback_with_number(btn, id, -1);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 0, 0);

    btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 64, 64);
    lbl_symbol = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl_symbol, &lv_font_montserrat_48, LV_STATE_DEFAULT);
    lv_label_set_text(lbl_symbol, LV_SYMBOL_PLUS);
    lv_obj_center(lbl_symbol);
    view_register_object_default_callback_with_number(btn, id, 1);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, 0, 0);

    return cont;
}


const pman_page_t page_immission_speed = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};
