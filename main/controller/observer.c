#include <assert.h>
#include "configuration.h"
#include "gel/data_structures/watcher.h"
#include "peripherals/storage.h"
#include "utils/utils.h"


#define NUM_OBSERVED_VARIABLES (4)


static watcher_t watchlist[NUM_OBSERVED_VARIABLES + 1] = {0};


void observer_init(model_t *pmodel) {
    size_t i = 0;

    watchlist[i++] =
        WATCHER_DELAYED(&pmodel->configuration.minimum_speed, storage_save_uint16, CONFIGURATION_MIN_SPEED_KEY, 4000UL);
    watchlist[i++] = WATCHER_DELAYED(&pmodel->configuration.immission_percentages[0], storage_save_uint16,
                                     CONFIGURATION_IMM_PERC_1_KEY, 4000UL);
    watchlist[i++] = WATCHER_DELAYED(&pmodel->configuration.immission_percentages[1], storage_save_uint16,
                                     CONFIGURATION_IMM_PERC_2_KEY, 4000UL);
    watchlist[i++] = WATCHER_DELAYED(&pmodel->configuration.immission_percentages[2], storage_save_uint16,
                                     CONFIGURATION_IMM_PERC_3_KEY, 4000UL);

    assert(NUM_OBSERVED_VARIABLES == i);
    watchlist[i++] = WATCHER_NULL;

    watcher_list_init(watchlist);
}


void observer_observe(model_t *pmodel) {
    watcher_process_changes(watchlist, get_millis());
}
