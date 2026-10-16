#ifndef HRXL_SERIAL_H
#define HRXL_SERIAL_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
struct hrxl_parser { uint16_t value; uint8_t digits; uint8_t active; };
// 1: complete Rdddd\r frame, 0: incomplete/noise, -1: malformed frame.
int hrxl_feed(struct hrxl_parser *p, char c, uint16_t *distance);
#ifdef __cplusplus
}
#endif
#endif
