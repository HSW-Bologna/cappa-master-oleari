#ifndef RS485_H_INCLUDED
#define RS485_H_INCLUDED

#include <stdint.h>
#include <stdlib.h>

void rs485_get_control(void);
void rs485_init(void);
void rs485_sbus_write(uint8_t *buffer, size_t len);
int rs485_read(uint8_t *buffer, size_t len, unsigned long timeout_ms);
void rs485_set_eol(int eol);
void rs485_flush(void);
void rs485_flush_input(void);

#endif
