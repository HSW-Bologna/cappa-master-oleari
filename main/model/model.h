#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>


#define NUM_FANS      3
#define IMMISSION_FAN 3


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


typedef struct {
    struct {
        uint16_t language;
        uint16_t minimum_speed;
        uint16_t immission_percentages[NUM_FANS];
    } configuration;

    struct {
        uint16_t fan_speeds[NUM_FANS];
        uint8_t  fan_on[NUM_FANS];
        uint8_t  light_on[NUM_FANS];
        uint8_t  communication_error;

        firmware_update_state_t firmware_update_state;
    } run;
} model_t;


void model_init(model_t *pmodel);

uint16_t model_get_language(model_t *pmodel);
uint16_t model_get_fan_speed(model_t *pmodel, size_t fan);
void     model_set_fan_speed(model_t *pmodel, size_t fan, uint16_t speed);
uint8_t  model_get_light_on(model_t *pmodel, size_t fan);
void     model_toggle_light_on(model_t *pmodel, size_t fan);
uint8_t  model_get_fan_on(model_t *pmodel, size_t fan);
void     model_toggle_fan_on(model_t *pmodel, size_t fan);
uint16_t model_get_immission_percentage(model_t *pmodel, size_t fan);
void     model_modify_immission_percentage(model_t *pmodel, size_t fan, int16_t mod);
uint16_t model_get_required_immission(model_t *pmodel);

GETTERNSETTER(minimum_speed, configuration.minimum_speed);
GETTERNSETTER(communication_error, run.communication_error);
GETTERNSETTER(firmware_update_state, run.firmware_update_state);


#endif