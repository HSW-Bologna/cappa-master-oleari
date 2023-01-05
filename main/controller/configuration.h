#ifndef CONFIGURATION_H_INCLUDED
#define CONFIGURATION_H_INCLUDED


#include "model/model.h"


void configuration_load(model_t *pmodel);


extern const char *CONFIGURATION_MIN_SPEED_KEY;
extern const char *CONFIGURATION_IMM_PERC_1_KEY;
extern const char *CONFIGURATION_IMM_PERC_2_KEY;
extern const char *CONFIGURATION_IMM_PERC_3_KEY;


#endif