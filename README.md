# FarmBot Water Monitor

A standalone reservoir monitoring prototype developed for the REIT4841 engineering thesis at The University of Queensland. An MB7389 ultrasonic sensor and a Freenove Arduino Uno R3-compatible board estimate water depth, detect low and high water conditions, and provide LCD and browser dashboard feedback.

The system provides a water-availability permission indication through serial telemetry and the built-in LED. Physical pump or valve control and FarmBot OS integration remain future work. The active implementation uses Arduino C++ for the hardware adapter and a shared C monitoring core.

## Repository layout

| Directory | Contents |
| --- | --- |
| `arduino/FarmbotWaterMonitor/` | Active Uno sketch, configuration and monitoring core |
| `dashboard/` | Browser application, usage guide and JavaScript tests |
| `tools/` | Serial capture and CSV analysis tools |
| `tests/` | Host-based firmware and Python tests |
| `docs/` | Hardware worksheet, LCD instructions and experiment metadata template |
| `legacy/zephyr/` | Earlier nRF52840/Zephyr implementation |
| `experiments/original_baseline/` | Earlier baseline modules excluded from active firmware |

## Live web dashboard

A browser dashboard is available under `dashboard/dist/`. Raw telemetry viewing works with existing firmware. Direct calibration saving requires the current sketch. Close other serial connections and run this command from the repository root:

```sh
python3 -m http.server 8765 --bind 127.0.0.1 --directory dashboard/dist
```

Open http://127.0.0.1:8765 in desktop Chrome or Edge, click **Connect Arduino**, and select the USB serial device. It shows live distance, configured water depth, status, a chart, and log import/export. Unconfigured geometry and minimum-range/no-target readings are clearly labelled. See [dashboard instructions](dashboard/README.md). Sensor data stays in the browser tab.

## Hardware connections

The default build uses MB7389 serial output on D8. Identify the numbered sensor pads before connecting power.

| MB7389 pin | Uno connection |
| --- | --- |
| 6, V+ | 5V |
| 7, GND | GND |
| 5, serial TX | D8 |
| 4, ranging control | Leave disconnected and insulated for free-running operation |
| 1, 2, 3 | Leave disconnected for the default serial build |

