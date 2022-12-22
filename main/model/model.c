#include <stdlib.h>
#include <assert.h>
#include "model.h"

void model_init(model_t *pmodel) {
    assert(pmodel != NULL);
    pmodel->configuration.language = 0;

    pmodel->run.fan_speeds[0] = 50;
    pmodel->run.fan_speeds[1] = 50;
    pmodel->run.fan_speeds[2] = 50;

    pmodel->run.fan_on[0] = 0;
    pmodel->run.fan_on[1] = 0;
    pmodel->run.fan_on[2] = 0;
}


uint16_t model_get_language(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->configuration.language;
}