#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "model.h"
#include "config/app_config.h"


static void check_immission_percentages(model_t *pmodel, int currently_editing_fan);


void model_init(model_t *pmodel) {
    assert(pmodel != NULL);
    pmodel->configuration.language           = 0;
    pmodel->configuration.num_fans           = 1;
    pmodel->configuration.immission_fan      = 0;
    pmodel->configuration.normal_brightness  = 80;
    pmodel->configuration.standby_brightness = 0;
    pmodel->configuration.gas_enabled        = 0;
    pmodel->configuration.logo               = LOGO_OLEARI;

    pmodel->run.communication_error   = 0;
    pmodel->run.firmware_update_state = FIRMWARE_UPDATE_STATE_NONE;
    pmodel->run.standby               = 0;

    for (size_t i = 0; i < MAX_FANS; i++) {
        snprintf(pmodel->configuration.names[i], MAX_FAN_NAME_LEN, "Cappa %zu", i + 1);
        pmodel->configuration.minimum_speeds[i] = APP_CONFIG_MIN_MINIMUM_SPEED;
        pmodel->run.fan_speeds[i]               = 50;
        pmodel->run.fan_on[i]                   = 0;
        pmodel->run.light_on[i]                 = 0;
        memset(pmodel->run.minion_firmware_version[i], 0, sizeof(pmodel->run.minion_firmware_version[i]));
        pmodel->configuration.immission_percentages[i] = 30;
    }

    check_immission_percentages(pmodel, -1);
}


void model_check_config(model_t *pmodel) {
#define CHECK_LIMITS(Var, Min, Max, Def)                                                                               \
    if ((Var) < (Min) || (Var) > (Max)) {                                                                              \
        (Var) = (Def);                                                                                                 \
    }

    for (size_t i = 0; i < MAX_FANS; i++) {
        CHECK_LIMITS(pmodel->configuration.minimum_speeds[i], APP_CONFIG_MIN_MINIMUM_SPEED,
                     APP_CONFIG_MAX_MINIMUM_SPEED, APP_CONFIG_MIN_MINIMUM_SPEED);
        CHECK_LIMITS(pmodel->run.fan_speeds[i], pmodel->configuration.minimum_speeds[i], 100,
                     pmodel->configuration.minimum_speeds[i]);
    }
    CHECK_LIMITS(pmodel->configuration.num_fans, 1, MAX_FANS, 1);
    CHECK_LIMITS(pmodel->configuration.normal_brightness, APP_CONFIG_MIN_NORMAL_BRIGHTNESS,
                 APP_CONFIG_MAX_NORMAL_BRIGHTNESS, APP_CONFIG_MAX_NORMAL_BRIGHTNESS);
    CHECK_LIMITS(pmodel->configuration.standby_brightness, APP_CONFIG_MIN_STANDBY_BRIGHTNESS,
                 APP_CONFIG_MAX_STANDBY_BRIGHTNESS, APP_CONFIG_MIN_STANDBY_BRIGHTNESS);

    check_immission_percentages(pmodel, -1);

#undef CHECK_LIMITS
}


const char *model_get_fan_name(model_t *pmodel, size_t fan_index) {
    assert(pmodel != NULL);
    return pmodel->configuration.names[fan_index];
}


void model_set_fan_name(model_t *pmodel, size_t fan_index, const char *name) {
    assert(pmodel != NULL);
    snprintf(pmodel->configuration.names[fan_index], MAX_FAN_NAME_LEN, "%s", name);
}


void model_set_minion_firmware_version(model_t *pmodel, uint16_t minion, uint16_t major, uint16_t minor,
                                       uint16_t patch) {
    assert(pmodel != NULL && minion > 0);
    snprintf(pmodel->run.minion_firmware_version[minion - 1], sizeof(pmodel->run.minion_firmware_version[minion - 1]),
             "v%u.%u.%u", major, minor, patch);
}


void model_set_minion_firmware_version_error(model_t *pmodel, uint16_t minion) {
    assert(pmodel != NULL && minion > 0);
    snprintf(pmodel->run.minion_firmware_version[minion - 1], sizeof(pmodel->run.minion_firmware_version[minion - 1]),
             "<error>");
}


