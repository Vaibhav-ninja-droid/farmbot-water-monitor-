# Two-day demo plan — 15 October 2026

## Choice and scope

Use the pictured Freenove Uno-compatible ATmega328P board and **MB7389** as the primary candidate. The other sensor is **MB1000 LV-MaxSonar-EZ0**. Weather resistance and finer output resolution make MB7389 the stronger candidate for the outdoor installation, subject to clearance and measured reliability. MB1000 is a useful comparison and backup, especially if the installation cannot accommodate MB7389's longer minimum distance. Manufacturer resolution is not your measured accuracy.

Do not redesign around both sensors simultaneously. Keep MB7389 working first. Photograph and record a short MB1000 comparison only after the main demonstration is repeatable. Operate one ultrasonic sensor at a time.

## Exact input change

Code now defaults to MB7389 TTL serial pin 5 → Uno D8, plus pin 6 → 5V and pin 7 → GND. Sensor pin 4 stays unconnected. The photo appears to show black/red/yellow/green on pins 7/6/5/4; verify the pad numbers before relying on this mapping. Pin 2 (PW) appears unwired. This is why the previous pulse-only code would not work with the existing harness unchanged. Do not connect the green/control wire to ground: that would stop free-running measurement.

No pump is wired. D13 and console permission demonstrate the monitoring decision only.

## Day 1: make readings trustworthy

1. Wire with USB disconnected. Verify supply and ground. Upload the main sketch as Arduino Uno and open Serial Monitor at 115200.
2. Point at a large flat target about one metre away. Look for changing `raw_mm`, `sample_event=1`, and the boot banner `input=TTL_SERIAL_D8`. `pulse_us=0` is expected in serial mode. UNCONFIGURED is also expected initially.
3. Check five known distances in the valid range, recording the reference and repeated readings. Do not treat the clamped 300 mm reading as a valid close measurement. Prefer at least 500 mm clearance if physically practical, and assess the actual range in the installed mount.
4. At the tank measure sensor-face height above the bottom datum H, maximum operating depth, intake height, and the sensor-to-water gap at maximum fill. Record mounting position and reference uncertainty. Coordinate with the lid mechanism's owner. Insufficient clearance requires a raised mount or a different test arrangement, not invented dimensions.
5. Choose trip depth above intake with an uncertainty/drawdown margin, and restart above trip. Enter measurements in config.h and only then enable `calibrated=true`. Rebuild/upload.
6. Collect at least five known water levels, with extra attention around the low-water boundary. Keep the water calm long enough to obtain steady readings. Save raw console/CSV, reference measurements, setup photos and the exact config.

## Day 2: prove decisions and rehearse

1. Demonstrate sufficient water: wait for five healthy recovery readings, then observe permission=1 and D13 on.
2. Lower water below trip: permission must block. Do not run a real pump dry; drain manually or use an approved controlled setup.
3. Hover in the trip/restart band: verify it remains blocked after a trip, and only recovers above restart after confirmation. The same band retains permission if approached from the healthy side without crossing trip.
4. With initially healthy data, disconnect only the sensor signal: verify permission blocks after the configured timeout (nominal 1 s plus processing/logging delay). Reconnect and demonstrate recovery. Repeat at least five times and log the outcome and observed delay; use an independent clock/video for elapsed-time claims.
5. Make a calibration plot and one time trace of low-water/disconnection/recovery. The supplied analysis tool reports numeric depth errors only after you add independent reference_depth_mm values. Draw plots from actual logs, never manufactured readings.
6. Record a short backup video of the working demonstration and save a known-working code/config copy.

If a sensor stops responding, check pin number, D8 versus D2 input selection, power, unconnected control pin and the Serial Monitor baud before changing processing algorithms. The new MB7389 parser expects R plus four digits and carriage return. An unreadable model/startup banner may briefly cause a capture fault; a valid stream must recover through the normal checks.

## Demo story

Show the real tank constraint, explain sensor/interface selection, present one measured calibration result, and demonstrate healthy/low/lost-signal/recovery states. State clearly whether it was tested on the real tank or a separate rig and whether actuation is simulated. A flat target confirms ranging only; it does not validate water-surface operation.

## What belongs in the thesis

Include alternatives you actually investigated and explain what each trial changed: sensor comparison, mounting clearance/false echoes, filter tradeoff, serial versus pulse timing, low-water hysteresis and fault handling. Keep original failed runs with their cause and resolution. A coherent argument with evidence matters more than the number of pages. Keep plans, code tests, bench measurements, tank measurements and demonstrated integration distinct.

The MB7389's internal free-running filtering contributes delay before the Arduino sees a changed level. SoftwareSerial and USB printing can also influence timing. Measure end-to-end response; do not claim that the 1 s software freshness setting bounds physical water-change detection.

## Outstanding facts

Tank dimensions, intake margin, actual echo quality, power/noise stability, receiver timing and FarmBot control interface remain unverified. Firmware will not allow permission until consistent tank configuration is supplied. If integration is unavailable, report the local decision demonstration honestly.

Sources: [MB7389 specifications](https://maxbotix.com/products/mb7389), [HRXL pinout and protocol](https://maxbotix.com/pages/hrxl-maxsonar-wr-datasheet), [MB1000 specifications](https://maxbotix.com/products/ultrasonic_sensors_mb1000).
