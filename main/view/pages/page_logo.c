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
    LOGO_ROLLER_ID,
};


struct page_data {
    lv_obj_t *roller;
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

    view_common_create_title(cont, "Logo", BACK_BTN_ID);

    lv_obj_t   *roller  = lv_roller_create(cont);
    const char *options = "Oleari\nHSW\nAntralux";
    lv_roller_set_options(roller, options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(roller, 3);
    lv_obj_align(roller, LV_ALIGN_CENTER, 0, 16);
    view_register_object_default_callback(roller, LOGO_ROLLER_ID);
    pdata->roller = roller;

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
                    }
                    break;
                }

                case LV_EVENT_VALUE_CHANGED: {
                    switch (event.data.id) {
                        case LOGO_ROLLER_ID:
                            pmodel->configuration.logo = lv_roller_get_selected(pdata->roller);
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
    lv_roller_set_selected(pdata->roller, pmodel->configuration.logo, LV_ANIM_OFF);
}


const pman_page_t page_logo = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};
