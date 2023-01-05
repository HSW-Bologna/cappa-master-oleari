#include "controller.h"
#include "model/model.h"
#include "view/view.h"
#include "view/common.h"
#include "modbus.h"
#include "configuration.h"
#include "observer.h"
#include "network/network.h"
#include "network/server.h"
#include "peripherals/system.h"


void controller_init(model_t *pmodel) {
    modbus_init();
    network_init();

    observer_init(pmodel);
    configuration_load(pmodel);

    view_change_page(pmodel, &page_splash);
}


void controller_process_message(model_t *pmodel, view_controller_message_t *msg) {
    switch (msg->code) {
        case VIEW_CONTROLLER_MESSAGE_CODE_NOTHING:
            break;

        case VIEW_CONTROLLER_MESSAGE_CODE_SET_FAN_SPEED:
            modbus_set_speed(msg->fan, msg->speed);
            modbus_set_speed(IMMISSION_FAN, model_get_required_immission(pmodel));
            break;

        case VIEW_CONTROLLER_MESSAGE_CODE_SET_LIGHT:
            modbus_set_light(msg->light, msg->value);
            break;

        case VIEW_CONTROLLER_MESSAGE_CODE_START_OTA:
            if (network_is_ap_running()) {
                view_common_toast("Rete gia' attiva");
            } else {
                network_start_ap();
            }
            break;

        case VIEW_CONTROLLER_MESSAGE_CODE_RESET:
            system_reset();
            break;
    }
}


void controller_manage(model_t *pmodel) {
    (void)pmodel;
    static uint8_t ap_started = 0;

    modbus_response_t response;
    if (modbus_get_response(&response)) {
        switch (response) {
            case MODBUS_RESPONSE_OK:
                if (model_set_communication_error(pmodel, 0)) {
                    view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
                }
                break;

            case MODBUS_RESPONSE_ERROR:
                if (model_set_communication_error(pmodel, 1)) {
                    view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
                }
                break;
        }
    }

    observer_observe(pmodel);

    if (ap_started != network_is_ap_running()) {
        view_common_toast("Rete WiFi per l'aggiornamento firmware attivata.");
        ap_started = network_is_ap_running();
    }

    if (model_set_firmware_update_state(pmodel, server_firmware_update_state())) {
        if (model_get_firmware_update_state(pmodel) != FIRMWARE_UPDATE_STATE_NONE &&
            view_current_page_id() != PAGE_ID_FIRMWARE_UPDATE) {
            view_change_page(pmodel, &page_firmware_update);
        }
    }
}