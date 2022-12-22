#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include <stdint.h>


#define NUM_FANS 3

typedef struct {
    struct {
        uint16_t language;
    } configuration;

    struct {
        uint16_t fan_speeds[NUM_FANS];
        uint8_t  fan_on[NUM_FANS];
    } run;
} model_t;


void model_init(model_t *pmodel);

uint16_t model_get_language(model_t *pmodel);


#endif