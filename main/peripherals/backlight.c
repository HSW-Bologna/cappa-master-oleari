#include "driver/gpio.h"
#include "backlight.h"
#include "hardwareprofile.h"


void backlight_init(void) {
    gpio_config_t config = {
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = GPIO_MODE_OUTPUT,
        .pin_bit_mask = BIT64(HAP_RETRO),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&config));
}


void backlight_update(uint8_t value) {
    gpio_set_level(HAP_RETRO, value);
}