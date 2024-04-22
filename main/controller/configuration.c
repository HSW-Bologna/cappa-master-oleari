#include "peripherals/storage.h"
#include "model/model.h"
#include "esp_log.h"


static const char *TAG = "Configuration";

const char *CONFIGURATION_MIN_SPEED_1_KEY        = "MINSPEED1";
const char *CONFIGURATION_MIN_SPEED_2_KEY        = "MINSPEED2";
const char *CONFIGURATION_MIN_SPEED_3_KEY        = "MINSPEED3";
const char *CONFIGURATION_IMM_PERC_1_KEY         = "IMMPERC1";
const char *CONFIGURATION_IMM_PERC_2_KEY         = "IMMPERC2";
const char *CONFIGURATION_IMM_PERC_3_KEY         = "IMMPERC3";
const char *CONFIGURATION_NUM_FANS_KEY           = "NUMFANS";
const char *CONFIGURATION_IMMISSION_FAN_KEY      = "IMMFAN";
const char *CONFIGURATION_NORMAL_BRIGHTNESS_KEY  = "NORMALB";
const char *CONFIGURATION_STANDBY_BRIGHTNESS_KEY = "STANDBYB";
const char *CONFIGURATION_LOGO_KEY               = "LOGO";
const char *CONFIGURATION_GAS_KEY                = "GAS";


void configuration_load(model_t *pmodel) {
    storage_load_uint16(&pmodel->configuration.minimum_speeds[0], (char *)CONFIGURATION_MIN_SPEED_1_KEY);
    storage_load_uint16(&pmodel->configuration.minimum_speeds[1], (char *)CONFIGURATION_MIN_SPEED_2_KEY);
    storage_load_uint16(&pmodel->configuration.minimum_speeds[2], (char *)CONFIGURATION_MIN_SPEED_3_KEY);
    storage_load_uint16(&pmodel->configuration.immission_percentages[0], (char *)CONFIGURATION_IMM_PERC_1_KEY);
    storage_load_uint16(&pmodel->configuration.immission_percentages[1], (char *)CONFIGURATION_IMM_PERC_2_KEY);
    storage_load_uint16(&pmodel->configuration.immission_percentages[2], (char *)CONFIGURATION_IMM_PERC_3_KEY);
    storage_load_uint16(&pmodel->configuration.num_fans, (char *)CONFIGURATION_NUM_FANS_KEY);
    storage_load_uint8(&pmodel->configuration.immission_fan, (char *)CONFIGURATION_IMMISSION_FAN_KEY);
    storage_load_uint8(&pmodel->configuration.normal_brightness, (char *)CONFIGURATION_NORMAL_BRIGHTNESS_KEY);
    storage_load_uint8(&pmodel->configuration.standby_brightness, (char *)CONFIGURATION_STANDBY_BRIGHTNESS_KEY);
    storage_load_uint8(&pmodel->configuration.logo, (char *)CONFIGURATION_LOGO_KEY);
    storage_load_uint8(&pmodel->configuration.gas_enabled, (char *)CONFIGURATION_GAS_KEY);

    model_check_config(pmodel);

    ESP_LOGI(TAG, "Configuration loaded");
}
