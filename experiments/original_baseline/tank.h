#ifndef TANK_H
#define TANK_H

#include <stdint.h>

enum tank_state {
    TANK_EMPTY,
    TANK_LOW,
    TANK_OK,
    TANK_FULL,
    TANK_ERROR
};

struct tank_measurement {
    uint32_t sensor_distance_mm;
    uint32_t water_depth_mm;
    uint8_t level_percent;
    enum tank_state state;
};

struct tank_measurement tank_process(uint32_t distance_mm);

const char *tank_state_string(enum tank_state state);

#endif