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
    OTA_BTN_ID,
    MINION_READ_FW_BTN_ID,
};


struct page_data {
    lv_obj_t *lbl_minion_fw_version[MAX_DEVICES];
};


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

    view_common_create_title(cont, "Gestione Firmware", BACK_BTN_ID);

    lv_obj_t *lbl = lv_label_create(cont);
    lv_label_set_text_fmt(lbl, "Display v%i.%i.%i", APP_CONFIG_FIRMWARE_VERSION_MAJOR,
                          APP_CONFIG_FIRMWARE_VERSION_MINOR, APP_CONFIG_FIRMWARE_VERSION_PATCH);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 80, 68);

    lv_obj_t *btn = menu_btn_create(lv_scr_act(), LV_SYMBOL_UPLOAD);
    view_register_object_default_callback(btn, OTA_BTN_ID);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 16, 72);


    lbl = lv_label_create(cont);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 80, 72 + 56);
    pdata->lbl_minion_fw_version[0] = lbl;

    btn = menu_btn_create(lv_scr_act(), LV_SYMBOL_REFRESH);
    view_register_object_default_callback(btn, MINION_READ_FW_BTN_ID);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 16, 72 + 56 + 68);

    lbl = lv_label_create(cont);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 80, 72 + 56 + 44);
    pdata->lbl_minion_fw_version[1] = lbl;

    lbl = lv_label_create(cont);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 80, 72 + 56 + 44 * 2);
    pdata->lbl_minion_fw_version[2] = lbl;

    lbl = lv_label_create(cont);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 80, 72 + 56 + 44 * 3);
    pdata->lbl_minion_fw_version[3] = lbl;

    update_page(pmodel, pdata);
}


static view_message_t page_event(model_t *pmodel, void *args, view_event_t event) {
    struct page_data *pdata = args;
    view_message_t    msg   = VIEW_NULL_MESSAGE;

    switch (event.code) {
        case VIEW_EVENT_CODE_TIMER:
            break;

        case VIEW_EVENT_CODE_OPEN:
            msg.cmsg.code = VIEW_CONTROLLER_MESSAGE_CODE_READ_FW_VERSIONS;
            break;

        case VIEW_EVENT_CODE_UPDATE:
            update_page(pmodel, pdata);
            break;

        case VIEW_EVENT_CODE_LVGL: {
            switch (event.event) {
                case LV_EVENT_CLICKED: {
                    switch (event.data.id) {
                        case BACK_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
                            break;

                        case OTA_BTN_ID:
                            msg.cmsg.code = VIEW_CONTROLLER_MESSAGE_CODE_START_OTA;
                            break;

                        case MINION_READ_FW_BTN_ID:
                            msg.cmsg.code = VIEW_CONTROLLER_MESSAGE_CODE_READ_FW_VERSIONS;
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
    view_common_set_hidden(pdata->lbl_minion_fw_version[0], pmodel->configuration.num_fans <= 0);
    view_common_set_hidden(pdata->lbl_minion_fw_version[1], pmodel->configuration.num_fans <= 1);
    view_common_set_hidden(pdata->lbl_minion_fw_version[2], pmodel->configuration.num_fans <= 2);
    view_common_set_hidden(pdata->lbl_minion_fw_version[3], !pmodel->configuration.immission_fan);

    lv_label_set_text_fmt(pdata->lbl_minion_fw_version[0], "Disp. 1: %s", model_get_minion_firmware_version(pmodel, 0));
    lv_label_set_text_fmt(pdata->lbl_minion_fw_version[1], "Disp. 2: %s", model_get_minion_firmware_version(pmodel, 1));
    lv_label_set_text_fmt(pdata->lbl_minion_fw_version[2], "Disp. 3: %s", model_get_minion_firmware_version(pmodel, 2));
    lv_label_set_text_fmt(pdata->lbl_minion_fw_version[3], "Disp. 4: %s", model_get_minion_firmware_version(pmodel, 3));
}


static lv_obj_t *menu_btn_create(lv_obj_t *root, const char *text) {
    lv_obj_t *btn = lv_btn_create(root);
    lv_obj_set_size(btn, 64, 64);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_center(lbl);

    return btn;
}


const pman_page_t page_firmware_management = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};
