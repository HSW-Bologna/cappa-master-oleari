#include <stdlib.h>
#include "lvgl.h"
#include "model/model.h"
#include "view/view.h"
#include "view/common.h"
#include "view/view_types.h"
#include "view/theme/style.h"
#include "gel/pagemanager/page_manager.h"


enum {
    BTN_OK,
};


struct page_data {
    lv_timer_t *timer;

    lv_obj_t *lbl_status;

    lv_obj_t *btn_ok;

    lv_obj_t *spinner;
};


static void update_page(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = lv_mem_alloc(sizeof(struct page_data));
    assert(pdata != NULL);
    pdata->timer = view_register_periodic_timer(100UL, 0);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_obj_t         *lbl, *spinner, *btn;

    lv_timer_resume(pdata->timer);

    lbl = lv_label_create(lv_scr_act());
    lv_obj_set_width(lbl, LV_PCT(95));
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, -64);
    pdata->lbl_status = lbl;

    btn = lv_btn_create(lv_scr_act());
    lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "Conferma");
    lv_obj_center(lbl);
    view_register_object_default_callback(btn, BTN_OK);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 64);
    pdata->btn_ok = btn;

    spinner = lv_spinner_create(lv_scr_act(), 2000, 60);
    lv_obj_align(spinner, LV_ALIGN_CENTER, 0, 64);
    pdata->spinner = spinner;

    update_page(pmodel, pdata);
}


static view_message_t page_event(model_t *pmodel, void *args, view_event_t event) {
    view_message_t    msg   = VIEW_NULL_MESSAGE;
    struct page_data *pdata = args;

    switch (event.code) {
        case VIEW_EVENT_CODE_TIMER:
            update_page(pmodel, pdata);
            break;

        case VIEW_EVENT_CODE_LVGL: {
            switch (event.event) {
                case LV_EVENT_CLICKED: {
                    switch (event.data.id) {
                        case BTN_OK: {
                            switch (model_get_firmware_update_state(pmodel)) {
                                case FIRMWARE_UPDATE_STATE_SUCCESS:
                                case FIRMWARE_UPDATE_STATE_NONE:
                                    msg.cmsg.code = VIEW_CONTROLLER_MESSAGE_CODE_RESET;
                                    break;

                                case FIRMWARE_UPDATE_STATE_FAILURE:
                                    model_set_firmware_update_state(pmodel, FIRMWARE_UPDATE_STATE_NONE);
                                    msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_REBASE;
                                    msg.vmsg.page = (void *)&page_main;
                                    break;

                                default:
                                    break;
                            }
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
    lv_timer_pause(pdata->timer);
    lv_obj_clean(lv_scr_act());
}


static void destroy_page(void *args, void *extra) {
    (void)extra;
    struct page_data *pdata = args;
    lv_timer_del(pdata->timer);
    lv_mem_free(pdata);
}


static void update_page(model_t *pmodel, struct page_data *pdata) {
    switch (model_get_firmware_update_state(pmodel)) {
        case FIRMWARE_UPDATE_STATE_SUCCESS:
        case FIRMWARE_UPDATE_STATE_NONE:
            view_common_set_hidden(pdata->spinner, 1);
            view_common_set_hidden(pdata->btn_ok, 0);
            lv_label_set_text(pdata->lbl_status, "Aggiornamento firmware concluso");
            break;

        case FIRMWARE_UPDATE_STATE_UPDATING:
            view_common_set_hidden(pdata->spinner, 0);
            view_common_set_hidden(pdata->btn_ok, 1);
            lv_label_set_text(pdata->lbl_status, "Aggiornamento firmware in corso");
            break;

        case FIRMWARE_UPDATE_STATE_FAILURE:
            view_common_set_hidden(pdata->spinner, 1);
            view_common_set_hidden(pdata->btn_ok, 0);
            lv_label_set_text(pdata->lbl_status, "Aggiornamento firmware fallito");
            break;
    }
}


const pman_page_t page_firmware_update = {
    .id            = PAGE_ID_FIRMWARE_UPDATE,
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};