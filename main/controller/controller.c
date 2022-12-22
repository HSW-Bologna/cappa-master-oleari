#include "controller.h"
#include "model/model.h"
#include "view/view.h"
#include "modbus.h"


void controller_init(model_t *pmodel) {
    modbus_init();

    view_change_page(pmodel, &page_main);
}


void controller_process_message(model_t *pmodel, view_controller_message_t *msg) {
    switch (msg->code) {
        case VIEW_CONTROLLER_MESSAGE_CODE_NOTHING:
            break;

        case VIEW_CONTROLLER_MESSAGE_CODE_SET_FAN_SPEED:
            modbus_set_speed(msg->fan, msg->speed);
            break;
    }
}


void controller_manage(model_t *pmodel) {
    (void)pmodel;

    modbus_response_t response;
    modbus_get_response(&response);
}