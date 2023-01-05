#include <stdlib.h>
#include "lvgl.h"
#include "model/model.h"
#include "view/view.h"
#include "view/view_types.h"
#include "view/theme/style.h"
#include "gel/pagemanager/page_manager.h"


LV_IMG_DECLARE(img_logo_oleari);



struct page_data {
    lv_timer_t *timer;
};


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = lv_mem_alloc(sizeof(struct page_data));
    assert(pdata != NULL);
    pdata->timer = view_register_periodic_timer(4000UL, 0);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_timer_resume(pdata->timer);

    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *img = lv_img_create(cont);
    lv_img_set_src(img, &img_logo_oleari);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
}


static view_message_t page_event(model_t *pmodel, void *args, view_event_t event) {
    view_message_t msg = VIEW_NULL_MESSAGE;

    switch (event.code) {
        case VIEW_EVENT_CODE_TIMER:
            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_REBASE;
            msg.vmsg.page = (void *)&page_main;
            break;

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


const pman_page_t page_splash = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};