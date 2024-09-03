#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

void led_blink(void);
void set_to_zero(uint8_t* buf);
int count_channels(uint8_t bitmask);

#endif 