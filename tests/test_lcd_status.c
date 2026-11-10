#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "lcd_status.h"
int main(void) {
    struct monitor m = {0};
    char lines[2][17];
    lcd_status_lines(&m, lines);
    assert(!strncmp(lines[0], "Calibration req.", 16));
    m.config_ok = true;
    m.validity = MON_UNKNOWN;
    lcd_status_lines(&m, lines);
    assert(!strncmp(lines[0], "Waiting sensor", 14));
    m.validity = MON_VALID;
    m.have_valid = true;
    m.depth_mm = 75;
    m.allowed = true;
    m.reason = REASON_READY;
    lcd_status_lines(&m, lines);
    assert(strstr(lines[0], "75 mm"));
    assert(!strncmp(lines[1], "Water available", 15));
    m.high_water = true;
    m.reason = REASON_HIGH;
    lcd_status_lines(&m, lines);
    assert(!strncmp(lines[1], "HIGH WATER", 10));
    m.high_water = false;
    m.allowed = false;
    m.reason = REASON_LOW;
    lcd_status_lines(&m, lines);
    assert(!strncmp(lines[1], "LOW WATER", 9));
    assert(strlen(lines[1]) == 16);
    m.reason = REASON_RECOVERING;
    lcd_status_lines(&m, lines);
    assert(!strncmp(lines[1], "Recovering...", 13));
    m.validity = MON_STALE;
    lcd_status_lines(&m, lines);
    assert(!strncmp(lines[0], "Sensor timeout", 14));
    assert(!strstr(lines[0], "75"));
    m.validity = MON_INVALID;
    lcd_status_lines(&m, lines);
    assert(!strncmp(lines[0], "Sensor fault", 12));
    for (int i = 0; i < 2; i++)
        assert(strlen(lines[i]) == 16);
    puts("PASS: LCD depth, low water, recovery, calibration, stale and invalid display states");
}
