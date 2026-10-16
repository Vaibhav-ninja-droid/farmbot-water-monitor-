# FarmBot water monitor

Zephyr firmware for the nRF52840 DK plus host tools for water-monitoring experiments. The preferred profile is HRXL pulse width. MB1000 encoding is also selectable. The exact HRXL model, usable distance limits, electrical wiring and tank geometry still require verification.

## Implemented

- Paired-edge pulse capture with a synchronised ISR/main-thread handoff, capture-error detection and overrun counts.
- Separate MB1000 (147 us/inch) and HRXL (1 us/mm) distance conversions, with additive calibration offset.
- Rejection of zero/malformed pulses, minimum-range saturation, values outside the sensor/tank range and stale/out-of-order samples.
- RAW, five-sample MEAN and five-sample MEDIAN estimator options for controlled comparisons.
- Immediate raw low-water trip, hysteresis and consecutive healthy samples before restoring permission.
- Timeout supervision even when no new sensor samples arrive.
- Boot/configuration/fault states that block permission.
- Built-in DK LEDs: LED1 indicates permission, LED2 indicates blocked.
- CSV sample records and 250 ms heartbeat records through the board's configured console.
- Capture and analysis scripts plus host tests.

The code reports a **water-availability permission**, not a pump command. No pump, buzzer, relay, float switch, BLE or FarmBot API is configured without a known interface. Volume/litres are not inferred from unmeasured tank shape. Fresh plausible stuck readings and false echoes can remain undetectable; validate these in experiments.

## Before connecting the sensor

Verify the precise model on the sensor, supply voltage, ground, pulse-width output pin and board signal voltage. Do not infer wire functions from colours. `app.overlay` currently uses **P0.03**, inherited from the original project. Check this against the actual connection. Use a fixed sensor face and verify full-tank clearance against the model's minimum ranging distance. A moving lid may need a different mounting point.

Pin 2 is pulse width for the documented MaxBotix families; verify your exact unit's datasheet. HRXL output encoding differs from MB1000. Profile selection does not certify mounting clearance, voltage compatibility, or outdoor reliability.

## Configure measured values

By default `CONFIG_WATER_CALIBRATED=n` and dimensions are zero. Readings are logged, but validity is UNCONFIGURED and permission remains blocked. This lets you check raw conversion before making control decisions.

Make a local file `configs/tank_measured.conf` after completing [the hardware worksheet](docs/HARDWARE_CONFIG.md). Set these Kconfig symbols:

| Symbol | What to measure/choose |
|---|---|
| WATER_SENSOR_HEIGHT_MM | Sensor face to tank-bottom datum |
| WATER_MAX_DEPTH_MM | Maximum operating water depth from the same datum |
| WATER_TRIP_DEPTH_MM | Intake height plus uncertainty/drawdown margin |
| WATER_RESTART_DEPTH_MM | Higher recovery threshold |
| WATER_MIN_DISTANCE_MM | Model-specific saturation boundary, with conservative operating limit |
| WATER_MAX_DISTANCE_MM | Confirmed usable upper range |
| WATER_OFFSET_MM | Independently measured additive distance correction |
| WATER_TIMEOUT_MS | Freshness limit justified against update rate and drawdown |
| WATER_RECOVERY_SAMPLES | Consecutive healthy restart readings, minimum five |
| WATER_CALIBRATED | Set y only after verifying all the above |

The geometry validator requires the full-tank air gap to be strictly above the configured minimum distance, maximum operating depth below the sensor face, and restart above trip. Rejecting incompatible geometry is intentional. Reposition the sensor or revise the measured operating range rather than disabling validation.

`configs/bench_example.conf` contains hypothetical numbers and stays uncalibrated. It is not tank calibration. Calibration here is an additive distance correction plus measured geometry, not a multi-point sensor fit or volume lookup.

## Build in your Zephyr environment

Use your installed Zephyr workspace/toolchain. Current Zephyr uses this board name:

```sh
west build -p always -b nrf52840dk/nrf52840 .
west flash
```

Older versions use `nrf52840dk_nrf52840`. Check `west boards` for your installed version. Both use `app.overlay`; the historical board overlay now contains only a comment. Confirm the build output includes `app.overlay` and `farmbot,sonar-pw` resolves to P0.03.

To select a sensor and measured tank configuration:

