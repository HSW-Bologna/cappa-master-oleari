#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>


#define MAX_FANS         3
#define MAX_DEVICES      4
#define IMMISSION_FAN    3
#define MAX_FAN_NAME_LEN 33


#define GETTER(name, field)                                                                                            \
    static inline                                                                                                      \
        __attribute__((always_inline, const)) typeof(((model_t *)0)->field) model_get_##name(model_t *pmodel) {        \
        assert(pmodel != NULL);                                                                                        \
        return pmodel->field;                                                                                          \
    }

#define SETTER(name, field)                                                                                            \
    static inline __attribute__((always_inline))                                                                       \
    uint8_t model_set_##name(model_t *pmodel, typeof(((model_t *)0)->field) value) {                                   \
        assert(pmodel != NULL);                                                                                        \
        if (pmodel->field != value) {                                                                                  \
            pmodel->field = value;                                                                                     \
            return 1;                                                                                                  \
        } else {                                                                                                       \
            return 0;                                                                                                  \
        }                                                                                                              \
    }

#define TOGGLER(name, field)                                                                                           \
    static inline __attribute__((always_inline)) void model_toggle_##name(model_t *pmodel) {                           \
        assert(pmodel != NULL);                                                                                        \
        pmodel->field = !pmodel->field;                                                                                \
    }

#define GETTERNSETTER(name, field)                                                                                     \
    GETTER(name, field)                                                                                                \
    SETTER(name, field)


typedef enum {
    FIRMWARE_UPDATE_STATE_NONE = 0,
    FIRMWARE_UPDATE_STATE_UPDATING,
    FIRMWARE_UPDATE_STATE_SUCCESS,
    FIRMWARE_UPDATE_STATE_FAILURE,
} firmware_update_state_t;


typedef enum {
    LOGO_OLEARI,
    LOGO_HSW,
    LOGO_ANTRALUX,
} logo_t;


typedef struct {
    struct {
        char     names[MAX_FANS][MAX_FAN_NAME_LEN];
        uint8_t  logo;
        uint16_t language;
        uint16_t minimum_speeds[MAX_FANS];
        uint16_t immission_percentages[MAX_FANS];
        uint16_t num_fans;
        uint8_t  immission_fan;
        uint8_t  normal_brightness;
        uint8_t  standby_brightness;
        uint8_t  gas_enabled;
    } configuration;

    struct {
        uint16_t fan_speeds[MAX_FANS];
        uint8_t  fan_on[MAX_FANS];
        uint8_t  light_on[MAX_FANS];
        uint8_t  communication_error;
        uint8_t  standby;

        firmware_update_state_t firmware_update_state;

        char minion_firmware_version[MAX_DEVICES][32];
    } run;
} model_t;


void model_init(model_t *pmodel);

uint16_t    model_get_language(model_t *pmodel);
uint16_t    model_get_fan_speed(model_t *pmodel, size_t fan);
void        model_set_fan_speed(model_t *pmodel, size_t fan, uint16_t speed);
uint8_t     model_get_light_on(model_t *pmodel, size_t fan);
void        model_toggle_light_on(model_t *pmodel, size_t fan);
uint8_t     model_get_fan_on(model_t *pmodel, size_t fan);
void        model_toggle_fan_on(model_t *pmodel, size_t fan);
uint16_t    model_get_immission_percentage(model_t *pmodel, size_t fan);
void        model_modify_immission_percentage(model_t *pmodel, size_t fan, int16_t mod);
uint16_t    model_get_required_immission(model_t *pmodel);
void        model_set_minion_firmware_version(model_t *pmodel, uint16_t minion, uint16_t major, uint16_t minor,
                                              uint16_t patch);
void        model_set_minion_firmware_version_error(model_t *pmodel, uint16_t minion);
const char *model_get_minion_firmware_version(model_t *pmodel, uint16_t minion);
void        model_check_config(model_t *pmodel);
void        model_modify_num_fans(model_t *pmodel, int16_t mod);
uint16_t    model_get_minimum_speed(model_t *pmodel, uint16_t fan);
void        model_set_minimum_speed(model_t *pmodel, uint16_t fan, uint16_t speed);
void        model_turn_fan_off(model_t *pmodel, size_t fan);
const char *model_get_fan_name(model_t *pmodel, size_t fan_index);

GETTERNSETTER(communication_error, run.communication_error);
GETTERNSETTER(firmware_update_state, run.firmware_update_state);


#endif
