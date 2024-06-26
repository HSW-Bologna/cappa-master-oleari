#include <stdlib.h>
#include "lvgl.h"
#include "model/model.h"
#include "view/view.h"
#include "view/common.h"
#include "view/view_types.h"
#include "view/theme/style.h"
#include "gel/pagemanager/page_manager.h"
#include "config/app_config.h"


enum {
    BACK_BTN_ID,
    SETTINGS_BTN_ID,
    BRIGHTNESS_BTN_ID,
    OTA_BTN_ID,
};


struct page_data {};


static void      update_page(model_t *pmodel, struct page_data *pdata);
static lv_obj_t *menu_btn_create(lv_obj_t *root, const char *text);


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
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    view_common_create_title(cont, "Impostazioni", BACK_BTN_ID);

    lv_obj_t *btn = menu_btn_create(cont, "Parametri");
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, -40);
    view_register_object_default_callback(btn, SETTINGS_BTN_ID);

    btn = menu_btn_create(cont, "Luminosita'");
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 40);
    view_register_object_default_callback(btn, BRIGHTNESS_BTN_ID);

    btn = menu_btn_create(cont, "Gestione Firmware");
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 120);
    view_register_object_default_callback(btn, OTA_BTN_ID);


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

                        case BRIGHTNESS_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE;
                            msg.vmsg.page = (void *)&page_brightness;
                            break;

                        case SETTINGS_BTN_ID: {
                            view_page_message_t pw_msg = {
                                .code = VIEW_PAGE_MESSAGE_CODE_SWAP,
                                .page = (void *)&page_settings,
                            };
                            password_page_options_t *opts =
                                view_common_default_password_page_options(pw_msg, (const char *)APP_CONFIG_PASSWORD);
                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.extra = opts;
                            msg.vmsg.page  = (void *)&page_password;
                            break;
                        }

                        case OTA_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE;
                            msg.vmsg.page = (void *)&page_firmware_management;
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


static void update_page(model_t *pmodel, struct page_data *pdata) {}


static lv_obj_t *menu_btn_create(lv_obj_t *root, const char *text) {
    lv_obj_t *btn = lv_btn_create(root);
    lv_obj_set_size(btn, 440, 64);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_center(lbl);

    return btn;
}


const pman_page_t page_menu = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};
