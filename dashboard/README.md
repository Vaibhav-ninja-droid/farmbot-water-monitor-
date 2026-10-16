# FarmBot Water Lab dashboard

A browser-only dashboard for the existing Uno firmware. No firmware re-upload is needed. It reads serial CSV at 115200 baud; it does not write commands or control the pump. Sensor data and imported files stay in the tab, with no telemetry API or database upload.

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
