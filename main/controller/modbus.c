#include <stdint.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "lightmodbus/lightmodbus.h"
#include "modbus.h"
#include "esp_log.h"
#include "peripherals/rs485.h"
#include "model/model.h"


#define MODBUS_RESPONSE_03_LEN(data_len) (5 + data_len * 2)
#define MODBUS_RESPONSE_05_LEN           8
#define MODBUS_MESSAGE_QUEUE_SIZE        32
#define MODBUS_TIMEOUT                   25
#define MODBUS_MAX_PACKET_SIZE           256
#define MODBUS_COMMUNICATION_ATTEMPTS    5

#define HOLDING_REGISTER_FAN                0
#define HOLDING_REGISTER_RELAYS             1
#define HOLDING_REGISTER_FIRMWARE_VERSION_1 2
#define HOLDING_REGISTER_FIRMWARE_VERSION_2 3
#define HOLDING_REGISTER_ADDRESS            65033

typedef enum {
    TASK_MESSAGE_TAG_SET_SPEED,
    TASK_MESSAGE_TAG_SET_LIGHT,
    TASK_MESSAGE_TAG_SET_ADDRESS,
    TASK_MESSAGE_TAG_READ_FW_VERSION,
} task_message_tag_t;


struct __attribute__((packed)) task_message {
    task_message_tag_t tag;
    union {
        uint8_t address;
        struct {
            uint16_t fan;
            uint16_t speed;
            uint16_t gas;
        };
        struct {
            uint16_t light;
            uint8_t  value;
        };
    };
};


typedef struct {
    uint16_t start;
    void    *pointer;
} master_context_t;


static void        modbus_task(void *args);
static ModbusError exception_callback(const ModbusMaster *master, uint8_t address, uint8_t function,
                                      ModbusExceptionCode code);
static ModbusError data_callback(const ModbusMaster *master, const ModbusDataCallbackArgs *args);
static int write_holding_registers(ModbusMaster *master, uint8_t address, uint16_t starting_address, uint16_t *data,
                                   size_t num);
static int read_holding_registers(ModbusMaster *master, uint16_t *registers, uint8_t address, uint16_t start,
                                  uint16_t count);


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


void modbus_set_speed(uint16_t fan, uint16_t speed, uint8_t gas) {
    struct task_message msg = {.tag = TASK_MESSAGE_TAG_SET_SPEED, .fan = fan, .speed = speed, .gas = gas};
    xQueueSend(messageq, &msg, portMAX_DELAY);
}


void modbus_set_light(uint16_t light, uint8_t value) {
    struct task_message msg = {.tag = TASK_MESSAGE_TAG_SET_LIGHT, .light = light, .value = value};
    xQueueSend(messageq, &msg, portMAX_DELAY);
}


void modbus_set_address(uint8_t address) {
    struct task_message msg = {.tag = TASK_MESSAGE_TAG_SET_ADDRESS, .address = address};
    xQueueSend(messageq, &msg, portMAX_DELAY);
}


