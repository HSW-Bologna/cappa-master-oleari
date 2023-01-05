#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "config/app_config.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "server.h"


static const char *TAG = "Network";

static const uint32_t EVENT_AP_STARTED = 0x01;

static EventGroupHandle_t event_group = NULL;


static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_START) {
        ESP_LOGI(TAG, "Starting server");
        server_start();
        xEventGroupSetBits(event_group, EVENT_AP_STARTED);
    } else if (event_id == WIFI_EVENT_AP_STOP) {
        ESP_LOGI(TAG, "Stopping server");
        server_stop();
        xEventGroupClearBits(event_group, EVENT_AP_STARTED);
    }
}


void network_init(void) {
    assert(event_group == NULL);
    event_group = xEventGroupCreate();

    server_init();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
}


void network_start_ap(void) {
    wifi_config_t wifi_config = {
        .ap = {.ssid           = APP_CONFIG_WIFI_SSID,
               .ssid_len       = strlen(APP_CONFIG_WIFI_SSID),
               .channel        = 0,
               .password       = APP_CONFIG_WIFI_PASS,
               .max_connection = 4,
               .authmode       = WIFI_AUTH_WPA_WPA2_PSK},
    };
    if (strlen(APP_CONFIG_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished");
}


uint8_t network_is_ap_running(void) {
    return (xEventGroupGetBits(event_group) & EVENT_AP_STARTED) > 0;
}