const char *model_get_minion_firmware_version(model_t *pmodel, uint16_t minion) {
    assert(pmodel != NULL && minion < MAX_DEVICES);
    return pmodel->run.minion_firmware_version[minion];
}


uint16_t model_get_fan_speed(model_t *pmodel, size_t fan) {
    assert(pmodel != NULL && fan < MAX_FANS);
    return pmodel->run.fan_speeds[fan];
}


uint16_t model_get_required_immission(model_t *pmodel) {
    assert(pmodel != NULL);

    uint16_t total = 0;
    for (size_t i = 0; i < MAX_FANS; i++) {
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
    assert(pmodel != NULL && fan < MAX_FANS);

    if (speed > 100) {
        speed = 100;
    }

    pmodel->run.fan_speeds[fan] = speed;
}


uint8_t model_get_light_on(model_t *pmodel, size_t fan) {
    assert(pmodel != NULL && fan < MAX_FANS);
    return pmodel->run.light_on[fan];
}


void model_toggle_light_on(model_t *pmodel, size_t fan) {
    assert(pmodel != NULL && fan < MAX_FANS);
    pmodel->run.light_on[fan] = !pmodel->run.light_on[fan];
}


uint8_t model_get_fan_on(model_t *pmodel, size_t fan) {
    assert(pmodel != NULL && fan < MAX_FANS);
    return pmodel->run.fan_on[fan];
}


void model_toggle_fan_on(model_t *pmodel, size_t fan) {
    assert(pmodel != NULL && fan < MAX_FANS);
    pmodel->run.fan_on[fan] = !pmodel->run.fan_on[fan];
}


void model_turn_fan_off(model_t *pmodel, size_t fan) {
    assert(pmodel != NULL && fan < MAX_FANS);
    pmodel->run.fan_on[fan] = 0;
}


uint16_t model_get_language(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->configuration.language;
}


uint16_t model_get_immission_percentage(model_t *pmodel, size_t fan) {
    assert(pmodel != NULL && fan < MAX_FANS);
    return pmodel->configuration.immission_percentages[fan];
}


void model_modify_num_fans(model_t *pmodel, int16_t mod) {
    assert(pmodel != NULL);
    if (mod > 0) {
        pmodel->configuration.num_fans += mod;
        if (pmodel->configuration.num_fans > MAX_FANS) {
            pmodel->configuration.num_fans = MAX_FANS;
        }
    } else {
        if (pmodel->configuration.num_fans > -mod + 1) {
            pmodel->configuration.num_fans += mod;
        } else {
            pmodel->configuration.num_fans = 1;
        }
    }
}


void model_modify_immission_percentage(model_t *pmodel, size_t fan, int16_t mod) {
    assert(pmodel != NULL && fan < MAX_FANS);
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


void model_set_minimum_speed(model_t *pmodel, uint16_t fan, uint16_t speed) {
    assert(pmodel != NULL && fan < MAX_FANS);

    if (speed < APP_CONFIG_MIN_MINIMUM_SPEED) {
        speed = APP_CONFIG_MIN_MINIMUM_SPEED;
    } else if (speed > 100) {
        speed = 100;
    }
    pmodel->configuration.minimum_speeds[fan] = speed;
}


uint16_t model_get_minimum_speed(model_t *pmodel, uint16_t fan) {
    assert(pmodel != NULL && fan < MAX_FANS);

    return pmodel->configuration.minimum_speeds[fan];
}


static void check_immission_percentages(model_t *pmodel, int currently_editing_fan) {
    uint16_t total    = 0;
    size_t   num_fans = pmodel->configuration.num_fans;

    for (size_t i = 0; i < num_fans; i++) {
        total += model_get_immission_percentage(pmodel, i);
    }

    if (currently_editing_fan < 0) {
        currently_editing_fan = 0;
    }

    int16_t correction = 100 - total;

    if (num_fans > 1) {
        for (size_t i = (currently_editing_fan + 1) % num_fans; i != (size_t)currently_editing_fan && correction != 0;
             i        = (i + 1) % num_fans) {

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
    } else {
        pmodel->configuration.immission_percentages[0] = 100;
    }
}
