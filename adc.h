#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void ADC_init(void);
uint8_t ADC_read(uint8_t ch);

#endif
