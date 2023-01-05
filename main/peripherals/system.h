#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

#include <stdint.h>


void    system_random_init(void);
void    system_reset(void);
uint8_t system_psram_enabled(void);
void    system_print_heap_status(void);


#endif