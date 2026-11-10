#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include "monitor.h"
#include "sonar.h"
/* DK LEDs only. No pump output is inferred from a board pin. */
static const struct gpio_dt_spec permit_led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec block_led = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
int main(void) {
    const struct monitor_config config = {
        .sensor = IS_ENABLED(CONFIG_WATER_SENSOR_HRXL) ? SENSOR_HRXL : SENSOR_MB1000,
        .filter = IS_ENABLED(CONFIG_WATER_FILTER_RAW)    ? FILTER_RAW
                  : IS_ENABLED(CONFIG_WATER_FILTER_MEAN) ? FILTER_MEAN
                                                         : FILTER_MEDIAN,
        .calibrated = IS_ENABLED(CONFIG_WATER_CALIBRATED),
        .min_distance_mm = CONFIG_WATER_MIN_DISTANCE_MM,
        .max_distance_mm = CONFIG_WATER_MAX_DISTANCE_MM,
        .offset_mm = CONFIG_WATER_OFFSET_MM,
        .sensor_height_mm = CONFIG_WATER_SENSOR_HEIGHT_MM,
        .max_depth_mm = CONFIG_WATER_MAX_DEPTH_MM,
        .trip_depth_mm = CONFIG_WATER_TRIP_DEPTH_MM,
        .restart_depth_mm = CONFIG_WATER_RESTART_DEPTH_MM,
        .timeout_ms = CONFIG_WATER_TIMEOUT_MS,
        .recovery_samples = CONFIG_WATER_RECOVERY_SAMPLES,
    };
    struct monitor m;
    monitor_init(&m, &config);
    bool leds_ok = gpio_is_ready_dt(&permit_led) && gpio_is_ready_dt(&block_led);
    if (leds_ok)
        leds_ok = gpio_pin_configure_dt(&permit_led, GPIO_OUTPUT_INACTIVE) == 0 &&
                  gpio_pin_configure_dt(&block_led, GPIO_OUTPUT_ACTIVE) == 0;
    int sensor_status = sonar_init();
    printk("# "
           "water-monitor,v1,sensor=%s,filter=%d,calibrated=%d,height=%u,max_depth=%u,trip=%u,"
           "restart=%u,min_distance=%u,max_distance=%u,offset=%d,timeout=%u,recovery=%u,sonar_"
           "status=%d,leds=%d\n",
           config.sensor == SENSOR_MB1000 ? "MB1000" : "HRXL", config.filter, config.calibrated,
           config.sensor_height_mm, config.max_depth_mm, config.trip_depth_mm,
           config.restart_depth_mm, config.min_distance_mm, config.max_distance_mm,
           config.offset_mm, config.timeout_ms, config.recovery_samples, sensor_status, leds_ok);
    printk("uptime_ms,sequence,sample_event,pulse_us,raw_mm,filtered_mm,depth_mm,depth_percent,"
           "sample_age_ms,validity,permission,reason,capture_overruns\n");
    uint32_t sequence = 0;
    uint64_t last_log = 0;
    uint32_t previous_overruns = 0;
    while (1) {
        uint64_t now = k_uptime_get();
        struct sonar_sample sample;
        bool received = sensor_status == 0 && sonar_get_sample(&sample);
        now = k_uptime_get();
        monitor_tick(&m, now);
        if (sensor_status != 0)
            monitor_fault(&m, REASON_CAPTURE);
        if (received) {
            if (sample.capture_ok)
                monitor_sample(&m, sample.pulse_us, sample.captured_ms, now);
            else {
                m.pulse_us = sample.pulse_us;
                m.raw_mm = 0;
                monitor_fault(&m, REASON_CAPTURE);
            }
        }
        uint32_t dropped = sonar_overruns();
        if (dropped != previous_overruns)
            monitor_fault(&m, REASON_CAPTURE);
        previous_overruns = dropped;
        if (leds_ok) {
            gpio_pin_set_dt(&permit_led, m.allowed);
            gpio_pin_set_dt(&block_led, !m.allowed);
        }
        if (received || now - last_log >= 250 || sequence == 0) {
            int64_t age = m.have_valid ? (int64_t)(now - m.last_valid_ms) : -1;
            printk("%llu,%u,%d,%u,%u,%u,%u,%u,%lld,%s,%d,%s,%u\n", (unsigned long long)now,
                   sequence++, received, m.pulse_us, m.raw_mm, m.filtered_mm, m.depth_mm,
                   m.level_percent, (long long)age, monitor_validity_name(m.validity), m.allowed,
                   monitor_reason_name(m.reason), sonar_overruns());
            last_log = now;
        }
        k_sleep(K_MSEC(10));
    }
}
