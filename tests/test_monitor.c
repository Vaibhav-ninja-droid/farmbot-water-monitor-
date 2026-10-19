#include "monitor.h"
#include <assert.h>
#include <stdio.h>
static struct monitor_config cfg(void) {
    return (struct monitor_config){.sensor=SENSOR_HRXL,.filter=FILTER_MEDIAN,.calibrated=true,
        .min_distance_mm=300,.max_distance_mm=5000,.sensor_height_mm=1500,.max_depth_mm=1000,
        .trip_depth_mm=200,.restart_depth_mm=300,.timeout_ms=1000,.recovery_samples=5};
}
static void healthy(struct monitor *m, uint64_t first) {
    for (int i=0;i<5;i++) monitor_sample(m, 1000, first+i*150, first+i*150);
    assert(m->allowed);
}
int main(void) {
    uint32_t mm;
    assert(monitor_distance(SENSOR_MB1000,1470,0,&mm) && mm==254);
    assert(monitor_distance(SENSOR_HRXL,1000,0,&mm) && mm==1000);
    assert(monitor_distance(SENSOR_HRXL,1000,-10,&mm) && mm==990);
    assert(!monitor_distance(SENSOR_HRXL,0,0,&mm));
    assert(!monitor_distance(SENSOR_HRXL,100001,0,&mm));
    struct monitor m; struct monitor_config c=cfg();
    monitor_init(&m,&c); assert(!m.allowed && m.validity==MON_UNKNOWN);
    for(int i=0;i<4;i++) monitor_sample(&m,1000,150*(i+1),150*(i+1));
    assert(!m.allowed);
    monitor_sample(&m,1000,750,750); assert(m.allowed);
    /* Immediate raw trip must override a still-high median. */
    monitor_sample(&m,1300,900,900); assert(!m.allowed && m.reason==REASON_LOW);
    monitor_sample(&m,1250,1050,1050); assert(!m.allowed); /* hysteresis band */
    healthy(&m,1200);
    monitor_sample(&m,1250,1950,1950); assert(m.allowed); /* remain enabled in band */
    monitor_tick(&m,2949); assert(m.allowed);
    monitor_tick(&m,2950); assert(!m.allowed && m.validity==MON_STALE);
    healthy(&m,3100);
    monitor_sample(&m,0,3850,3850); assert(!m.allowed && m.reason==REASON_PULSE);
    healthy(&m,4000);
    monitor_sample(&m,1600,4750,4750); assert(!m.allowed && m.reason==REASON_RANGE);
    assert(m.count==0);
    healthy(&m,4900);
    monitor_sample(&m,300,5650,5650); assert(!m.allowed); /* saturation */
    healthy(&m,5800);
    monitor_sample(&m,400,6550,6550); assert(!m.allowed); /* above operating fill */
    healthy(&m,6700);
    monitor_sample(&m,1000,7000,7450); assert(!m.allowed); /* out of order */
    monitor_init(&m,&c);
    monitor_sample(&m,1000,1,1001); assert(!m.allowed && m.reason==REASON_CAPTURE);
    monitor_sample(&m,1000,2000,1999); assert(!m.allowed);
    c.calibrated=false; monitor_init(&m,&c);
    monitor_sample(&m,1000,1,1); assert(!m.allowed && m.validity==MON_UNCONFIGURED && m.raw_mm==1000);
    c=cfg(); c.sensor_height_mm=0; assert(!monitor_config_valid(&c));
    c=cfg(); c.restart_depth_mm=c.trip_depth_mm; assert(!monitor_config_valid(&c));
    c=cfg(); c.max_depth_mm=1200; assert(!monitor_config_valid(&c));
    /* Compare estimators without changing the permission policy. */
    for(int f=FILTER_RAW;f<=FILTER_MEDIAN;f++) {
        c=cfg(); c.filter=f; monitor_init(&m,&c); healthy(&m,150);
        monitor_sample(&m,1100,900,900);
        assert(m.filtered_mm==(f==FILTER_RAW?1100:f==FILTER_MEAN?1020:1000));
    }
    c=cfg(); monitor_init(&m,&c); healthy(&m,150);
    monitor_fault(&m,REASON_CAPTURE); assert(!m.allowed && m.count==0);
    c=cfg();c.high_depth_mm=900;monitor_init(&m,&c);healthy(&m,100);
    monitor_distance_sample(&m,600,900,900);assert(m.reason==REASON_HIGH&&m.high_water&&m.allowed);
    monitor_distance_sample(&m,620,1050,1050);assert(m.high_water); // 880 is inside clear band
    for(int i=0;i<5;i++)monitor_distance_sample(&m,700,1200+i*150,1200+i*150);
    assert(!m.high_water&&m.reason==REASON_READY&&m.allowed);
    monitor_distance_sample(&m,550,2000,2000);assert(m.high_water); // 950 still valid
    monitor_distance_sample(&m,499,2150,2150);assert(m.validity==MON_INVALID&&!m.high_water&&!m.allowed);
    c.high_depth_mm=1001;assert(!monitor_config_valid(&c));c.high_depth_mm=299;assert(!monitor_config_valid(&c));
    puts("PASS: conversion, configuration, startup/recovery, raw trip, hysteresis, timeouts, invalid and stale inputs, filters");
}
