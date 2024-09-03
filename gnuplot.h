#ifndef GNUPLOT_H
#define GNUPLOT_H

#include <stdint.h>

void gnuplot_start(uint8_t bitmask);
int count_channels(uint8_t bitmask);

#endif 