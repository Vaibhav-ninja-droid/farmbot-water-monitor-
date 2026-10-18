# FarmBot water monitor — Freenove Uno

The active firmware now targets the Freenove Arduino Uno R3-compatible board (ATmega328P), using Arduino C++ for the hardware adapter and the tested C monitoring core. Open **arduino/FarmbotWaterMonitor/FarmbotWaterMonitor.ino** in Arduino IDE. Keep the complete sketch folder, including `config.h` and `src/`, together.

## Live web dashboard

A browser dashboard is available under `dashboard/dist/`; it uses the existing firmware without re-uploading. Close the serial monitor first. Start it with:

```sh
python3 -m http.server 8765 --bind 127.0.0.1 --directory dashboard/dist
```

Open http://127.0.0.1:8765 in desktop Chrome or Edge, click **Connect Arduino**, and select the USB serial device. It shows live distance, configured water depth, status, a chart, and log import/export. Unconfigured geometry and minimum-range/no-target readings are clearly labelled. See [dashboard instructions](dashboard/README.md). Sensor data stays in the browser tab.

## Connections — updated 15 October: MB7389 serial input

The new photo identifies MB7389 (30mm1.5 housing). The default is now **TTL serial**, not pulse width. Identify the numbered pads before powering; wire colours below are an inference from the supplied photo, not a universal colour code.

| MB7389 pin | Uno | Photo interpretation to verify |
|---|---|---|
| 6, V+ | 5V | Red appears to occupy this pad |
| 7, GND | GND | Black appears to occupy this pad |
| 5, serial TX | **D8** | Yellow appears to occupy this pad |
| 4, ranging control | Leave its wire disconnected and insulated | Green appears to occupy this pad |
| 1, 2, 3 | Leave disconnected | No extra sensor wire needed |

