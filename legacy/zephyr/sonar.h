#ifndef SONAR_H
#define SONAR_H
#include <stdbool.h>
#include <stdint.h>
struct sonar_sample { uint32_t pulse_us; uint64_t captured_ms; bool capture_ok; };
int sonar_init(void);
bool sonar_get_sample(struct sonar_sample *sample);
uint32_t sonar_overruns(void);
#endif
