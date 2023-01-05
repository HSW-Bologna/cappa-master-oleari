#include "driver/i2c.h"
#include "esp_log.h"
#include "hardwareprofile.h"
#include <esp_random.h>
#include <bootloader_random.h>
#include <esp_system.h>
#include <lvgl.h>

static const char *TAG = "System";


void system_random_init(void) {
    (void)TAG;
    /* Initialize RNG address */
    bootloader_random_enable();
    srand(esp_random());
    bootloader_random_disable();
}


void system_reset(void) {
    esp_restart();
}


uint8_t system_psram_enabled(void) {
    return heap_caps_get_total_size(MALLOC_CAP_SPIRAM) > 0;
}


void system_print_heap_status(void) {
    printf("[%s] - Internal RAM: LWM = %u, free = %u, biggest = %u\n", TAG,
           heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL), heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
           heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    printf("[%s] - PSRAM       : LWM = %u, free = %u\n", TAG, heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM),
           heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    if (lv_is_initialized()) {
        lv_mem_monitor_t monitor = {0};
        lv_mem_monitor(&monitor);
        printf("[%s] - LVGL        : free = %u, frag = %u%%\n", TAG, monitor.free_size, monitor.frag_pct);
    }
}