Leave **D9 disconnected**. SoftwareSerial reserves it as an unused transmit pin. Leave Uno D0/D1 free for USB upload/logging. Leave the sensor control wire unconnected for free-running measurement. Confirm positions against the [manufacturer pinout](https://maxbotix.com/pages/hrxl-maxsonar-wr-datasheet); do not connect a computer RS232 port to this TTL signal.

Default sensor input is 9600 baud; USB Serial Monitor stays at 115200 baud. The existing PW pad appears unwired, so the old D2 connection will not read the pictured harness. The LED remains D13: on means permission, off means blocked. There is no physical pump-control output.

For pulse input instead, set `WATER_USE_HRXL_SERIAL=0` in config.h and connect sensor **pin 2 PW to D2**. Do not run both input methods together. For the MB1000 comparison also choose `SENSOR_MB1000`, minimum distance 153 mm and maximum distance 6450 mm, then rebuild. MB1000 serial output is not supported by the default MB7389 serial decoder.

## Upload

1. Open the `.ino` file in Arduino IDE.
2. Install/select **Arduino AVR Boards → Arduino Uno**.
3. Select the Freenove board's serial port.
4. Click Verify, then Upload.
5. Open Serial Monitor at **115200 baud**.

Do not select Mega, Uno R4 or nRF52840. Some compatible boards use a different USB serial chip; investigate the chip/driver only if no port appears.

CLI equivalent, with Arduino CLI and the AVR core installed:

```sh
arduino-cli core install arduino:avr
arduino-cli compile --fqbn arduino:avr:uno arduino/FarmbotWaterMonitor
arduino-cli upload --fqbn arduino:avr:uno --port /dev/cu.YOUR_PORT arduino/FarmbotWaterMonitor
```

## First bench check

Point the sensor at a flat target roughly one metre away. With HRXL selected, `raw_mm` should be around 1000. The output will initially say **UNCONFIGURED, permission=0**. That is expected: tank dimensions remain unknown. Verify raw distance across several known positions before using water-level estimates.

## Configure your actual tank

Edit **arduino/FarmbotWaterMonitor/config.h**:

- `sensor`: defaults to `SENSOR_HRXL`; select `SENSOR_MB1000` only for that sensor and change its range limits.
- `min_distance_mm`, `max_distance_mm`: verify the exact sensor model. For the identified MB7389 these are 300/4999 mm; the 5000 serial value means no target. Validate mounting clearance independently.
- `sensor_height_mm`: sensor face height above the tank-bottom datum.
- `max_depth_mm`: maximum operating water depth above that datum.
- `trip_depth_mm`: minimum usable depth including pump/intake and uncertainty margin.
- `restart_depth_mm`: higher recovery threshold.
- `offset_mm`: measured additive distance correction, initially zero.
- `timeout_ms`: maximum accepted measurement age, initially 1000 ms.
- `recovery_samples`: consecutive healthy restart samples, initially five.
- `calibrated`: keep false until the dimensions and profile are verified, then set true.

Use [the measurement worksheet](docs/HARDWARE_CONFIG.md). The validator blocks permission for inconsistent geometry, including insufficient full-tank sensor clearance. Depth percentage is relative to maximum operating depth, **not volume percentage**. No litres estimate is generated.

Follow [the two-day demo guide](docs/DEMO_TWO_DAY_PLAN.md) for the minimum experiment set.

## What the firmware does

- Reads bounded MB7389 `Rdddd` serial frames, rejects malformed/truncated frames and the no-target code; optional D2 pulse capture is retained.
- Rejects unpaired edges, malformed/zero pulses, invalid range, stale samples and capture overruns.
- Uses reported millimetres directly for MB7389 serial input. Optional pulse input converts HRXL at 1 us/mm or MB1000 at 147 us/inch.
- Supports `FILTER_RAW`, `FILTER_MEAN` and `FILTER_MEDIAN` for comparisons.
- Immediately blocks on raw low water even when the smoother still reports a higher level.
- Uses hysteresis and consecutive healthy readings before recovery.
- Checks timeouts independently of sensor arrivals, and extends `millis()` across rollover.
- Outputs sample records and 250 ms heartbeat records in the same CSV schema as before.

The pulse ISR contains no serial printing or filtering. Serial mode uses SoftwareSerial; its interrupt load and reception timestamps require bench verification. Frame reception time is not the instant of the acoustic measurement, and the sensor already filters free-running measurements. `micros()` on a standard 16 MHz Uno has 4 us granularity, so HRXL pulse timing is not automatically 1 mm accurate. Measure actual capture error and water-level performance. A plausible stuck reading or false echo can still be undetectable. This is an availability recommendation, not a guaranteed equipment-protection system.

## Experiment logs

Close Serial Monitor before opening the capture tool; both use the same port. Opening the Uno serial port commonly resets it, which also blocks permission and restarts the recovery process.

```sh
python3 tools/capture.py --port /dev/cu.YOUR_PORT --output run01.csv
python3 tools/analyse.py run01.csv
```

Live capture needs `pyserial`. Saved-console processing needs no extra packages:

```sh
python3 tools/capture.py --input saved_console.txt --output run01.csv
```

Capture refuses to overwrite logs, preserves raw console text in an adjacent file, adds UTC reception time, and separates boot sessions. Add independently measured `reference_depth_mm` values for MAE/RMSE analysis. Heartbeat and non-VALID rows are excluded from error statistics. On invalid/stale records, processed estimates retain their previous values; only VALID rows represent accepted current estimates. `sample_age_ms=-1` means no valid sample has been accepted.

## Verification completed

15 October: MB7389 serial-input build compiled for Uno with AVR core 1.8.8: 8416 bytes flash and 579 bytes static RAM. Core monitoring, serial-parser and logging-tool tests pass. The optional MB1000 pulse-input build also compiles. Hardware reception and tank tests are still outstanding.

### Previous pulse-only build, 14 October 2026

Compiled for `arduino:avr:uno` using Arduino AVR core 1.8.8. Default unconfigured build: 7810 bytes flash, 477 bytes static RAM. A temporary compile with hypothetical valid geometry also passed: 8046 bytes flash, 477 bytes static RAM. The hypothetical values were not applied to the delivered configuration. Host C tests with sanitizers and Python tests pass. No board upload or physical sensor test has been performed.

## Testing and old firmware

Run `./tests/run.sh` for the platform-independent monitoring and Python tooling tests. This does not replace hardware calibration, ISR timing checks, disconnect/recovery tests or an actual watering-interface demonstration.

The former nRF52840/Zephyr files are preserved under `legacy/zephyr/`. Its build file now points to the same monitoring core. The legacy adapter has not been rebuilt during this migration. Original pre-reliability modules remain under `experiments/original_baseline/` and are excluded from active firmware.

See [the thesis plan](THESIS_RECOVERY_PLAN.md) for experiments. Its original Zephyr discussion is historical; the active controller is now Uno. Record the controller change, Arduino/core version, build configuration and AI assistance accurately in your thesis.

References: [Arduino Uno](https://docs.arduino.cc/hardware/uno-rev3/), [Arduino interrupts](https://docs.arduino.cc/language-reference/funktionen/external-interrupts/attachInterrupt), [HRXL datasheet](https://maxbotix.com/pages/hrxl-maxsonar-wr-datasheet).

## Calibration saved over USB
The current Uno sketch supports saving the four geometry/threshold values directly from the dashboard after one firmware upload. See [dashboard instructions](dashboard/README.md#direct-usb-calibration-one-time-firmware-update-required). Valid EEPROM geometry overrides config.h after reboot; replacing config.h alone does not override an existing saved calibration. Use the dashboard Save button to change saved values.
