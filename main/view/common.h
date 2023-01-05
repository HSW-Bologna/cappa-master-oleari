#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED


typedef struct {
    const char         *password;
    view_page_message_t msg;
} password_page_options_t;


void                     view_common_set_checked(lv_obj_t *obj, uint8_t checked);
void                     view_common_set_disabled(lv_obj_t *obj, uint8_t disabled);
password_page_options_t *view_common_default_password_page_options(view_page_message_t msg, const char *password);
lv_obj_t                *view_common_back_btn_create(lv_obj_t *root);
lv_obj_t                *view_common_toast_with_parent(const char *msg, lv_obj_t *parent);
lv_obj_t                *view_common_toast(const char *msg);
void                     view_common_set_hidden(lv_obj_t *obj, int hidden);


#endif