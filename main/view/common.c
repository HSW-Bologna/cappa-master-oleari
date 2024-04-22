#include "view.h"
#include "common.h"
#include "theme/style.h"


static void delete_obj_timer(lv_timer_t *timer);


lv_obj_t *view_common_create_title(lv_obj_t *parent, const char *text, int id) {
    lv_obj_t *btn = view_common_back_btn_create(parent);
    view_register_object_default_callback(btn, id);

    lv_obj_t *lbl = lv_label_create(parent);
    lv_obj_set_width(lbl, LV_HOR_RES - 80);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);

    size_t len = strlen(text);
    if (len > 12) {
        lv_obj_set_style_text_font(lbl, STYLE_FONT_MEDIUM, LV_STATE_DEFAULT);
    } else {
        lv_obj_set_style_text_font(lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    }
    lv_label_set_text(lbl, text);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 0, 0);

    return btn;
}


void view_common_set_checked(lv_obj_t *obj, uint8_t checked) {
    if ((lv_obj_get_state(obj) & LV_STATE_CHECKED) > 0 && !checked) {
        lv_obj_clear_state(obj, LV_STATE_CHECKED);
    } else if ((lv_obj_get_state(obj) & LV_STATE_CHECKED) == 0 && checked) {
        lv_obj_add_state(obj, LV_STATE_CHECKED);
    }
}


void view_common_set_disabled(lv_obj_t *obj, uint8_t disabled) {
    if ((lv_obj_get_state(obj) & LV_STATE_DISABLED) > 0 && !disabled) {
        lv_obj_clear_state(obj, LV_STATE_DISABLED);
    } else if ((lv_obj_get_state(obj) & LV_STATE_DISABLED) == 0 && disabled) {
        lv_obj_add_state(obj, LV_STATE_DISABLED);
    }
}


void view_common_set_hidden(lv_obj_t *obj, int hidden) {
    if (((obj->flags & LV_OBJ_FLAG_HIDDEN) == 0) && hidden) {
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    } else if (((obj->flags & LV_OBJ_FLAG_HIDDEN) > 0) && !hidden) {
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
}


password_page_options_t *view_common_default_password_page_options(view_page_message_t msg, const char *password) {
    password_page_options_t *fence = (password_page_options_t *)lv_mem_alloc(sizeof(password_page_options_t));
    assert(fence != NULL);
    fence->password = password;
    fence->msg      = msg;
    return fence;
}


lv_obj_t *view_common_back_btn_create(lv_obj_t *root) {
    lv_obj_t *btn = lv_btn_create(root);
    lv_obj_set_size(btn, 64, 64);
    lv_obj_t *lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_48, LV_STATE_DEFAULT);
    lv_label_set_text(lbl, LV_SYMBOL_CLOSE);
    lv_obj_center(lbl);
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, 0, -8);

    return btn;
}


lv_obj_t *view_common_toast(const char *msg) {
    lv_obj_t *obj = view_common_toast_with_parent(msg, lv_layer_top());
    return obj;
}


lv_obj_t *view_common_toast_with_parent(const char *msg, lv_obj_t *parent) {
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_width(obj, 400);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(obj, (lv_style_t *)&style_toast, LV_STATE_DEFAULT);

    lv_obj_t *lbl = lv_label_create(obj);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_label_set_text(lbl, msg);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_width(lbl, 380);

    lv_obj_align(obj, LV_ALIGN_BOTTOM_MID, 0, -40);

    lv_timer_t *timer = lv_timer_create(delete_obj_timer, 5000, obj);
    lv_timer_set_repeat_count(timer, 1);

    return obj;
}


static void delete_obj_timer(lv_timer_t *timer) {
    lv_obj_del(timer->user_data);
}
