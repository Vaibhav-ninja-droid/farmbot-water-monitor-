#ifndef FILTER_H
#define FILTER_H

#include <stdint.h>

void filter_init(void);
uint32_t filter_add_sample(uint32_t sample);

#endif