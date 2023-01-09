#ifndef MODBUS_H_INCLUDED
#define MODBUS_H_INCLUDED


typedef enum {
    MODBUS_RESPONSE_TAG_OK,
    MODBUS_RESPONSE_TAG_FIRMWARE_VERSION,
    MODBUS_RESPONSE_TAG_START_OTA,
} modbus_response_tag_t;


typedef struct {
    modbus_response_tag_t tag;
    uint16_t              device;
    uint8_t               error;
    union {
        struct {
            uint16_t version_major;
            uint16_t version_minor;
            uint16_t version_patch;
        };
    };
} modbus_response_t;


void    modbus_init(void);
void    modbus_set_speed(uint16_t fan, uint16_t speed);
void    modbus_set_light(uint16_t light, uint8_t value);
uint8_t modbus_get_response(modbus_response_t *response);
void    modbus_read_firmware_version(uint16_t device);
void    modbus_start_ota(uint16_t device);


#endif