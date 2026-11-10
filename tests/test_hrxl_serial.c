#include "hrxl_serial.h"
#include "monitor.h"
#include <assert.h>
#include <stdio.h>
static int feed(struct hrxl_parser *p, const char *s, uint16_t *d) {
    int result = 0;
    while (*s) {
        int r = hrxl_feed(p, *s++, d);
        if (r)
            result = r;
    }
    return result;
}
int main(void) {
    struct hrxl_parser p = {0};
    uint16_t d = 0;
    assert(feed(&p, "boot info\r\n", &d) == 0);
    assert(feed(&p, "R1000\r", &d) == 1 && d == 1000);
    assert(feed(&p, "R0300\r", &d) == 1 && d == 300);
    assert(feed(&p, "R5000\r", &d) == 1 && d == 5000); // caller must reject no target
    assert(feed(&p, "R123\r", &d) == -1);
    assert(feed(&p, "R12345\r", &d) == -1);
    assert(feed(&p, "R1x00\r", &d) == -1);
    assert(feed(&p, "R12R1400\r", &d) == 1 && d == 1400); // resync after bad frame
    assert(feed(&p, "R0000\r", &d) == 1 && d == 0);
    struct monitor_config c = {.sensor = SENSOR_HRXL,
                               .filter = FILTER_MEDIAN,
                               .calibrated = true,
                               .min_distance_mm = 300,
                               .max_distance_mm = 4999,
                               .sensor_height_mm = 1500,
                               .max_depth_mm = 1000,
                               .trip_depth_mm = 200,
                               .restart_depth_mm = 300,
                               .timeout_ms = 1000,
                               .recovery_samples = 5};
    struct monitor m;
    monitor_init(&m, &c);
    for (int i = 1; i <= 5; i++)
        monitor_distance_sample(&m, 1000, i * 150, i * 150);
    assert(m.allowed && m.depth_mm == 500 && m.pulse_us == 0);
    monitor_distance_sample(&m, 1300, 900, 900);
    assert(!m.allowed && m.reason == REASON_LOW);
    monitor_distance_sample(&m, 0, 1050, 1050);
    assert(!m.allowed && m.reason == REASON_RANGE);
    c.offset_mm = -10;
    monitor_init(&m, &c);
    monitor_distance_sample(&m, 1000, 1, 1);
    assert(m.raw_mm == 990 && m.depth_mm == 510);
    monitor_tick(&m, 1001);
    assert(!m.allowed && m.validity == MON_STALE);
    puts("PASS: HRXL serial framing, resynchronisation, direct-distance input, offset and "
         "low-water/timeout handling");
}
