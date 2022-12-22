#include "rs485.h"
#include "config/app_config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "hardwareprofile.h"
#include <driver/gpio.h>
#include <driver/uart.h>
#include <string.h>

#define HAP_R_485 21
#define HAP_SEL_485 39
#define HAP_D_ESP 40
#define HAP_TX_ESP 41
#define HAP_RX_ESP 42
#define PORTNUM UART_NUM_1
#define ECHO_READ_TOUT (3) // 3.5T * 8 = 28 ticks, TOUT=3 -> ~24..33 ticks

static const char *TAG = "RS485";

void rs485_init(void) {
  ESP_LOGI(TAG, "Initializing RS485...");

  gpio_config_t config = {
      .intr_type = GPIO_INTR_DISABLE,
      .mode = GPIO_MODE_OUTPUT,
      .pin_bit_mask = BIT64(HAP_D_ESP) | BIT64(HAP_SEL_485),
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .pull_up_en = GPIO_PULLUP_DISABLE,
  };
  ESP_ERROR_CHECK(gpio_config(&config));
  gpio_set_level(HAP_D_ESP, 0);
  gpio_set_level(HAP_R_485, 1);
  rs485_get_control();

  uart_config_t uart_config = {
      .baud_rate = 19200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 122,
  };

  // Configure UART parameters
  ESP_ERROR_CHECK(uart_param_config(PORTNUM, &uart_config));

  uart_set_pin(PORTNUM, HAP_TX_ESP, HAP_RX_ESP, -1, -1);
  ESP_ERROR_CHECK(uart_driver_install(PORTNUM, 512, 512, 16, NULL, 0));
  ESP_ERROR_CHECK(uart_set_mode(PORTNUM, UART_MODE_RS485_HALF_DUPLEX));
  ESP_ERROR_CHECK(uart_set_rx_timeout(PORTNUM, ECHO_READ_TOUT));
}

void rs485_set_eol(int eol) { gpio_set_level(HAP_R_485, eol == 0); }

void rs485_flush(void) { uart_flush(PORTNUM); }

void rs485_flush_input(void) { uart_flush_input(PORTNUM); }

void rs485_sbus_write(uint8_t *buffer, size_t len) {
  uart_parity_t current_parity = UART_PARITY_DISABLE;
  ESP_ERROR_CHECK(uart_get_parity(PORTNUM, &current_parity));
  gpio_set_level(HAP_D_ESP, 1);
  ets_delay_us(10);

  uart_write_bytes(PORTNUM, buffer, len);

  ets_delay_us(10);
  gpio_set_level(HAP_D_ESP, 0);
}

int rs485_read(uint8_t *buffer, size_t len, unsigned long timeout_ms) {
  return uart_read_bytes(PORTNUM, buffer, len, pdMS_TO_TICKS(timeout_ms));
}

void rs485_get_control(void) {
  ESP_ERROR_CHECK(gpio_set_level(HAP_SEL_485, 0));
}

void rs485_relinquish_control(void) {
  ESP_ERROR_CHECK(gpio_set_level(HAP_SEL_485, 0));
}
