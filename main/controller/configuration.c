#include "peripherals/storage.h"
#include "model/model.h"
#include "esp_log.h"


static const char *TAG = "Configuration";

const char *CONFIGURATION_MIN_SPEED_KEY  = "MINSPEED";
const char *CONFIGURATION_IMM_PERC_1_KEY = "IMMPERC1";
const char *CONFIGURATION_IMM_PERC_2_KEY = "IMMPERC2";
const char *CONFIGURATION_IMM_PERC_3_KEY = "IMMPERC3";


void configuration_load(model_t *pmodel) {
    storage_load_uint16(&pmodel->configuration.minimum_speed, (char *)CONFIGURATION_MIN_SPEED_KEY);
    storage_load_uint16(&pmodel->configuration.immission_percentages[0], (char *)CONFIGURATION_IMM_PERC_1_KEY);
    storage_load_uint16(&pmodel->configuration.immission_percentages[1], (char *)CONFIGURATION_IMM_PERC_2_KEY);
    storage_load_uint16(&pmodel->configuration.immission_percentages[2], (char *)CONFIGURATION_IMM_PERC_3_KEY);
}