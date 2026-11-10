#include "filter.h"

#define FILTER_SIZE 5

static uint32_t samples[FILTER_SIZE];
static uint32_t index_pos;
static uint32_t sample_count;

void filter_init(void) {
    index_pos = 0;
    sample_count = 0;

    for (int i = 0; i < FILTER_SIZE; i++) {
        samples[i] = 0;
    }
}

uint32_t filter_add_sample(uint32_t sample) {
    samples[index_pos] = sample;

    index_pos = (index_pos + 1) % FILTER_SIZE;

    if (sample_count < FILTER_SIZE) {
        sample_count++;
    }

    uint64_t total = 0;

    for (uint32_t i = 0; i < sample_count; i++) {
        total += samples[i];
    }

    return (uint32_t)(total / sample_count);
}