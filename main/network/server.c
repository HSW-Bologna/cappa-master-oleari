#include <unistd.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <cJSON.h>
#include <esp_ota_ops.h>
#include "utils/utils.h"
#include "server.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "model/model.h"
#include "config/app_config.h"


static esp_err_t firmware_update_put_handler(httpd_req_t *req);
static esp_err_t home_get_handler(httpd_req_t *req);
static void      set_firmware_update_state(firmware_update_state_t state);


static const char *TAG               = "Server";
static const char *MEMORY_ERR_STRING = "{\"error\":\"Could not allocate memory!\", \"code\":1}";
static const char *INVALID_OTA       = "{\"error\":\"Invalid ota image!\", \"code\":4}";


static httpd_handle_t server = NULL;

static SemaphoreHandle_t sem = NULL;

static firmware_update_state_t firmware_update = FIRMWARE_UPDATE_STATE_NONE;


void server_init(void) {
    static StaticSemaphore_t mutex_buffer;
    sem = xSemaphoreCreateMutexStatic(&mutex_buffer);
}


firmware_update_state_t server_firmware_update_state(void) {
    xSemaphoreTake(sem, portMAX_DELAY);
    firmware_update_state_t res = firmware_update;
    xSemaphoreGive(sem);
    return res;
}


void server_start(void) {
    if (server != NULL) {
        return;
    }

    httpd_config_t config   = HTTPD_DEFAULT_CONFIG();
    config.task_priority    = 1;
    config.stack_size       = APP_CONFIG_TASK_SIZE * 10;
    config.lru_purge_enable = true;
    config.max_uri_handlers = 4;

    /* Start the httpd server */
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    esp_err_t res = httpd_start(&server, &config);
    if (res == ESP_OK) {

        // PUT /firmware_update
        const httpd_uri_t system_firmware_update = {
            .uri     = (const char *)"/firmware_update",
            .method  = HTTP_PUT,
            .handler = firmware_update_put_handler,
        };

        const httpd_uri_t home = {
            .uri     = (const char *)"/",
            .method  = HTTP_GET,
            .handler = home_get_handler,
        };

        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &system_firmware_update);
        httpd_register_uri_handler(server, &home);

    } else {
        ESP_LOGW(TAG, "Error starting server (0x%03X)!", res);
    }
}


void server_stop(void) {
    /* Stop the httpd server */
    if (server != NULL) {
        httpd_stop(server);
        server = NULL;
    }
}


static esp_err_t home_get_handler(httpd_req_t *req) {
    extern const unsigned char index_html_start[] asm("_binary_index_html_gzip_start");
    extern const unsigned char index_html_end[] asm("_binary_index_html_gzip_end");
    const size_t               index_html_size = (index_html_end - index_html_start);

    httpd_resp_set_hdr(req, "Content-encoding", "gzip");
    httpd_resp_send(req, (const char *)index_html_start, index_html_size);

    return ESP_OK;
}


static esp_err_t firmware_update_put_handler(httpd_req_t *req) {
#define BUFFER_SIZE 1024

    const esp_partition_t *update_partition = NULL;
    esp_ota_handle_t       update_handle;
    esp_err_t              err = ESP_OK;
    int                    ret;

    set_firmware_update_state(FIRMWARE_UPDATE_STATE_UPDATING);
    vTaskDelay(pdMS_TO_TICKS(1200));     // Allow time for the application to display the update page

    update_partition = esp_ota_get_next_update_partition(NULL);
    if (update_partition == NULL) {
        ESP_LOGE(TAG, "esp_ota_get_next_update_partition failed (0x%04X)!", err);
        httpd_resp_send_500(req);
        set_firmware_update_state(FIRMWARE_UPDATE_STATE_FAILURE);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x, request size %zu", update_partition->subtype,
             update_partition->address, req->content_len);

    // The following call takes about 1000ms
    if ((err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle) != ESP_OK)) {
        ESP_LOGE(TAG, "esp_ota_begin failed (0x%04X)!", err);
        httpd_resp_send_500(req);
        set_firmware_update_state(FIRMWARE_UPDATE_STATE_FAILURE);
        return ESP_FAIL;
    }

    char *buffer = malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, MEMORY_ERR_STRING);
        set_firmware_update_state(FIRMWARE_UPDATE_STATE_FAILURE);
        return ESP_FAIL;
    }

    size_t total    = 0;
    size_t attempts = 0;
    while (total < req->content_len) {
        ret = httpd_req_recv(req, buffer, BUFFER_SIZE);

        if (ret == 0) {
            ESP_LOGI(TAG, "Received nothing, continue...");
            if (attempts++ > 10) {
                break;
            }
        } else if (ret > 0) {
            ESP_LOGI(TAG, "Reading %i bytes into flash...", ret);
            total += ret;
            attempts = 0;

            if ((err = esp_ota_write(update_handle, buffer, ret)) != ESP_OK) {
                ESP_LOGE(TAG, "esp_ota_write failed (0x%04X)!", err);
                free(buffer);
                esp_ota_abort(update_handle);
                httpd_resp_send_500(req);
                set_firmware_update_state(FIRMWARE_UPDATE_STATE_FAILURE);
                return ESP_FAIL;
            } else {
                vTaskDelay(1);     // Delay for WDT
            }
        } else {
            ESP_LOGW(TAG, "Error while receiving ota: %i", ret);
            free(buffer);
            esp_ota_abort(update_handle);
            httpd_resp_send_500(req);
            set_firmware_update_state(FIRMWARE_UPDATE_STATE_FAILURE);
            return ESP_FAIL;
        }
    }
    free(buffer);

    if ((err = esp_ota_end(update_handle)) != ESP_OK) {
        // Invalid image
        ESP_LOGW(TAG, "Invalid OTA image (0x%X)", err);
        esp_ota_abort(update_handle);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, INVALID_OTA);
        set_firmware_update_state(FIRMWARE_UPDATE_STATE_FAILURE);
        return ESP_FAIL;
    }

    if ((err = esp_ota_set_boot_partition(update_partition) != ESP_OK)) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (0x%04X)!", err);
        httpd_resp_send_500(req);
        set_firmware_update_state(FIRMWARE_UPDATE_STATE_FAILURE);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Update successful! Reset the device.");
    httpd_resp_send(req, "", HTTPD_RESP_USE_STRLEN);
    httpd_resp_set_hdr(req, "Connection", "close");
    set_firmware_update_state(FIRMWARE_UPDATE_STATE_SUCCESS);
    return ESP_OK;
}


static void set_firmware_update_state(firmware_update_state_t state) {
    xSemaphoreTake(sem, portMAX_DELAY);
    firmware_update = state;
    xSemaphoreGive(sem);
}