#include "hrxl_serial.h"
int hrxl_feed(struct hrxl_parser *p, char c, uint16_t *distance)
{
    if (c == 'R') {
        int interrupted = p->active;
        p->active = 1; p->digits = 0; p->value = 0;
        return interrupted ? -1 : 0;
    }
    if (!p->active) return 0;
    if (c >= '0' && c <= '9' && p->digits < 4) {
        p->value = p->value * 10 + (c - '0'); p->digits++; return 0;
    }
    p->active = 0;
    if (c == '\r' && p->digits == 4) { *distance = p->value; return 1; }
    return -1;
}
