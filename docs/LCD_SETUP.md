# Local I2C LCD display

The current Uno firmware includes an LCD driver using the bundled Arduino Wire library. No additional library installation is required. It targets the PCF8574/PCF8574A backpack pictured in IMG_1106: RS=P0, RW=P1, Enable=P2, backlight=P3, data=P4–P7 (the common [LiquidCrystal_I2C mapping](https://github.com/johnrickman/LiquidCrystal_I2C/blob/master/LiquidCrystal_I2C.h)). It uses the first 16 characters of the first two rows, also suitable for a larger 20x4 screen. Exact module geometry/mapping remains a hardware assumption to confirm. It is output-only and does not change measurement, thresholds, EEPROM record format or pump control.

## Wiring (USB and other power disconnected)

Use the four pins on the black adapter board, following their printed labels:

| LCD | Uno |
|---|---|
| GND | GND |
| VCC | 5V |
| SDA | A4 |
| SCL | A5 |

With the LCD back facing you and four pins at the top as in IMG_1106, the labels run left to right GND, VCC, SDA, SCL. Verify the labels rather than wire colours. Leave ultrasonic sensor TX on D8 and its control wire disconnected. Sensor and LCD share 5V and GND: use a breadboard rail or proper splitter for 5V, not two wires forced into one Uno socket. Connect with power off and check polarity before USB power. A4/A5 are labelled in the ANALOG IN header; do not confuse them with D4/D5.

## Upload

Disconnect the dashboard/other serial monitors first. Use the existing Arduino CLI installation:

```sh
cd /Users/vaibhavrana/farmbot-water-monitor
export ARDUINO_DIRECTORIES_DATA=/private/tmp/farmbot-arduino-check/data
export ARDUINO_DIRECTORIES_DOWNLOADS=/private/tmp/farmbot-arduino-check/downloads
export ARDUINO_DIRECTORIES_USER=/private/tmp/farmbot-arduino-check/user
task_cli=/private/tmp/farmbot-arduino-check/arduino-cli
"$task_cli" board list
# Replace this if board list shows a different USB serial port.
task_port=/dev/cu.usbserial-1320
"$task_cli" compile --build-path /private/tmp/farmbot-lcd-build --fqbn arduino:avr:uno arduino/FarmbotWaterMonitor
# Run only after compilation succeeds:
"$task_cli" upload --fqbn arduino:avr:uno --port "$task_port" arduino/FarmbotWaterMonitor
```

Reconnect USB/dashboard after upload. Ordinary sketch upload preserves saved EEPROM calibration on this Uno workflow. Confirm its values in the dashboard; if no calibration was saved, enter measured values and use Save calibration to Arduino.

## Expected text

- No valid geometry: `Calibration req.` / `Save via USB`.
- Waiting for first reading: `Waiting sensor` / `Water blocked`.
- Valid depth: `Depth:  75 mm` / `Water available`, `LOW WATER`, `HIGH WATER` or `Recovering...`.
- Stale/invalid measurement: `Sensor timeout` or `Sensor fault` / `Water blocked`. Last good depth is not presented as current.

Refresh is approximately twice per second, writing changed characters incrementally rather than clearing the entire screen. The firmware scans only PCF8574 ranges 0x20–0x27 and 0x38–0x3F and uses the first responding device; attach only this LCD to I2C for this setup. Boot diagnostics in dashboard raw output show `# lcd,address=0x...` or `# lcd,not_found,...`. A missing LCD does not prevent monitoring. A bus error disables LCD writes until reset; a 3 ms Wire timeout prevents indefinite I2C blocking. Physical UART/I2C coexistence still needs testing.

If backlight is on but no text, slowly turn the blue contrast screw through its adjustment range without forcing its stops. Check the backlight jumper is fitted. If not detected, unplug power, check wiring, reconnect. If detection succeeds but contrast adjustment never reveals text, the backpack mapping/module may differ: report the raw LCD diagnostic and module markings. No actual LCD/power-cycle test has been performed by the agent.

## Demo check

Compare LCD depth to the main firmware depth card and a ruler. With saved thresholds 30/60 mm and suitable geometry, an empty/20 mm container should show LOW WATER and L off. At 70–80 mm depth, five healthy recovery samples should lead to Water available and L on. Lower to 20 mm and confirm LOW WATER/L off. Disconnect only dashboard serial (keep Uno powered) and confirm the LCD continues updating. Power cycle and confirm the saved geometry is restored. Do not claim accuracy from the LCD alone.

High warning is advisory and does not switch the L LED off by itself. Test by raising water to the configured high threshold while remaining at/below maximum operating depth. Compare LCD HIGH WATER with the dashboard banner. Lower below its clearing threshold and wait for the median to settle.

### High-water firmware build (19 October)
The earlier temporary core directory may no longer contain the AVR core. The high-warning build was verified against the installed core using:

```sh
export ARDUINO_DIRECTORIES_DATA="$HOME/Library/Arduino15"
export ARDUINO_DIRECTORIES_USER="$HOME/Documents/Arduino"
export ARDUINO_DIRECTORIES_DOWNLOADS=/private/tmp/farmbot-arduino-check/downloads
/private/tmp/farmbot-arduino-check/arduino-cli compile --build-path /private/tmp/farmbot-high-build --fqbn arduino:avr:uno arduino/FarmbotWaterMonitor
```

Use the same environment for board list and upload. Upload once, refresh the dashboard, enter all five values and Save to Arduino. Example high=90 mm applies only when maximum is 100 mm, recovery is at most 90 mm, and the sensor has adequate air clearance. Test HIGH at 90–100 mm, then lower below 85 mm to clear (allow median settling). Above maximum remains an invalid-range fault; do not treat that as an overflow detector. Existing saved v1 geometry is loaded with a derived high threshold as documented in dashboard/README.md; review and save the intended value explicitly.
