#include "monitor.h"
#include <limits.h>
#include <string.h>
bool monitor_config_valid(const struct monitor_config *c)
{
    return c && c->calibrated && c->sensor <= SENSOR_HRXL && c->sensor >= SENSOR_MB1000 &&
        c->filter >= FILTER_RAW && c->filter <= FILTER_MEDIAN &&
        c->min_distance_mm > 0 && c->max_distance_mm > c->min_distance_mm &&
        c->sensor_height_mm <= c->max_distance_mm &&
        c->max_depth_mm > 0 && c->sensor_height_mm > c->max_depth_mm &&
        c->sensor_height_mm - c->max_depth_mm > c->min_distance_mm &&
        c->trip_depth_mm > 0 && c->trip_depth_mm < c->restart_depth_mm &&
        c->restart_depth_mm <= c->max_depth_mm && c->timeout_ms > 0 &&
        c->recovery_samples >= MONITOR_WINDOW && c->recovery_samples <= 1000;
}
bool monitor_distance(enum sensor_profile profile, uint32_t pulse_us, int32_t offset, uint32_t *mm)
{
    if (!mm || pulse_us == 0 || pulse_us > 100000 ||
        (profile != SENSOR_MB1000 && profile != SENSOR_HRXL)) return false;
    int64_t distance = profile == SENSOR_MB1000 ?
        ((uint64_t)pulse_us * 254 + 735) / 1470 : pulse_us;
    distance += offset;
    if (distance <= 0 || distance > UINT32_MAX) return false;
    *mm = (uint32_t)distance;
    return true;
}
void monitor_init(struct monitor *m, const struct monitor_config *c)
{
    memset(m, 0, sizeof(*m));
    m->config = *c;
    m->config_ok = monitor_config_valid(c);
    m->validity = m->config_ok ? MON_UNKNOWN : MON_UNCONFIGURED;
    m->reason = m->config_ok ? REASON_STARTUP : REASON_CONFIG;
}
void monitor_fault(struct monitor *m, enum monitor_reason reason)
{
    m->allowed = false;
    m->count = m->index = m->recovery_count = 0;
    m->validity = m->config_ok ? MON_INVALID : MON_UNCONFIGURED;
    m->reason = m->config_ok ? reason : REASON_CONFIG;
}
void monitor_tick(struct monitor *m, uint64_t now_ms)
{
    if (!m->config_ok) return;
    if (m->have_valid && (now_ms < m->last_valid_ms ||
        now_ms - m->last_valid_ms >= m->config.timeout_ms)) {
        monitor_fault(m, REASON_TIMEOUT);
        m->validity = MON_STALE;
    }
}
static uint32_t estimate(struct monitor *m, uint32_t value)
{
    m->samples[m->index] = value;
    m->index = (m->index + 1) % MONITOR_WINDOW;
    if (m->count < MONITOR_WINDOW) m->count++;
    if (m->config.filter == FILTER_RAW) return value;
    uint32_t sorted[MONITOR_WINDOW];
    uint64_t total = 0;
    for (uint32_t i = 0; i < m->count; i++) {
        sorted[i] = m->samples[i]; total += sorted[i];
    }
    if (m->config.filter == FILTER_MEAN) return (uint32_t)(total / m->count);
    for (uint32_t i = 1; i < m->count; i++) {
        uint32_t v = sorted[i], j = i;
        while (j > 0 && sorted[j - 1] > v) { sorted[j] = sorted[j - 1]; j--; }
        sorted[j] = v;
    }
    return sorted[m->count / 2];
}
static void accept_distance(struct monitor *m, uint32_t distance_mm, uint64_t sample_ms, uint64_t now_ms);

void monitor_distance_sample(struct monitor *m, uint32_t distance_mm, uint64_t sample_ms, uint64_t now_ms)
{
    monitor_tick(m, now_ms);
    m->pulse_us = 0; // Serial input has no measured pulse width.
    m->raw_mm = distance_mm;
    int64_t corrected = (int64_t)distance_mm + m->config.offset_mm;
    if (distance_mm == 0 || corrected <= 0 || corrected > UINT32_MAX) {
        monitor_fault(m, REASON_RANGE); return;
    }
    accept_distance(m, (uint32_t)corrected, sample_ms, now_ms);
}

void monitor_sample(struct monitor *m, uint32_t pulse_us, uint64_t sample_ms, uint64_t now_ms)
{
    monitor_tick(m, now_ms);
    m->pulse_us = pulse_us;
    m->raw_mm = 0;
    if (!monitor_distance(m->config.sensor, pulse_us, m->config.offset_mm, &m->raw_mm)) {
        monitor_fault(m, REASON_PULSE); return;
    }
    /* Converted pulse values already include the additive correction. */
    uint32_t converted = m->raw_mm;
    accept_distance(m, converted, sample_ms, now_ms);
}
static void accept_distance(struct monitor *m, uint32_t distance_mm, uint64_t sample_ms, uint64_t now_ms)
{
    m->raw_mm = distance_mm;
    if (!m->config_ok) { monitor_fault(m, REASON_CONFIG); return; }
    if (sample_ms > now_ms || now_ms - sample_ms >= m->config.timeout_ms ||
        (m->have_valid && sample_ms <= m->last_valid_ms)) {
        monitor_fault(m, REASON_CAPTURE); return;
    }
    /* Minimum-range values may be saturation, so equality is also rejected. */
    if (m->raw_mm <= m->config.min_distance_mm || m->raw_mm > m->config.max_distance_mm ||
        m->raw_mm > m->config.sensor_height_mm ||
        m->config.sensor_height_mm - m->raw_mm > m->config.max_depth_mm) {
        monitor_fault(m, REASON_RANGE); return;
    }
    m->last_valid_ms = sample_ms; m->have_valid = true;
    m->filtered_mm = estimate(m, m->raw_mm);
    m->depth_mm = m->config.sensor_height_mm - m->filtered_mm;
    m->level_percent = (uint32_t)((uint64_t)m->depth_mm * 100 / m->config.max_depth_mm);
    m->validity = MON_VALID;
    uint32_t raw_depth = m->config.sensor_height_mm - m->raw_mm;
    /* Raw low water blocks immediately, even if the smoother still says OK. */
    if (raw_depth <= m->config.trip_depth_mm || m->depth_mm <= m->config.trip_depth_mm) {
        m->allowed = false; m->recovery_count = 0; m->reason = REASON_LOW;
    } else if (!m->allowed) {
        if (raw_depth >= m->config.restart_depth_mm && m->depth_mm >= m->config.restart_depth_mm)
            m->recovery_count++;
        else m->recovery_count = 0;
        m->allowed = m->recovery_count >= m->config.recovery_samples;
        m->reason = m->allowed ? REASON_READY : REASON_RECOVERING;
    } else m->reason = REASON_READY;
}
const char *monitor_validity_name(enum monitor_validity v)
{
    static const char *names[] = {"UNKNOWN", "VALID", "INVALID", "STALE", "UNCONFIGURED"};
    return (unsigned)v < sizeof(names)/sizeof(names[0]) ? names[v] : "INVALID";
}
const char *monitor_reason_name(enum monitor_reason r)
{
    static const char *names[] = {"STARTUP", "READY", "LOW", "RECOVERING", "RANGE", "PULSE", "TIMEOUT", "CONFIG", "CAPTURE"};
    return (unsigned)r < sizeof(names)/sizeof(names[0]) ? names[r] : "CAPTURE";
}