Leave **D9 disconnected**. SoftwareSerial reserves it as an unused transmit pin. Leave Uno D0/D1 free for USB upload/logging. Leave the sensor control wire unconnected for free-running measurement. Confirm positions against the [manufacturer pinout](https://maxbotix.com/pages/hrxl-maxsonar-wr-datasheet); do not connect a computer RS232 port to this TTL signal.

Sensor input runs at 9600 baud and USB telemetry at 115200 baud. The built-in D13 LED indicates water-availability permission, with on meaning allowed and off meaning blocked.

For pulse input instead, set `WATER_USE_HRXL_SERIAL=0` in config.h and connect sensor **pin 2 PW to D2**. Do not run both input methods together. For the MB1000 comparison also choose `SENSOR_MB1000`, minimum distance 153 mm and maximum distance 6450 mm, then rebuild. MB1000 serial output is not supported by the default MB7389 serial decoder.

## Upload

1. Open `arduino/FarmbotWaterMonitor/FarmbotWaterMonitor.ino` in Arduino IDE. Keep the complete sketch folder, including `config.h` and `src/`, together.
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

Point the sensor at a flat target roughly one metre away. With HRXL selected, `raw_mm` should be around 1000. The committed configuration leaves tank dimensions unset. Without valid EEPROM calibration, the firmware reports **UNCONFIGURED, permission=0**. Verify raw distance across several known positions before using water-level estimates.

## Configure your actual tank

Use the dashboard to enter five measured settings in millimetres: sensor height above the bottom datum, maximum operating depth, low-water trip depth, recovery depth and high-water warning depth. Click **Save calibration to Arduino** and verify the acknowledgement and matching active settings. Valid EEPROM settings take priority over compiled geometry after restart.

For a compiled configuration or changes to sensor profile, filter and timeout, edit **arduino/FarmbotWaterMonitor/config.h** and upload the sketch:

- `sensor`: defaults to `SENSOR_HRXL`; select `SENSOR_MB1000` only for that sensor and change its range limits.
- `min_distance_mm`, `max_distance_mm`: verify the exact sensor model. For the identified MB7389 these are 300/4999 mm; the 5000 serial value means no target. Validate mounting clearance independently.
- `sensor_height_mm`: sensor face height above the tank-bottom datum.
- `max_depth_mm`: maximum operating water depth above that datum.
- `trip_depth_mm`: minimum usable depth including pump/intake and uncertainty margin.
- `restart_depth_mm`: higher recovery threshold.
- `high_depth_mm`: high-water advisory threshold.
- `offset_mm`: measured additive distance correction, initially zero.
- `timeout_ms`: maximum accepted measurement age, initially 1000 ms.
- `recovery_samples`: consecutive healthy restart samples, initially five.
- `calibrated`: keep false until the dimensions and profile are verified, then set true.

Use [the measurement worksheet](docs/HARDWARE_CONFIG.md). The validator blocks permission for inconsistent geometry, including insufficient full-tank sensor clearance. Depth percentage is relative to maximum operating depth, **not volume percentage**. No litres estimate is generated.

Record each experiment using [the run metadata template](docs/experiment_log_template.csv), and preserve the active configuration alongside the captured sensor data and independently measured reference depths.

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

## Verification and evaluation status

The presentation poster documents dashboard measurements, EEPROM calibration readback and LCD water-depth and high-water indications. These observations establish prototype functionality. Quantitative ground-truth testing is being prepared to evaluate depth error, repeatability and behaviour during water-level changes.

Host tests cover monitoring decisions, serial parsing, calibration records, LCD states and Python logging tools. Dashboard tests cover parsing, calibration validation, preview decisions and save confirmation. Run the checks below from the repository root. Firmware tests require Python 3 and a C compiler with AddressSanitizer and UndefinedBehaviorSanitizer support. Dashboard tests require Node.js.

```sh
./tests/run.sh
node dashboard/test.mjs
node --check dashboard/dist/app.js
```

Physical trials are needed to establish installed sensor accuracy, timing, restart persistence and performance under disturbances.

## Testing and old firmware

Run `./tests/run.sh` for the platform-independent monitoring and Python tooling tests. This does not replace hardware calibration, ISR timing checks, disconnect/recovery tests or an actual watering-interface demonstration.

The former nRF52840/Zephyr files are preserved under `legacy/zephyr/`. Its build file now points to the same monitoring core. The legacy adapter has not been rebuilt during this migration. Original pre-reliability modules remain under `experiments/original_baseline/` and are excluded from active firmware.

Record the controller change, Arduino/core version, build configuration and AI assistance accurately in your thesis. Thesis drafts, poster exports and project planning notes are maintained separately from this source repository.

References: [Arduino Uno](https://docs.arduino.cc/hardware/uno-rev3/), [Arduino interrupts](https://docs.arduino.cc/language-reference/funktionen/external-interrupts/attachInterrupt), [HRXL datasheet](https://maxbotix.com/pages/hrxl-maxsonar-wr-datasheet).

## Calibration saved over USB
The current Uno sketch supports saving the five geometry/threshold values directly from the dashboard after one firmware upload. See [dashboard instructions](dashboard/README.md#direct-usb-calibration-one-time-firmware-update-required). Valid EEPROM geometry overrides config.h after reboot; replacing config.h alone does not override an existing saved calibration. Use the dashboard Save button to change saved values.

## Local LCD
The firmware supports a PCF8574 I2C LCD using built-in Wire. See [LCD wiring, upload and test instructions](docs/LCD_SETUP.md). Connect GND/GND, VCC/5V, SDA/A4, SCL/A5 with power off; share supply rails properly with the sensor. One updated firmware upload is required. The poster documents LCD depth and high-water indications. Compare its readings with an independent reference during validation.
