#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "lightmodbus/lightmodbus.h"
#include "modbus.h"
#include "esp_log.h"
#include "peripherals/rs485.h"


#define MODBUS_RESPONSE_03_LEN(data_len) (5 + data_len * 2)
#define MODBUS_RESPONSE_05_LEN           8
#define MODBUS_MESSAGE_QUEUE_SIZE        32
#define MODBUS_TIMEOUT                   25
#define MODBUS_MAX_PACKET_SIZE           256
#define MODBUS_COMMUNICATION_ATTEMPTS    5

#define HOLDING_REGISTER_FAN   0
#define HOLDING_REGISTER_LIGHT 2

#define MINION_1_ADDR 1
#define MINION_2_ADDR 2


typedef enum {
    TASK_MESSAGE_TAG_SET_SPEED,
    TASK_MESSAGE_TAG_SET_LIGHT,
} task_message_tag_t;


struct __attribute__((packed)) task_message {
    task_message_tag_t tag;
    union {
        struct {
            uint16_t fan;
            uint16_t speed;
        };
        struct {
            uint16_t light;
            uint8_t  value;
        };
    };
};


static void        modbus_task(void *args);
static ModbusError exception_callback(const ModbusMaster *master, uint8_t address, uint8_t function,
                                      ModbusExceptionCode code);
static ModbusError data_callback(const ModbusMaster *master, const ModbusDataCallbackArgs *args);
static int write_holding_registers(ModbusMaster *master, uint8_t address, uint16_t starting_address, uint16_t *data,
                                   size_t num);


static const char   *TAG       = "Modbus";
static QueueHandle_t messageq  = NULL;
static QueueHandle_t responseq = NULL;


void modbus_init(void) {
    static StaticQueue_t static_queue1;
    static uint8_t       queue_buffer1[MODBUS_MESSAGE_QUEUE_SIZE * sizeof(struct task_message)] = {0};
    messageq =
        xQueueCreateStatic(MODBUS_MESSAGE_QUEUE_SIZE, sizeof(struct task_message), queue_buffer1, &static_queue1);

    static StaticQueue_t static_queue2;
    static uint8_t       queue_buffer2[MODBUS_MESSAGE_QUEUE_SIZE * sizeof(modbus_response_t)] = {0};
    responseq = xQueueCreateStatic(MODBUS_MESSAGE_QUEUE_SIZE, sizeof(modbus_response_t), queue_buffer2, &static_queue2);

    xTaskCreate(modbus_task, TAG, 512 * 6, NULL, 5, NULL);
}


void modbus_set_speed(uint16_t fan, uint16_t speed) {
    struct task_message msg = {.tag = TASK_MESSAGE_TAG_SET_SPEED, .fan = fan, .speed = speed};
    xQueueSend(messageq, &msg, portMAX_DELAY);
}


void modbus_set_light(uint16_t light, uint8_t value) {
    struct task_message msg = {.tag = TASK_MESSAGE_TAG_SET_LIGHT, .light = light, .value = value};
    xQueueSend(messageq, &msg, portMAX_DELAY);
}


uint8_t modbus_get_response(modbus_response_t *response) {
    return xQueueReceive(responseq, response, 0);
}


static void modbus_task(void *args) {
    (void)args;
    ModbusMaster    master;
    ModbusErrorInfo err = modbusMasterInit(&master,
                                           data_callback,              // Callback for handling incoming data
                                           exception_callback,         // Exception callback (optional)
                                           modbusDefaultAllocator,     // Memory allocator used to allocate request
                                           modbusMasterDefaultFunctions,        // Set of supported functions
                                           modbusMasterDefaultFunctionCount     // Number of supported functions
    );

    // Check for errors
    assert(modbusIsOk(err) && "modbusMasterInit() failed");
    struct task_message message = {0};

    ESP_LOGI(TAG, "Task starting");

    for (;;) {
        if (xQueueReceive(messageq, &message, pdMS_TO_TICKS(100))) {
            switch (message.tag) {
                case TASK_MESSAGE_TAG_SET_SPEED: {
                    modbus_response_t response = MODBUS_RESPONSE_OK;
                    uint16_t          values[] = {message.speed};

                    ESP_LOGI(TAG, "Setting fan speed for %i to %i", message.fan, message.speed);

                    uint8_t  address = 0;
                    uint16_t fan     = 0;

                    if (message.fan < 2) {
                        address = MINION_1_ADDR;
                        fan     = message.fan;
                    } else {
                        address = MINION_2_ADDR;
                        fan     = (message.fan) % 2;
                    }

                    if (write_holding_registers(&master, address, HOLDING_REGISTER_FAN + fan, values, 1)) {
                        response = MODBUS_RESPONSE_ERROR;
                    }
                    xQueueSend(responseq, &response, portMAX_DELAY);
                    break;
                }

                case TASK_MESSAGE_TAG_SET_LIGHT: {
                    modbus_response_t response = MODBUS_RESPONSE_OK;
                    uint16_t          values[] = {message.value};

                    uint8_t address = MINION_2_ADDR;

                    if (write_holding_registers(&master, address, HOLDING_REGISTER_LIGHT + message.light, values, 1)) {
                        response = MODBUS_RESPONSE_ERROR;
                    }
                    xQueueSend(responseq, &response, portMAX_DELAY);
                    break;
                }
            }
            vTaskDelay(pdMS_TO_TICKS(MODBUS_TIMEOUT / 2));
        }
    }

    vTaskDelete(NULL);
}


static ModbusError data_callback(const ModbusMaster *master, const ModbusDataCallbackArgs *args) {
    // master_context_t *ctx = modbusMasterGetUserPointer(master);

    return MODBUS_OK;
}


static ModbusError exception_callback(const ModbusMaster *master, uint8_t address, uint8_t function,
                                      ModbusExceptionCode code) {
    ESP_LOGI(TAG, "Received exception (function %d) from slave %d code %d", function, address, code);

    return MODBUS_OK;
}


static int write_holding_registers(ModbusMaster *master, uint8_t address, uint16_t starting_address, uint16_t *data,
                                   size_t num) {
    uint8_t buffer[MODBUS_MAX_PACKET_SIZE] = {0};
    int     res                            = 0;
    size_t  counter                        = 0;

    rs485_flush();

    do {
        res                 = 0;
        ModbusErrorInfo err = modbusBuildRequest16RTU(master, address, starting_address, num, data);
        assert(modbusIsOk(err));
        rs485_sbus_write((uint8_t *)modbusMasterGetRequest(master), modbusMasterGetRequestLength(master));

        int len = rs485_read(buffer, sizeof(buffer), pdMS_TO_TICKS(MODBUS_TIMEOUT));
        err     = modbusParseResponseRTU(master, modbusMasterGetRequest(master), modbusMasterGetRequestLength(master),
                                         buffer, len);

        if (!modbusIsOk(err)) {
            ESP_LOGW(TAG, "Write holding registers for %i error (%i): %i %i", address, len, err.source, err.error);
            res = 1;
            vTaskDelay(pdMS_TO_TICKS(MODBUS_TIMEOUT));
        }
    } while (res && ++counter < MODBUS_COMMUNICATION_ATTEMPTS);

    return res;
}
