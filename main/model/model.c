#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "model.h"


static void check_immission_percentages(model_t *pmodel, int currently_editing_fan);


void model_init(model_t *pmodel) {
    assert(pmodel != NULL);
    pmodel->configuration.language      = 0;
    pmodel->configuration.minimum_speed = 0;

    pmodel->run.communication_error   = 0;
    pmodel->run.firmware_update_state = FIRMWARE_UPDATE_STATE_NONE;

    for (size_t i = 0; i < NUM_FANS; i++) {
        pmodel->run.fan_speeds[i] = 50;
        pmodel->run.fan_on[i]     = 0;
        pmodel->run.light_on[i]   = 0;
        memset(pmodel->run.minion_firmware_version[i], 0, sizeof(pmodel->run.minion_firmware_version[i]));
        pmodel->configuration.immission_percentages[i] = 30;
    }

    check_immission_percentages(pmodel, -1);
}


void model_set_minion_firmware_version(model_t *pmodel, uint16_t minion, uint16_t major, uint16_t minor,
                                       uint16_t patch) {
    assert(pmodel != NULL && minion < 2);
    snprintf(pmodel->run.minion_firmware_version[minion], sizeof(pmodel->run.minion_firmware_version[minion]),
             "v%u.%u.%u", major, minor, patch);
}


void model_set_minion_firmware_version_error(model_t *pmodel, uint16_t minion) {
    assert(pmodel != NULL && minion < 2);
    snprintf(pmodel->run.minion_firmware_version[minion], sizeof(pmodel->run.minion_firmware_version[minion]),
             "<error>");
}


const char *model_get_minion_firmware_version(model_t *pmodel, uint16_t minion) {
    assert(pmodel != NULL && minion < 2);
    return pmodel->run.minion_firmware_version[minion];
}


uint16_t model_get_fan_speed(model_t *pmodel, size_t fan) {
    assert(pmodel != NULL && fan < NUM_FANS);
    return pmodel->run.fan_speeds[fan];
}


uint16_t model_get_required_immission(model_t *pmodel) {
    assert(pmodel != NULL);

    uint16_t total = 0;
    for (size_t i = 0; i < NUM_FANS; i++) {
        total +=
            (model_get_fan_speed(pmodel, i) * model_get_immission_percentage(pmodel, i) * model_get_fan_on(pmodel, i)) /
            100;
    }

    if (total > 100) {
        return 100;
    } else {
        return total;
    }
}


void model_set_fan_speed(model_t *pmodel, size_t fan, uint16_t speed) {
    assert(pmodel != NULL && fan < NUM_FANS);

    if (speed > 100) {
        speed = 100;
    }

    pmodel->run.fan_speeds[fan] = speed;
}


uint8_t model_get_light_on(model_t *pmodel, size_t fan) {
    assert(pmodel != NULL && fan < NUM_FANS);
    return pmodel->run.light_on[fan];
}


void model_toggle_light_on(model_t *pmodel, size_t fan) {
    assert(pmodel != NULL && fan < NUM_FANS);
    pmodel->run.light_on[fan] = !pmodel->run.light_on[fan];
}


uint8_t model_get_fan_on(model_t *pmodel, size_t fan) {
    assert(pmodel != NULL && fan < NUM_FANS);
    return pmodel->run.fan_on[fan];
}


void model_toggle_fan_on(model_t *pmodel, size_t fan) {
    assert(pmodel != NULL && fan < NUM_FANS);
    pmodel->run.fan_on[fan] = !pmodel->run.fan_on[fan];
}


uint16_t model_get_language(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->configuration.language;
}


uint16_t model_get_immission_percentage(model_t *pmodel, size_t fan) {
    assert(pmodel != NULL && fan < NUM_FANS);
    return pmodel->configuration.immission_percentages[fan];
}


void model_modify_immission_percentage(model_t *pmodel, size_t fan, int16_t mod) {
    assert(pmodel != NULL && fan < NUM_FANS);
    if (mod > 0) {
        pmodel->configuration.immission_percentages[fan] += mod;
        if (pmodel->configuration.immission_percentages[fan] > 100) {
            pmodel->configuration.immission_percentages[fan] = 100;
        }
    } else {
        if (pmodel->configuration.immission_percentages[fan] > -mod) {
            pmodel->configuration.immission_percentages[fan] += mod;
        } else {
            pmodel->configuration.immission_percentages[fan] = 0;
        }
    }

    check_immission_percentages(pmodel, fan);
}


static void check_immission_percentages(model_t *pmodel, int currently_editing_fan) {
    assert(pmodel != NULL);

    uint16_t total = 0;
    for (size_t i = 0; i < NUM_FANS; i++) {
        total += model_get_immission_percentage(pmodel, i);
    }

    if (currently_editing_fan < 0) {
        currently_editing_fan = 0;
    }

    int16_t correction = 100 - total;

    for (size_t i = (currently_editing_fan + 1) % NUM_FANS; i != (size_t)currently_editing_fan && correction != 0;
         i        = (i + 1) % NUM_FANS) {

        if (correction > 0) {
            if (model_get_immission_percentage(pmodel, i) + correction < 100) {
                pmodel->configuration.immission_percentages[i] += correction;
                correction = 0;
            } else {
                pmodel->configuration.immission_percentages[i] = 100;
                correction = (model_get_immission_percentage(pmodel, i) + correction) - 100;
            }
        } else {
            if (model_get_immission_percentage(pmodel, i) >= -correction) {
                pmodel->configuration.immission_percentages[i] += correction;
                correction = 0;
            } else {
                pmodel->configuration.immission_percentages[i] = 0;
                correction = (((int16_t)model_get_immission_percentage(pmodel, i)) + correction);
            }
        }
    }
}