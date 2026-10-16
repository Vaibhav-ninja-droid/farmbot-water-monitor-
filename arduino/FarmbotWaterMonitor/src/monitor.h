#ifndef MONITOR_H
#define MONITOR_H
#include <stdbool.h>
#include <stdint.h>
#define MONITOR_WINDOW 5
/* These profiles describe pulse encoding, not mounting suitability. */
enum sensor_profile { SENSOR_MB1000, SENSOR_HRXL };
enum filter_mode { FILTER_RAW, FILTER_MEAN, FILTER_MEDIAN };
enum monitor_validity { MON_UNKNOWN, MON_VALID, MON_INVALID, MON_STALE, MON_UNCONFIGURED };
enum monitor_reason { REASON_STARTUP, REASON_READY, REASON_LOW, REASON_RECOVERING,
    REASON_RANGE, REASON_PULSE, REASON_TIMEOUT, REASON_CONFIG, REASON_CAPTURE };
struct monitor_config {
    enum sensor_profile sensor;
    enum filter_mode filter;
    bool calibrated;
    uint32_t min_distance_mm, max_distance_mm;
    int32_t offset_mm;
    uint32_t sensor_height_mm, max_depth_mm;
    uint32_t trip_depth_mm, restart_depth_mm;
    uint32_t timeout_ms, recovery_samples;
};
struct monitor {
    struct monitor_config config;
    uint32_t samples[MONITOR_WINDOW], count, index, recovery_count;
    uint64_t last_valid_ms;
    bool have_valid, allowed, config_ok;
    uint32_t pulse_us, raw_mm, filtered_mm, depth_mm, level_percent;
    enum monitor_validity validity;
    enum monitor_reason reason;
};

#ifdef __cplusplus
extern "C" {
#endif
bool monitor_config_valid(const struct monitor_config *c);
bool monitor_distance(enum sensor_profile profile, uint32_t pulse_us, int32_t offset, uint32_t *mm);
void monitor_init(struct monitor *m, const struct monitor_config *c);
void monitor_distance_sample(struct monitor *m, uint32_t distance_mm, uint64_t sample_ms, uint64_t now_ms);
void monitor_sample(struct monitor *m, uint32_t pulse_us, uint64_t sample_ms, uint64_t now_ms);
void monitor_fault(struct monitor *m, enum monitor_reason reason);
void monitor_tick(struct monitor *m, uint64_t now_ms);
const char *monitor_validity_name(enum monitor_validity v);
const char *monitor_reason_name(enum monitor_reason r);
#ifdef __cplusplus
}
#endif
#endif