void modbus_read_firmware_version(uint8_t address) {
    struct task_message msg = {.tag = TASK_MESSAGE_TAG_READ_FW_VERSION, .address = address};
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

    uint16_t relays[MAX_FANS] = {0};

    ESP_LOGI(TAG, "Task starting");

    for (;;) {
        if (xQueueReceive(messageq, &message, pdMS_TO_TICKS(100))) {
            switch (message.tag) {
                case TASK_MESSAGE_TAG_SET_SPEED: {
                    modbus_response_t response = {.tag = MODBUS_RESPONSE_TAG_OK, .error = 0};
                    uint8_t           address  = message.fan + 1;

                    uint8_t gas_relay = message.gas ? (message.speed > 0) : 0;
                    uint8_t relay     = 0;
                    if (message.fan < MAX_FANS) {
                        relays[message.fan] = (relays[message.fan] & (~0x02)) | (gas_relay ? 0x02 : 0x00);
                        relay               = relays[message.fan];
                    }

                    uint16_t values[] = {message.speed, relay};

                    ESP_LOGI(TAG, "Setting fan speed for %i to %i", message.fan, message.speed);

                    if (write_holding_registers(&master, address, HOLDING_REGISTER_FAN, values, 2)) {
                        response.error = 1;
                    }

                    xQueueSend(responseq, &response, portMAX_DELAY);
                    break;
                }

                case TASK_MESSAGE_TAG_SET_LIGHT: {
                    modbus_response_t response = {.tag = MODBUS_RESPONSE_TAG_OK, .error = 0};

                    relays[message.light] = (relays[message.light] & (~0x01)) | (message.value ? 0x01 : 0x00);
                    uint16_t values[]     = {relays[message.light]};

                    uint8_t address = message.light + 1;

                    if (write_holding_registers(&master, address, HOLDING_REGISTER_RELAYS, values, 1)) {
                        response.error = 1;
                    }
                    xQueueSend(responseq, &response, portMAX_DELAY);
                    break;
                }

                case TASK_MESSAGE_TAG_SET_ADDRESS: {
                    modbus_response_t response = {.tag = MODBUS_RESPONSE_TAG_OK, .error = 0};
                    uint16_t          values[] = {message.address};

                    if (write_holding_registers(&master, 0, HOLDING_REGISTER_ADDRESS, values, 1)) {
                        response.error = 1;
                    } else {
                        vTaskDelay(pdMS_TO_TICKS(MODBUS_TIMEOUT / 2));
                        uint16_t values[2] = {0};
                        if (read_holding_registers(&master, values, message.address,
                                                   HOLDING_REGISTER_FIRMWARE_VERSION_1, 2)) {
                            response.error = 1;
                        }
                    }

                    xQueueSend(responseq, &response, portMAX_DELAY);
                    break;
                }

                case TASK_MESSAGE_TAG_READ_FW_VERSION: {
                    modbus_response_t response = {.tag = MODBUS_RESPONSE_TAG_FIRMWARE_VERSION, .error = 0};
                    response.address           = message.address;

                    uint16_t values[2] = {0};
                    if (read_holding_registers(&master, values, message.address, HOLDING_REGISTER_FIRMWARE_VERSION_1,
                                               2)) {
                        response.error = 1;
                    } else {
                        response.version_major = (values[0] >> 8) & 0xFF;
                        response.version_minor = values[0] & 0xFF;
                        response.version_patch = values[1] & 0xFF;
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
    master_context_t *ctx = modbusMasterGetUserPointer(master);

    if (ctx != NULL) {
        switch (args->type) {
            case MODBUS_HOLDING_REGISTER: {
                uint16_t *buffer                 = ctx->pointer;
                buffer[args->index - ctx->start] = args->value;
                break;
            }

            case MODBUS_DISCRETE_INPUT: {
                uint8_t *buffer                  = ctx->pointer;
                buffer[args->index - ctx->start] = args->value;
                break;
            }

            case MODBUS_INPUT_REGISTER: {
                uint16_t *buffer                 = ctx->pointer;
                buffer[args->index - ctx->start] = args->value;
                break;
            }

            case MODBUS_COIL: {
                uint8_t *buffer                  = ctx->pointer;
                buffer[args->index - ctx->start] = args->value;
                break;
            }
        }
    }

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

    do {
        res                 = 0;
        ModbusErrorInfo err = modbusBuildRequest16RTU(master, address, starting_address, num, data);
        assert(modbusIsOk(err));
        rs485_flush();
        rs485_write((uint8_t *)modbusMasterGetRequest(master), modbusMasterGetRequestLength(master));

        int len = rs485_read(buffer, sizeof(buffer), pdMS_TO_TICKS(MODBUS_TIMEOUT));

        size_t starting_index = 0;
        if (len > 0) {
            if (buffer[starting_index] == 0) {
                len--;
                starting_index++;
            }
        }
        err = modbusParseResponseRTU(master, modbusMasterGetRequest(master), modbusMasterGetRequestLength(master),
                                     &buffer[starting_index], len);

        if (!modbusIsOk(err)) {
            ESP_LOGW(TAG, "Write holding registers for %i error (%i): %i %i", address, len, err.source, err.error);
            // ESP_LOG_BUFFER_HEX(TAG, (uint8_t *)buffer, len);

            res = 1;
            vTaskDelay(pdMS_TO_TICKS(MODBUS_TIMEOUT));
        }
    } while (res && ++counter < MODBUS_COMMUNICATION_ATTEMPTS);

    if (res) {
        ESP_LOGW(TAG, "ERROR!");
    } else {
        ESP_LOGI(TAG, "Success");
    }

    return res;
}


static int read_holding_registers(ModbusMaster *master, uint16_t *registers, uint8_t address, uint16_t start,
                                  uint16_t count) {
    ModbusErrorInfo err;
    int             res                            = 0;
    size_t          counter                        = 0;
    uint8_t         buffer[MODBUS_MAX_PACKET_SIZE] = {0};

    master_context_t ctx = {.pointer = registers, .start = start};
    if (registers == NULL) {
        modbusMasterSetUserPointer(master, NULL);
    } else {
        modbusMasterSetUserPointer(master, &ctx);
    }

    do {
        res = 0;
        err = modbusBuildRequest03RTU(master, address, start, count);
        assert(modbusIsOk(err));

        rs485_write((uint8_t *)modbusMasterGetRequest(master), modbusMasterGetRequestLength(master));

        int    len            = rs485_read(buffer, sizeof(buffer), pdMS_TO_TICKS(MODBUS_TIMEOUT));
        size_t starting_index = 0;
        if (len > 0) {
            if (buffer[starting_index] == 0) {
                len--;
                starting_index++;
            }
        }
        err = modbusParseResponseRTU(master, modbusMasterGetRequest(master), modbusMasterGetRequestLength(master),
                                     &buffer[starting_index], len);

        if (!modbusIsOk(err)) {
            ESP_LOGW(TAG, "Read holding registers for %i error (%i): %i %i", address, len, err.source, err.error);
            res = 1;
            vTaskDelay(pdMS_TO_TICKS(MODBUS_TIMEOUT));
        }
    } while (res && ++counter < MODBUS_COMMUNICATION_ATTEMPTS);

    return res;
}
