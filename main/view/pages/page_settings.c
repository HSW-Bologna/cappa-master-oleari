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
    MIN_SPEED_BTN_ID,
    IMMISSION_SPEED_BTN_ID,
    NUM_FANS_BTN_ID,
    NAME_FANS_BTN_ID,
    LOGO_BTN_ID,
    GAS_BTN_ID,
};


struct page_data {};


static void      update_page(model_t *pmodel, struct page_data *pdata);
static lv_obj_t *menu_btn_create(lv_obj_t *root, const char *text, int id);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = lv_mem_alloc(sizeof(struct page_data));
    assert(pdata != NULL);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;

    lv_obj_t *screen = lv_obj_create(lv_scr_act());
    lv_obj_set_size(screen, LV_HOR_RES, LV_VER_RES);
    lv_obj_align(screen, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    view_common_create_title(screen, "Parametri", BACK_BTN_ID);

    lv_obj_t *cont = lv_obj_create(screen);
    lv_obj_add_style(cont, (lv_style_t *)&style_transparent_cont, LV_STATE_DEFAULT);
    lv_obj_set_size(cont, LV_PCT(100), LV_VER_RES - 80);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    menu_btn_create(cont, "Velocita' minima", MIN_SPEED_BTN_ID);
    // menu_btn_create(cont, "Nomi", NAME_FANS_BTN_ID);
    menu_btn_create(cont, "Numero ventole", NUM_FANS_BTN_ID);
    lv_obj_t *btn = menu_btn_create(cont, "Porzione di immissione", IMMISSION_SPEED_BTN_ID);
    if (!pmodel->configuration.immission_fan) {
        view_common_set_disabled(btn, 1);
    }
    menu_btn_create(cont, "Logo", LOGO_BTN_ID);
    menu_btn_create(cont, "Gas", GAS_BTN_ID);

    update_page(pmodel, pdata);
}


static view_message_t page_event(model_t *pmodel, void *args, view_event_t event) {
    view_message_t msg = VIEW_NULL_MESSAGE;

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

                        case MIN_SPEED_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE;
                            msg.vmsg.page = (void *)&page_minimum_speed;
                            break;

                        case IMMISSION_SPEED_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE;
                            msg.vmsg.page = (void *)&page_immission_speed;
                            break;

                        case NUM_FANS_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE;
                            msg.vmsg.page = (void *)&page_num_fans;
                            break;

                        case GAS_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE;
                            msg.vmsg.page = (void *)&page_gas;
                            break;

                        case LOGO_BTN_ID: {
                            view_page_message_t pw_msg = {
                                .code = VIEW_PAGE_MESSAGE_CODE_SWAP,
                                .page = (void *)&page_logo,
                            };
                            password_page_options_t *opts =
                                view_common_default_password_page_options(pw_msg, (const char *)"12345");
                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.extra = opts;
                            msg.vmsg.page  = (void *)&page_password;
                            break;
                        }
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


static void update_page(model_t *pmodel, struct page_data *pdata) {}


static lv_obj_t *menu_btn_create(lv_obj_t *root, const char *text, int id) {
    lv_obj_t *btn = lv_btn_create(root);
    lv_obj_set_size(btn, 210, 60);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_obj_set_width(lbl, 200);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(lbl, STYLE_FONT_MEDIUM, LV_STATE_DEFAULT);
    lv_label_set_text(lbl, text);
    lv_obj_center(lbl);

    view_register_object_default_callback(btn, id);

    return btn;
}


const pman_page_t page_settings = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};
