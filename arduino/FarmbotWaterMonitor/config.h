#ifndef WATER_CONFIG_H
#define WATER_CONFIG_H
#include "src/monitor.h"
// 1 = MB7389 TTL serial pin 5 -> D8 (default, matches photo's wired output).
// 0 = pulse width pin 2 -> D2 (required for MB1000 comparison).
#ifndef WATER_USE_HRXL_SERIAL
#define WATER_USE_HRXL_SERIAL 1
#endif
static const uint8_t SENSOR_SERIAL_RX_PIN = 8;
static const uint8_t SENSOR_SERIAL_UNUSED_TX_PIN = 9; // Leave D9 disconnected.
// Confirmed sensor label: MB7389, 30mm1.5 housing.
static const uint8_t SENSOR_PW_PIN = 2;
static const uint8_t PERMISSION_LED_PIN = 13; // Built-in LED, HIGH = allowed.
static const uint32_t SERIAL_BAUD = 115200;
static const uint32_t HEARTBEAT_MS = 250;
// No pump/relay output. Geometry is unknown; permission stays blocked.
static inline monitor_config water_config() {
    monitor_config c = {};
    c.sensor = SENSOR_HRXL; // For MB1000: SENSOR_MB1000 and WATER_USE_HRXL_SERIAL=0.
    c.filter = FILTER_MEDIAN; // FILTER_RAW or FILTER_MEAN for comparisons.
    c.calibrated = false; // Set true only after replacing and checking dimensions.
    c.min_distance_mm = 300; // MB7389 minimum reported range; use >500 mm clearance if practical.
    c.max_distance_mm = 4999; // 5000 is MB7389 no-target code, not a valid distance.
    c.offset_mm = 0;
    c.sensor_height_mm = 0;
    c.max_depth_mm = 0;
    c.trip_depth_mm = 0;
    c.restart_depth_mm = 0;
    c.high_depth_mm = 0; // Set through dashboard calibration.
    c.timeout_ms = 1000;
    c.recovery_samples = 5;
    return c;
}
#endif
