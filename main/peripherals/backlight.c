#include "driver/ledc.h"
#include "driver/gpio.h"
#include "backlight.h"
#include "hardwareprofile.h"
#include <esp_log.h>


#define BACKLIGHT_SPEED_MODE LEDC_LOW_SPEED_MODE
#define BACKLIGHT_TIMER      LEDC_TIMER_0
#define BACKLIGHT_CHANNEL    LEDC_CHANNEL_0


static const char *TAG = "Backlight";


void backlight_init(void) {

    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_8_BIT,         // resolution of PWM duty
        .freq_hz         = 2000,                     // frequency of PWM signal
        .speed_mode      = BACKLIGHT_SPEED_MODE,     // timer mode
        .timer_num       = BACKLIGHT_TIMER,          // timer index
        .clk_cfg         = LEDC_AUTO_CLK,            // Auto select the source clock
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .channel    = BACKLIGHT_CHANNEL,
        .duty       = 0,
        .gpio_num   = HAP_RETRO,
        .speed_mode = BACKLIGHT_SPEED_MODE,
        .hpoint     = 0,
        .timer_sel  = BACKLIGHT_TIMER,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    ledc_fade_func_install(0);
    backlight_update(100);
}


void backlight_update(uint8_t value) {
    value = value > 100 ? 100 : value;
    ESP_LOGI(TAG, "Update %i", value);
    uint32_t duty = (uint32_t)((((uint32_t)value) * 0xFF) / 100);
    ledc_set_duty(BACKLIGHT_SPEED_MODE, BACKLIGHT_CHANNEL, duty);
    ledc_update_duty(BACKLIGHT_SPEED_MODE, BACKLIGHT_CHANNEL);
}
