// MASTER

#include "controller/controller.h"
#include "controller/gui.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "hal/gpio_types.h"
#include "lvgl_helpers.h"
#include "lvgl_i2c/i2c_manager.h"
#include "model/model.h"
#include "nvs_flash.h"
#include "peripherals/phase_cut.h"
#include "peripherals/rs485.h"
#include "peripherals/buzzer.h"
#include "peripherals/storage.h"
#include "peripherals/backlight.h"
#include "rom/gpio.h"
#include "sdkconfig.h"
#include "view/view.h"
#include <driver/i2c.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config/app_config.h"


static const char *TAG = "Main";


const __attribute__((section(".rodata_custom_desc"))) struct {
    char     project_name[16];
    uint16_t project_version_major;
    uint16_t project_version_minor;
    uint16_t project_version_patch;
} custom_app_desc = {
    .project_name          = APP_CONFIG_FIRMWARE_NAME,
    .project_version_major = APP_CONFIG_FIRMWARE_VERSION_MAJOR,
    .project_version_minor = APP_CONFIG_FIRMWARE_VERSION_MINOR,
    .project_version_patch = APP_CONFIG_FIRMWARE_VERSION_PATCH,
};


void app_main(void) {
    model_t model;

    backlight_init();
    storage_init();
    buzzer_init();
    rs485_init();
    lvgl_i2c_init(I2C_NUM_0);
    lvgl_driver_init();

    model_init(&model);
    view_init(&model, disp_driver_flush, gt911_read);
    controller_init(&model);

    ESP_LOGI(TAG, "Begin main loop");
    for (;;) {
        controller_gui_manage(&model);
        controller_manage(&model);

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
