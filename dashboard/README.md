# FarmBot Water Lab dashboard

A browser-only dashboard for the existing Uno firmware. Raw viewing works with existing firmware. Direct calibration saving requires one upload of the updated sketch. It reads serial CSV at 115200 baud and sends calibration commands only through the explicit Save button (plus an automatic configuration query); it does not control a pump. Sensor data and imported files stay in the tab, with no telemetry API or database upload.

## Live use

Start the local server below and open its URL in desktop Chrome or Edge, in its own tab. Private hosting has been registered but source upload/publication awaits explicit user authorization. Close the Terminal/Arduino serial monitor first (Ctrl+C in Terminal). Click **Connect Arduino**, choose the USB serial device (previously `usbserial-1310`), and allow the connection. Connecting may reset the Uno; wait for the boot banner and measurements.

Embedded previews, Safari and Firefox may not expose Web Serial. Import logs still works without serial support. The browser requires a secure context; the deployed HTTPS site and localhost both qualify.

## Offline/local use

The source has no external runtime dependencies or fonts. In Terminal:

```sh
cd /Users/vaibhavrana/farmbot-water-monitor
python3 -m http.server 8765 --bind 127.0.0.1 --directory dashboard/dist
```

Then open http://127.0.0.1:8765 in desktop Chrome or Edge. Keep that terminal running. Stop it with Ctrl+C after use.

## What it displays

Raw sensor distance, firmware-accepted depth/percentage, local irrigation permission, accepted sample age, status changes, firmware geometry and a rolling 60-second chart. At/below minimum distance, out-of-range/no-target readings, firmware faults and stale USB data suppress the water fill/permission display. Unconfigured firmware shows distance only. Values retained after disconnection are labelled historical and cannot show permission as allowed.

The dashboard freshness watchdog is two seconds since the last parsed row; firmware also has its own accepted-sample age and timeout. Browser background throttling, sensor internal filtering and SoftwareSerial timing mean this is not a safety controller or a precise latency instrument. An allowed display does not establish actual pump actuation.

**Import log** accepts the raw firmware console text or the capture tool's CSV, up to 15 MB. Replayed files are explicitly historical. **Export CSV** saves up to the most recent 10,000 retained records, adding browser reception time and session. It contains measurements, not calibration metadata; preserve firmware/config files separately. Error statistics remain in tools/analyse.py.

**Try illustrative data** is explicitly synthetic. It is off by default and export is disabled while it runs. Never use these readings as thesis results.

## Verification

Run `node dashboard/test.mjs` for parsing, unconfigured/too-close/no-target warnings, stale data, valid depth and permission states. `node --check dashboard/dist/app.js` checks browser script syntax. Hardware connection requires the user's browser port selection and physical device. Local data has not been fabricated to claim hardware validation.

Browser QA: inspected desktop/narrow layouts, exercised explicitly labelled illustration mode and stopping it, and imported the supplied 480-record console log. The final no-target/unconfigured state was correctly shown. Live USB hardware access has not been verified from the development browser.

## Guided bucket calibration (17 October)

The new form takes four actual measurements in millimetres: sensor face to bottom, maximum water depth, low-water depth and recovery depth. It rejects invalid numbers, insufficient full-water clearance, and inconsistent thresholds. Keep the sensor fixed after measuring. A gap just above 300 mm satisfies the firmware check but still needs practical testing; prefer more clearance where possible.

**Start monitoring** starts a separate labelled calculation from new live sample events: five-reading median, immediate raw low-water trip, hysteresis, five consecutive recovery samples and 1 s expiry. It may use distance from an UNCONFIGURED firmware row, but blocks on faulty/out-of-range readings. Replayed data is labelled recorded and illustrative data cannot be used for this preview. Changing data sources/reboot clears recovery history. The preview does not turn the Uno LED on or change actual firmware permission.

**Download config.h** generates a complete Uno/MB7389 serial configuration with the entered measurements, zero offset, median filter, 1 s timeout and five recovery samples. For firmware without saved EEPROM geometry, replace `arduino/FarmbotWaterMonitor/config.h` with it, disconnect browser serial, compile/upload and reconnect. Existing EEPROM geometry overrides this file; use direct Save to change those values. Check the firmware settings panel and compare water depth with an independent ruler. The generated file is for the bucket you measured, not automatically for the rooftop tanks. CSV export continues to contain firmware measurements/permission, not browser-preview results. Preserve the downloaded config with your experiment log.

Validation tests cover geometry, preview trip/recovery/timeout and generated settings. A generated file with clearly hypothetical test dimensions compiled successfully for Uno; these dimensions were not applied to the real firmware. Browser QA verified an invalid-clearance rejection and successful application of valid test dimensions. Physical bucket validation remains required.

### Manual calibration and browser monitoring
Automatic empty-reference capture has been removed. Fix the sensor and measure from its sensing face to the inside bottom using a ruler. Enter this distance, maximum water depth and low/recovery thresholds, then **Start monitoring**. Use **Save calibration to Arduino** to persist confirmed settings and enable firmware decisions and the L LED. Editing measurements pauses browser monitoring until reapplied. Saved EEPROM settings survive power cycles; unsaved browser fields do not survive refresh. Keep more than 300 mm clearance at maximum fill and remeasure after moving the sensor.

The browser and generated firmware use a five-sample median to suppress isolated echo spikes, with immediate raw low-water blocking and five healthy recovery samples. No Kalman filter is applied; tuning a model without measured ultrasonic noise or dynamics can conceal errors and add lag.


### Direct USB calibration (one-time firmware update required)
After uploading the current sketch once, connect and apply measured browser settings, then click **Save calibration to Arduino**. The button is enabled only on live USB with a `usb_cal=1` capability banner. The dashboard sends `CAL,SAVE,height,max_depth,trip,restart` followed by LF. Firmware validates all fields and geometry, blocks permission while saving, stores a versioned CRC16 record in EEPROM bytes 0–12, verifies readback, resets the monitoring core and emits `# cal,saved` plus its active configuration. The browser requires the acknowledgement and matching EEPROM-backed settings before claiming success. `CAL?` requests an active banner. Commands are length-bounded, reject malformed input and expire partial lines after 1 second; the UI times out confirmation after 6 seconds.

On startup, valid saved geometry overrides config.h geometry. Invalid EEPROM falls back to config.h. Writes use EEPROM.update and skip unchanged records; writing invalidates the magic byte first so an interrupted write is rejected on boot. This is corruption detection, not a redundant transactional backup: power loss during writing may lose the previous saved record. Recheck geometry after remounting or changing sensors. Save only on explicit button clicks; do not save every sample. Uploaded code still determines sensor type, pins, filter and timeout. Changes to those still require a firmware upload. Saved settings survive browser refresh and Uno power cycles; browser preview fields themselves are not persisted. Check the firmware settings panel after reconnecting. No pump output is implemented.

Hardware validation still required: save measured geometry, check the confirmation and main metrics/L LED, unplug/reconnect the Uno, then verify identical active settings without saving again. Only software build, protocol/record checks and UI loading have been verified here.
