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
#include "rom/gpio.h"
#include "sdkconfig.h"
#include "view/view.h"
#include <driver/i2c.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * This is a example which echos any data it receives on UART back to the sender
 * using RS485 interface in half duplex mode.
 */
#define TAG "RS485_ECHO_APP"

#define UART_SCLK_DEFAULT 0

#define ECHO_TEST_TXD 41
#define ECHO_TEST_RXD 42

// RTS for RS485 Half-Duplex Mode manages DE/~RE
// #define ECHO_TEST_RTS 40
#define ECHO_TEST_RTS -1

// CTS is not used in RS485 Half-Duplex Mode
#define ECHO_TEST_CTS -1

#define BUF_SIZE  (127)
#define BAUD_RATE 115200

// Read packet timeout
#define PACKET_READ_TICS     (100 / portTICK_PERIOD_MS)
#define ECHO_TASK_STACK_SIZE (2048)
#define ECHO_TASK_PRIO       (10)
#define ECHO_UART_PORT       1

// Timeout threshold for UART = number of symbols (~10 tics) with unchanged
// state on receive pin
#define ECHO_READ_TOUT (3)     // 3.5T * 8 = 28 ticks, TOUT=3 -> ~24..33 ticks

static void echo_send(const int port, const char *str, uint8_t length) {
    if (uart_write_bytes(port, str, length) != length) {
        ESP_LOGE(TAG, "Send data critical failure.");
        // add your code to handle sending failure here
        abort();
    }
}


void app_main(void) {
    model_t model;

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

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
