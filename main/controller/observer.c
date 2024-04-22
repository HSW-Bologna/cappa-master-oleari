#include <assert.h>
#include "configuration.h"
#include "gel/data_structures/watcher.h"
#include "gel/timer/timecheck.h"
#include "peripherals/storage.h"
#include "peripherals/backlight.h"
#include "utils/utils.h"
#include "modbus.h"
#include "esp_log.h"


#define NUM_OBSERVED_VARIABLES (15)


void update_brightness(void *mem, void *data);


static const char *TAG = "Observer";

static watcher_t watchlist[NUM_OBSERVED_VARIABLES + 1] = {0};
static uint16_t  old_fan_speeds[MAX_FANS]              = {0};
static uint16_t  old_fan_on[MAX_FANS]                  = {0};


void observer_init(model_t *pmodel) {
    size_t i = 0;

    for (size_t i = 0; i < MAX_FANS; i++) {
        old_fan_speeds[i] = model_get_fan_speed(pmodel, i);
    }

    watchlist[i++] = WATCHER_DELAYED(&pmodel->configuration.minimum_speeds[0], storage_save_uint16,
                                     CONFIGURATION_MIN_SPEED_1_KEY, 4000UL);
    watchlist[i++] = WATCHER_DELAYED(&pmodel->configuration.minimum_speeds[1], storage_save_uint16,
                                     CONFIGURATION_MIN_SPEED_2_KEY, 4000UL);
    watchlist[i++] = WATCHER_DELAYED(&pmodel->configuration.minimum_speeds[2], storage_save_uint16,
                                     CONFIGURATION_MIN_SPEED_3_KEY, 4000UL);
    watchlist[i++] = WATCHER_DELAYED(&pmodel->configuration.immission_percentages[0], storage_save_uint16,
                                     CONFIGURATION_IMM_PERC_1_KEY, 4000UL);
    watchlist[i++] = WATCHER_DELAYED(&pmodel->configuration.immission_percentages[1], storage_save_uint16,
                                     CONFIGURATION_IMM_PERC_2_KEY, 4000UL);
    watchlist[i++] = WATCHER_DELAYED(&pmodel->configuration.immission_percentages[2], storage_save_uint16,
                                     CONFIGURATION_IMM_PERC_3_KEY, 4000UL);
    watchlist[i++] =
        WATCHER_DELAYED(&pmodel->configuration.num_fans, storage_save_uint16, CONFIGURATION_NUM_FANS_KEY, 4000UL);
    watchlist[i++] = WATCHER_DELAYED(&pmodel->configuration.immission_fan, storage_save_uint8,
                                     CONFIGURATION_IMMISSION_FAN_KEY, 4000UL);
    watchlist[i++] = WATCHER_DELAYED(&pmodel->configuration.normal_brightness, storage_save_uint8,
                                     CONFIGURATION_NORMAL_BRIGHTNESS_KEY, 4000UL);
    watchlist[i++] = WATCHER_DELAYED(&pmodel->configuration.standby_brightness, storage_save_uint8,
                                     CONFIGURATION_STANDBY_BRIGHTNESS_KEY, 4000UL);
    watchlist[i++] = WATCHER_DELAYED(&pmodel->configuration.logo, storage_save_uint8, CONFIGURATION_LOGO_KEY, 2000UL);
    watchlist[i++] =
        WATCHER_DELAYED(&pmodel->configuration.gas_enabled, storage_save_uint8, CONFIGURATION_GAS_KEY, 2000UL);
    watchlist[i++] = WATCHER(&pmodel->configuration.normal_brightness, update_brightness, pmodel);
    watchlist[i++] = WATCHER(&pmodel->configuration.standby_brightness, update_brightness, pmodel);
    watchlist[i++] = WATCHER(&pmodel->run.standby, update_brightness, pmodel);

    assert(NUM_OBSERVED_VARIABLES == i);
    watchlist[i++] = WATCHER_NULL;

    watcher_list_init(watchlist);
}


void observer_observe(model_t *pmodel) {
    static unsigned long timestamp   = 0;
    uint8_t              fan_changed = 0;

    watcher_process_changes(watchlist, get_millis());

    if (is_expired(timestamp, get_millis(), 500)) {
        for (size_t i = 0; i < MAX_FANS; i++) {
            if (old_fan_speeds[i] != model_get_fan_speed(pmodel, i)) {
                if (model_get_fan_on(pmodel, i)) {
                    modbus_set_speed(i, model_get_fan_speed(pmodel, i), pmodel->configuration.gas_enabled);
                } else {
                    modbus_set_speed(i, 0, pmodel->configuration.gas_enabled);
                }
                fan_changed = 1;

                old_fan_speeds[i] = model_get_fan_speed(pmodel, i);
            }
        }

        timestamp = get_millis();
    }

    for (size_t i = 0; i < MAX_FANS; i++) {
        if (old_fan_on[i] != model_get_fan_on(pmodel, i)) {
            if (model_get_fan_on(pmodel, i)) {
                modbus_set_speed(i, model_get_fan_speed(pmodel, i), pmodel->configuration.gas_enabled);
            } else {
                modbus_set_speed(i, 0, pmodel->configuration.gas_enabled);
            }
            fan_changed = 1;

            old_fan_on[i] = model_get_fan_on(pmodel, i);
        }
    }

    if (pmodel->configuration.immission_fan && fan_changed) {
        modbus_set_speed(IMMISSION_FAN, model_get_required_immission(pmodel), pmodel->configuration.gas_enabled);
    }
}


void update_brightness(void *mem, void *data) {
    (void)mem;
    model_t *pmodel = data;
    if (pmodel->run.standby) {
        ESP_LOGI(TAG, "Updating standby brightness");
        backlight_update(pmodel->configuration.standby_brightness);
    } else {
        ESP_LOGI(TAG, "Updating normal brightness");
        backlight_update(pmodel->configuration.normal_brightness);
    }
}
