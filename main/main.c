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

#define BUF_SIZE (127)
#define BAUD_RATE 115200

// Read packet timeout
#define PACKET_READ_TICS (100 / portTICK_PERIOD_MS)
#define ECHO_TASK_STACK_SIZE (2048)
#define ECHO_TASK_PRIO (10)
#define ECHO_UART_PORT 1

// Timeout threshold for UART = number of symbols (~10 tics) with unchanged
// state on receive pin
#define ECHO_READ_TOUT (3) // 3.5T * 8 = 28 ticks, TOUT=3 -> ~24..33 ticks

static void echo_send(const int port, const char *str, uint8_t length) {
  if (uart_write_bytes(port, str, length) != length) {
    ESP_LOGE(TAG, "Send data critical failure.");
    // add your code to handle sending failure here
    abort();
  }
}

// An example of echo test with hardware flow control on UART
static void echo_task_master(void *arg) {

  // gpio_config_t gpioconfig = {BIT64(40), GPIO_MODE_OUTPUT,
  // GPIO_PULLUP_DISABLE,
  //                             GPIO_PULLDOWN_DISABLE, GPIO_PIN_INTR_DISABLE};
  // gpio_config(&gpioconfig);
  // gpio_set_level(GPIO_NUM_40, 1);
  //
  // const int uart_num = ECHO_UART_PORT;
  // uart_config_t uart_config = {
  //     .baud_rate = BAUD_RATE,
  //     .data_bits = UART_DATA_8_BITS,
  //     .parity = UART_PARITY_DISABLE,
  //     .stop_bits = UART_STOP_BITS_1,
  //     .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
  //     .rx_flow_ctrl_thresh = 122,
  //     .source_clk = UART_SCLK_DEFAULT,
  // };
  // // Configure UART parameters
  // ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
  //
  // // Set UART pins as per KConfig settings
  // ESP_ERROR_CHECK(uart_set_pin(uart_num, ECHO_TEST_TXD, ECHO_TEST_RXD,
  //                              ECHO_TEST_RTS, ECHO_TEST_CTS));
  //
  // // Install UART driver (we don't need an event queue here)
  // // In this example we don't even use a buffer for sending data.
  // ESP_ERROR_CHECK(
  //     uart_driver_install(uart_num, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, 0));
  //
  // // Set RS485 half duplex mode
  // ESP_ERROR_CHECK(uart_set_mode(uart_num, UART_MODE_RS485_HALF_DUPLEX));
  //
  rs485_init();

  // Set UART log level
  esp_log_level_set(TAG, ESP_LOG_INFO);

  ESP_LOGI(TAG, "Start RS485 application test and configure UART.");

  ESP_LOGI(TAG, "UART set pins, mode and install driver.");
  ESP_LOGI(TAG, "TXD %d", ECHO_TEST_TXD);

  // Set read timeout of UART TOUT feature
  // ESP_ERROR_CHECK(uart_set_rx_timeout(uart_num, ECHO_READ_TOUT));

  // Allocate buffers for UART
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

  ESP_LOGI(TAG, "UART start recieve loop.\r\n");
  // echo_send(uart_num, "[M] Start RS485 UART test.\r\n", 24);

  while (1) {
    memset(data, 0, BUF_SIZE);
    // echo_send(uart_num, "a", 1);
    rs485_sbus_write((uint8_t *)"c", 1);
    vTaskDelay(pdMS_TO_TICKS(1000));
    // Read data from UART
    // int len = uart_read_bytes(uart_num, data, BUF_SIZE, PACKET_READ_TICS);
    //
    // // Write data back to UART
    // if (len > 0) {
    //   printf("size %d - %s\n", len, data);

    //   echo_send(uart_num, "\r\n", 2);
    //   char prefix[] = "RS485 Received: [";
    //   echo_send(uart_num, prefix, (sizeof(prefix) - 1));
    //   ESP_LOGI(TAG, "Received %u bytes:", len);
    //   printf("[ ");
    //   for (int i = 0; i < len; i++) {
    //     printf("0x%.2X ", (uint8_t)data[i]);
    //     echo_send(uart_num, (const char *)&data[i], 1);
    //     // Add a Newline character if you get a return charater from paste
    //     // (Paste tests multibyte receipt/buffer)
    //     if (data[i] == '\r') {
    //       echo_send(uart_num, "\n", 1);
    //     }
    // }
    //   printf("] \n");
    //   echo_send(uart_num, "]\r\n", 3);
    // } else {
    // Echo a "." to show we are alive while we wait for input
    // ESP_ERROR_CHECK(uart_wait_tx_done(uart_num, 10));
    // }
  }
  vTaskDelete(NULL);
}

void app_main(void) {
  model_t model;

  lvgl_i2c_init(I2C_NUM_0);
  lvgl_driver_init();

  model_init(&model);
  view_init(&model, disp_driver_flush, gt911_read);
  controller_init(&model);

  xTaskCreate(echo_task_master, "uart_echo_task", ECHO_TASK_STACK_SIZE, NULL,
              ECHO_TASK_PRIO, NULL);

  ESP_LOGI(TAG, "Begin main loop");
  for (;;) {
    controller_gui_manage(&model);
    controller_manage(&model);

    vTaskDelay(pdMS_TO_TICKS(5));
  }
}
