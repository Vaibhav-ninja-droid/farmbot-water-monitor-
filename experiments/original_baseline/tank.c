#include "tank.h"

/*
 * IMPORTANT:
 * These are temporary example values.
 *
 * You must replace these after measuring
 * the actual FarmBot tank.
 */

#define SENSOR_TO_BOTTOM_MM 1000U

#define LOW_THRESHOLD_PERCENT   20U
#define FULL_THRESHOLD_PERCENT  90U

struct tank_measurement tank_process(uint32_t distance_mm)
{
    struct tank_measurement result;

    result.sensor_distance_mm = distance_mm;

    /*
     * Invalid reading:
     * water surface cannot physically be
     * farther away than the tank bottom.
     */
    if (distance_mm > SENSOR_TO_BOTTOM_MM) {
        result.water_depth_mm = 0;
        result.level_percent = 0;
        result.state = TANK_ERROR;

        return result;
    }

    result.water_depth_mm =
        SENSOR_TO_BOTTOM_MM - distance_mm;

    result.level_percent =
        (result.water_depth_mm * 100U)
        / SENSOR_TO_BOTTOM_MM;

    if (result.level_percent <= 5U) {
        result.state = TANK_EMPTY;
    }
    else if (result.level_percent <
             LOW_THRESHOLD_PERCENT) {
        result.state = TANK_LOW;
    }
    else if (result.level_percent >=
             FULL_THRESHOLD_PERCENT) {
        result.state = TANK_FULL;
    }
    else {
        result.state = TANK_OK;
    }

    return result;
}

const char *tank_state_string(enum tank_state state)
{
    switch (state) {
    case TANK_EMPTY:
        return "EMPTY";

    case TANK_LOW:
        return "LOW";

    case TANK_OK:
        return "OK";

    case TANK_FULL:
        return "FULL";

    case TANK_ERROR:
    default:
        return "ERROR";
    }
}