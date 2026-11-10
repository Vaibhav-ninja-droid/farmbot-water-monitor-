#include "lcd_status.h"
#include <stdio.h>
#include <string.h>
static void line(char *out, const char *text) {
    size_t n = strlen(text);
    if (n > 16)
        n = 16;
    memset(out, ' ', 16);
    memcpy(out, text, n);
    out[16] = 0;
}
void lcd_status_lines(const struct monitor *m, char lines[2][17]) {
    char text[24];
    if (!m->config_ok || m->validity == MON_UNCONFIGURED) {
        line(lines[0], "Calibration req.");
        line(lines[1], "Save via USB");
        return;
    }
    if (m->validity != MON_VALID || !m->have_valid) {
        line(lines[0], m->validity == MON_STALE     ? "Sensor timeout"
                       : m->validity == MON_UNKNOWN ? "Waiting sensor"
                                                    : "Sensor fault");
        line(lines[1], "Water blocked");
        return;
    }
    snprintf(text, sizeof(text), "Depth:%4lu mm", (unsigned long)m->depth_mm);
    line(lines[0], text);
    line(lines[1], m->reason == REASON_LOW ? "LOW WATER"
                   : m->high_water         ? "HIGH WATER"
                   : m->allowed            ? "Water available"
                                           : "Recovering...");
}
