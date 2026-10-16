#include <Arduino.h>
#include <util/atomic.h>
#include "config.h"
#include "src/hrxl_serial.h"
#if WATER_USE_HRXL_SERIAL
#include <SoftwareSerial.h>
static SoftwareSerial sensorSerial(SENSOR_SERIAL_RX_PIN, SENSOR_SERIAL_UNUSED_TX_PIN);
static hrxl_parser parser = {};
static uint32_t parserLastByteMs;
#endif
#if !defined(__AVR_ATmega328P__)
#error "Select Arduino Uno (ATmega328P); this capture adapter targets that board."
#endif

static monitor state;
static volatile uint32_t overruns;
#if !WATER_USE_HRXL_SERIAL
static volatile uint32_t riseUs, riseMs, pendingPulse, pendingMs;
static volatile bool riseSeen, ready, pendingOk;
#endif
static uint32_t previousMillis, previousOverruns, sequence;
static uint64_t elapsedMs, lastLog;

#if !WATER_USE_HRXL_SERIAL
// ISR only captures edges. No serial output, filtering or delay in interrupts.
static void captureEdge() {
    const uint32_t us = micros();
    const uint32_t ms = millis();
    const bool high = digitalRead(SENSOR_PW_PIN) == HIGH;
    if (high && !riseSeen) {
        riseUs = us; riseMs = ms; riseSeen = true;
        return;
    }
    const uint32_t width = us - riseUs; // unsigned subtraction survives micros rollover
    const bool paired = !high && riseSeen;
    const bool ok = paired && (uint32_t)(ms - riseMs) <= 100 && width > 0 && width <= 100000;
    riseSeen = false;
    if (ready) ++overruns;
    pendingPulse = paired ? width : 0;
    pendingMs = ms;
    pendingOk = ok;
    ready = true;
}

 #endif

// Arduino Print has no portable uint64_t overload on AVR.
static void print64(uint64_t value) {
    char digits[21];
    uint8_t length = 0;
    do { digits[length++] = '0' + value % 10; value /= 10; } while (value);
    while (length) Serial.write(digits[--length]);
}
static void comma() { Serial.write(','); }
static void logRow(bool event, uint64_t now, uint32_t drops) {
    print64(now); comma(); Serial.print(sequence++); comma(); Serial.print(event ? 1 : 0);
    comma(); Serial.print(state.pulse_us); comma(); Serial.print(state.raw_mm);
    comma(); Serial.print(state.filtered_mm); comma(); Serial.print(state.depth_mm);
    comma(); Serial.print(state.level_percent); comma();
    if (state.have_valid) print64(now - state.last_valid_ms); else Serial.print(-1);
    comma(); Serial.print(monitor_validity_name(state.validity)); comma(); Serial.print(state.allowed ? 1 : 0);
    comma(); Serial.print(monitor_reason_name(state.reason)); comma(); Serial.println(drops);
}
void setup() {
    pinMode(PERMISSION_LED_PIN, OUTPUT);
    digitalWrite(PERMISSION_LED_PIN, LOW);
    pinMode(SENSOR_PW_PIN, INPUT);
    Serial.begin(SERIAL_BAUD);
    monitor_config c = water_config();
    monitor_init(&state, &c);
    previousMillis = millis();
#if WATER_USE_HRXL_SERIAL
    sensorSerial.begin(9600); // MB7389 is non-inverted TTL, 9600-8-N-1.
#else
    attachInterrupt(digitalPinToInterrupt(SENSOR_PW_PIN), captureEdge, CHANGE);
#endif
    Serial.print(F("# water-monitor,v1,platform=uno,sensor="));
    Serial.print(c.sensor == SENSOR_HRXL ? F("HRXL") : F("MB1000"));
    Serial.print(F(",input="));
    Serial.print(WATER_USE_HRXL_SERIAL ? F("TTL_SERIAL_D8") : F("PULSE_D2"));
    Serial.print(F(",filter=")); Serial.print((int)c.filter);
    Serial.print(F(",calibrated=")); Serial.print(c.calibrated ? 1 : 0);
    Serial.print(F(",height=")); Serial.print(c.sensor_height_mm);
    Serial.print(F(",max_depth=")); Serial.print(c.max_depth_mm);
    Serial.print(F(",trip=")); Serial.print(c.trip_depth_mm);
    Serial.print(F(",restart=")); Serial.print(c.restart_depth_mm);
    Serial.print(F(",min_distance=")); Serial.print(c.min_distance_mm);
    Serial.print(F(",max_distance=")); Serial.print(c.max_distance_mm);
    Serial.print(F(",offset=")); Serial.print(c.offset_mm);
    Serial.print(F(",timeout=")); Serial.print(c.timeout_ms);
    Serial.print(F(",recovery=")); Serial.println(c.recovery_samples);
    Serial.println(F("uptime_ms,sequence,sample_event,pulse_us,raw_mm,filtered_mm,depth_mm,depth_percent,sample_age_ms,validity,permission,reason,capture_overruns"));
}
void loop() {
    uint32_t pulse = 0, drops;
    bool event;
#if !WATER_USE_HRXL_SERIAL
    uint32_t stamp = 0;
    bool ok = false;
#endif
    uint32_t ms;
#if WATER_USE_HRXL_SERIAL
    event = false;
    drops = overruns;
    ms = millis();
    // Inter-byte expiry prevents two truncated frames being joined together.
    if (parser.active && (uint32_t)(ms - parserLastByteMs) >= 50) {
        parser.active = 0;
        monitor_fault(&state, REASON_CAPTURE);
    }
    bool bad = sensorSerial.overflow();
    if (bad) { ++overruns; parser.active = 0; }
    // Drain bounded receive buffer before logging; retain newest complete sample.
    while (sensorSerial.available()) {
        uint16_t distance = 0;
        int result = hrxl_feed(&parser, (char)sensorSerial.read(), &distance);
        parserLastByteMs = millis();
        if (result < 0) bad = true;
        if (result > 0) {
            if (event) { ++overruns; bad = true; }
            event = true; pulse = distance;
        }
    }
    drops = overruns;
    ms = millis();
#else
    // AVR cannot read 32-bit ISR fields atomically without this critical section.
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        event = ready;
        if (event) { pulse = pendingPulse; stamp = pendingMs; ok = pendingOk; ready = false; }
        drops = overruns;
    }
    ms = millis();
#endif
    elapsedMs += (uint32_t)(ms - previousMillis);
    previousMillis = ms;
    const uint64_t now = elapsedMs;
    monitor_tick(&state, now);
#if WATER_USE_HRXL_SERIAL
    if (event) {
        if (pulse == 0 || pulse >= 5000) {
            state.raw_mm = pulse; state.pulse_us = 0;
            monitor_fault(&state, REASON_RANGE);
        } else monitor_distance_sample(&state, pulse, now, now);
    }
    if (bad) monitor_fault(&state, REASON_CAPTURE);
#else
    if (event) {
        const uint32_t age = ms - stamp;
        if (ok && age <= now) monitor_sample(&state, pulse, now - age, now);
        else { state.pulse_us = pulse; state.raw_mm = 0; monitor_fault(&state, REASON_CAPTURE); }
    }
#endif
    if (drops != previousOverruns) monitor_fault(&state, REASON_CAPTURE);
    previousOverruns = drops;
    digitalWrite(PERMISSION_LED_PIN, state.allowed ? HIGH : LOW);
    if (event || now - lastLog >= HEARTBEAT_MS || sequence == 0) {
        logRow(event, now, drops);
        lastLog = now;
    }
}
