#ifndef BACKLIGHT_H_INCLUDED
#define BACKLIGHT_H_INCLUDED


#include <stdint.h>


void backlight_init(void);
void backlight_update(uint8_t value);


#endif