```sh
west build -p always -b nrf52840dk/nrf52840 . -- -DEXTRA_CONF_FILE="configs/hrxl.conf;configs/tank_measured.conf"
```

Use `configs/mb1000.conf` instead for the MB1000. Change estimator through `west build -t menuconfig`, or a separate experiment config containing one of `CONFIG_WATER_FILTER_RAW=y`, `CONFIG_WATER_FILTER_MEAN=y`, `CONFIG_WATER_FILTER_MEDIAN=y`. Save exact config/build identifiers with each run. A pristine rebuild prevents accidental carry-over of settings.

This environment had no `west` or Zephyr workspace, so an actual Zephyr build, devicetree validation, flash and hardware timing check remain outstanding. Host tests do not validate Zephyr APIs or the electrical interface.

## Console and experiment capture

Use the DK debug-interface virtual serial port that carries the configured UART console. `CONFIG_SERIAL` alone does not set up a native-USB CDC device. Confirm console/baud settings in your board/SDK configuration; capture defaults to 115200 baud.

```sh
python3 tools/capture.py --port /dev/cu.YOUR_DK_PORT --output run01.csv
```

Serial capture needs `pyserial` installed in your chosen Python environment. No extra package is needed to parse saved console text:

```sh
python3 tools/capture.py --input saved_console.txt --output run01.csv
python3 tools/analyse.py run01.csv
```

Capture preserves complete console text in `run01.csv.console.txt`, adds UTC host reception timestamps and a session number incremented by the firmware boot/configuration banner. It refuses to overwrite existing logs. Device uptime is monotonic since boot, not civil time. Preserve the banner and build identifier. A logger started mid-session may not know the device boot boundary; restart capture/board deliberately at the start of a controlled run.

CSV fields are `uptime_ms,sequence,sample_event,pulse_us,raw_mm,filtered_mm,depth_mm,depth_percent,sample_age_ms,validity,permission,reason,capture_overruns`. Sample age means age of the last **accepted valid** sample. -1 means none. Heartbeats have `sample_event=0`. Processed depth/percent values on invalid/stale rows retain the previous estimate and must not be treated as current measurements. Use validity and permission together. Depth percent is relative to maximum operating depth, not volume percent.

For error analysis, add an independently measured `reference_depth_mm` column to the captured file. The analysis tool excludes heartbeat and non-VALID rows from error statistics, and reports each session separately. It does not invent references, statistical independence, ground-truth crossing times or end-to-end actuation latency. Collect these in the experiment log and analyse repeated runs as described in [the research plan](THESIS_RECOVERY_PLAN.md).

## Controller integration contract still needed

The CSV stream and DK LEDs demonstrate the local monitoring path. The eventual receiver must default to blocked on boot, lost/malformed telemetry or expired permission, then combine fresh water permission with its own irrigation demand. It must supervise ongoing watering, not only check before starting.

Confirm: stock FarmBot OS/version versus custom controller, transport (UART/USB/BLE/network), receiving message format, freshness timeout, active signal levels, and whether the interface merely inhibits watering or also stops an ongoing operation. Implement and test that adapter once these facts are known. The current firmware has no physical pump-control output and no claim of end-to-end FarmBot protection.

## Tests and verification

```sh
./tests/run.sh
```

This builds the platform-independent monitor with strict compiler warnings plus address/undefined-behaviour sanitizers and runs Python tooling tests. Tests cover pulse encoding, geometry checks, startup confirmation, raw low-water override, hysteresis, recovery, timeout boundaries, malformed/range/stale inputs, filter alternatives and reference-analysis exclusions.

Bench verification still required: correct output pin/voltage, known-distance conversion, timing under interrupts, saturation, water-surface echoes, disconnection, reboot, console throughput and LED states. Without a physical board and measured tank, this is a tested software core and firmware candidate, not a completed evaluated thesis system.

## Source references

- [Zephyr nRF52840 DK board/build documentation](https://docs.zephyrproject.org/latest/boards/nordic/nrf52840dk/doc/index.html)
- [MaxBotix HRXL datasheet](https://maxbotix.com/pages/hrxl-maxsonar-wr-datasheet)
- [MaxBotix MB1000/LV datasheet](https://maxbotix.com/pages/lv-maxsonar-ez-datasheet)
- [Nordic GPIO electrical specification](https://docs.nordicsemi.com/r/bundle/ps_nrf52840/page/gpio.html)

Record AI assistance accurately in your thesis and understand/validate every part of the code you present.
