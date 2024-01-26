#include <stdio.h>
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
#include "peripherals/backlight.h"
#include "config/app_config.h"
#include "esp_log.h"


static const char *TAG = "Controller";


void controller_init(model_t *pmodel) {
    modbus_init();
    network_init();

    observer_init(pmodel);
    configuration_load(pmodel);
    backlight_update(1);

    view_change_page_extra(pmodel, &page_splash, (void *)(uintptr_t)0);

    //modbus_set_address(2);
}


void controller_process_message(model_t *pmodel, view_controller_message_t *msg) {
    switch (msg->code) {
        case VIEW_CONTROLLER_MESSAGE_CODE_NOTHING:
            break;

        case VIEW_CONTROLLER_MESSAGE_CODE_SET_FAN_SPEED:
            ESP_LOGI(TAG, "Control on %i", msg->fan);
            modbus_set_speed(msg->fan, msg->speed);
            // modbus_set_speed(IMMISSION_FAN, model_get_required_immission(pmodel));
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

        case VIEW_CONTROLLER_MESSAGE_CODE_READ_FW_VERSION:
            modbus_read_firmware_version(msg->device);
            break;

        case VIEW_CONTROLLER_MESSAGE_CODE_READ_FW_VERSIONS:
            modbus_read_firmware_version(1);
            modbus_read_firmware_version(2);
            modbus_read_firmware_version(3);
            modbus_read_firmware_version(4);
            break;

        case VIEW_CONTROLLER_MESSAGE_CODE_STANDBY:
            ESP_LOGI(TAG, "Standby %s", msg->value ? "ON" : "OFF");
            backlight_update(!msg->value);
            break;
    }
}


void controller_manage(model_t *pmodel) {
    (void)pmodel;
    static uint8_t ap_started = 0;

    modbus_response_t response;
    if (modbus_get_response(&response)) {
        if (model_set_communication_error(pmodel, response.error)) {
            ESP_LOGI(TAG, "Communication error %i", response.error);
            view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
        }

        switch (response.tag) {
            case MODBUS_RESPONSE_TAG_OK:
                break;

            case MODBUS_RESPONSE_TAG_FIRMWARE_VERSION:
                if (response.error) {
                    model_set_minion_firmware_version_error(pmodel, response.address);
                } else {
                    model_set_minion_firmware_version(pmodel, response.address, response.version_major,
                                                      response.version_minor, response.version_patch);
                }
                view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
                break;

            case MODBUS_RESPONSE_TAG_START_OTA:
                if (response.error) {
                    char message[64] = {0};
                    snprintf(message, sizeof(message), "Non sono riusito a raggiungere il dispositivo %i!",
                             response.address);
                    view_common_toast(message);
                } else {
                    char message[64] = {0};
                    snprintf(message, sizeof(message), "Rete WiFi per aggiornamento: " APP_CONFIG_WIFI_SSID "-%i",
                             response.address);
                    view_common_toast(message);
                }
                break;
        }
    }

    observer_observe(pmodel);

    if (ap_started != network_is_ap_running()) {
        view_common_toast("Rete WiFi per aggiornamento: " APP_CONFIG_WIFI_SSID);
        ap_started = network_is_ap_running();
    }

    if (model_set_firmware_update_state(pmodel, server_firmware_update_state())) {
        if (model_get_firmware_update_state(pmodel) != FIRMWARE_UPDATE_STATE_NONE &&
            view_current_page_id() != PAGE_ID_FIRMWARE_UPDATE) {
            view_change_page(pmodel, &page_firmware_update);
        }
    }
}
