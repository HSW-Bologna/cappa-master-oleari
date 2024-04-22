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
    NUM_FANS_MODIFY_BTN_ID,
    IMMISSION_FAN_BTN_ID,
};


struct page_data {
    lv_obj_t *slider_min_speed;

    lv_obj_t *lbl_num_fans;
    lv_obj_t *btn_immission_fan;
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
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);

    view_common_create_title(cont, "Numero Ventole", BACK_BTN_ID);

    lv_obj_t *editor = percentage_editor_create(cont, &pdata->lbl_num_fans, NUM_FANS_MODIFY_BTN_ID);
    lv_obj_align(editor, LV_ALIGN_CENTER, 0, -10);

    lv_obj_t *lbl = lv_label_create(cont);
    lv_label_set_text(lbl, "Immissione");
    lv_obj_align(lbl, LV_ALIGN_CENTER, -64, 100);

    lv_obj_t *btn = lv_btn_create(cont);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
    view_common_set_checked(btn, pmodel->configuration.immission_fan);
    lv_obj_set_size(btn, 80, 64);
    lv_obj_set_style_bg_color(btn, lv_color_make(0, 255, 0), LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(btn, lv_color_make(255, 0, 0), LV_STATE_DEFAULT);
    view_register_object_default_callback(btn, IMMISSION_FAN_BTN_ID);

    lbl = lv_label_create(btn);
    lv_obj_center(lbl);

    lv_obj_align(btn, LV_ALIGN_CENTER, 128, 100);

    pdata->btn_immission_fan = btn;

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

                        case NUM_FANS_MODIFY_BTN_ID:
                            model_modify_num_fans(pmodel, event.data.number);
                            update_page(pmodel, pdata);
                            break;

                        case IMMISSION_FAN_BTN_ID:
                            pmodel->configuration.immission_fan = !pmodel->configuration.immission_fan;
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
    (void)pdata;
    lv_obj_clean(lv_scr_act());
}


static void destroy_page(void *args, void *extra) {
    (void)extra;
    struct page_data *pdata = args;
    lv_mem_free(pdata);
}


static void update_page(model_t *pmodel, struct page_data *pdata) {
    lv_label_set_text_fmt(pdata->lbl_num_fans, "Ventole\n%i", pmodel->configuration.num_fans);

    view_common_set_checked(pdata->btn_immission_fan, pmodel->configuration.immission_fan);
    lv_label_set_text(lv_obj_get_child(pdata->btn_immission_fan, 0),
                      pmodel->configuration.immission_fan ? "ON" : "OFF");
}


static lv_obj_t *percentage_editor_create(lv_obj_t *parent, lv_obj_t **lbl, int id) {
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(cont, 340, 72);
    lv_obj_add_style(cont, (lv_style_t *)&style_transparent_cont, LV_STATE_DEFAULT);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

    *lbl = lv_label_create(cont);
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


const pman_page_t page_num_fans = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};
