#ifndef MODBUS_H_INCLUDED
#define MODBUS_H_INCLUDED


typedef enum {
    MODBUS_RESPONSE_OK,
    MODBUS_RESPONSE_ERROR,
} modbus_response_t;


void    modbus_init(void);
void    modbus_set_speed(uint16_t fan, uint16_t speed);
void    modbus_set_light(uint16_t light, uint8_t value);
uint8_t modbus_get_response(modbus_response_t *response);


#endif