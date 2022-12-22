#include "view.h"


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
