# Component checklist — Uno + MB7389, 15 October 2026

## What the tank photos establish

IMG_9864 shows a large shaped tank and a smaller vessel with a geared lid assembly. IMG_9869 shows the smaller vessel more closely. IMG_9871 shows internal objects, wiring and pipework. Their functions, hydraulic connection, dimensions and electrical ratings are not established by the photos. Do not infer that the smaller vessel is the irrigation reservoir or that existing equipment can be rewired.

Use a fixed sensor mount with a clear vertical acoustic path. Keep the sensor away from the moving lid mechanism and clear of internal obstructions. Confirm the water-to-sensor gap at maximum fill before committing to MB7389. An elevated bracket may be necessary; its dimensions cannot be selected from these photos.

## Minimum working monitor and demo

| Quantity | Item | Status/purpose |
|---|---|---|
| 1 | Freenove Uno-compatible ATmega328P board | Pictured; main controller |
| 1 | MB7389 ultrasonic sensor | Pictured; primary candidate |
| 1 | USB data cable matching the pictured USB-C board socket | Needed for upload, power and laptop logging; verify cable carries data |
| 1 | Laptop with Arduino IDE and capture tools | Upload, configuration and display/logging |
| At least 3 | Suitable insulated connection leads/extensions | Power, ground, MB7389 serial signal to Uno D8; verify sensor pads |
| 1 | Small breadboard or terminal breakout | Useful if direct leads do not reach; keep dry and secured |
| 1 set | Secure temporary bracket/crossbar/clamps | Hold sensor face downward at a fixed measured height |
| As needed | Fasteners, cable ties, strain relief and insulation | Prevent loose connections and protect unused wires |
| 1 | Dry enclosure or splash-protected electronics box | Protect Uno and connections; sensor weather rating does not protect board/connectors |
| 1 | Measuring tape or rigid ruler, plus a marked dipstick if necessary | Reference water level and mounting geometry |
| 1 | Multimeter (lab instrument is sufficient) | Confirm power and wiring before connection |
| 1 set | Water access, bucket/jug and approved drain/siphon arrangement | Repeated reference levels and low-water trials; existing safe equipment may suffice |
| 1 | Phone/camera | Setup photographs and backup demonstration video |

The Uno's built-in D13 LED is enough to show permission. No extra LED, display or buzzer is required by the current firmware. An LED demonstrates a decision, not actual pump shutdown.

## Existing backup / comparison equipment

- MB1000: pictured; optional comparison. Use the documented pulse profile and D2 wiring, not the MB7389 serial decoder. Keep exposed circuitry dry.
- nRF52840 DK: previously pictured; not required for the selected Uno demo.
- Freenove starter kit: useful source of leads, breadboard and components, but inventory contents rather than assuming they are present.

## Outdoor completion after a successful bench demonstration

A correctly sized enclosure with cable glands, suitable connectors, secure mounting, strain relief and a verified stable supply will be needed for a maintained outdoor installation. Reuse suitable existing items only after checking condition and suitability. A 3D-printed box is not automatically weatherproof. Keep the acoustic face unobstructed; do not cover it with an ordinary box lid or film.

## Only if actual pump/valve control is required

Obtain the existing controller interface and pump/valve voltage, normal current, startup current and type. Then select a compatible controller interface or rated driver, supply, protection and connectors. A DC inductive load may require a flyback arrangement, depending on the chosen driver. Do not select a relay/MOSFET by guesswork, connect a pump to an Uno pin, or modify unknown mains equipment.

Current firmware outputs serial status and D13 indication only; it does not include a physical actuator interface. Those additional parts and firmware cannot be finalised from photographs.

## One or two tanks?

Current firmware monitors ONE sensor and ONE tank configuration. One MB7389 can be moved between tanks sequentially, but each mounted position/tank needs its own measured settings and labelled logs. Simultaneous monitoring of both tanks needs another suitable sensor/input and code changes; the MB1000 is not a drop-in second channel. Hydraulic connection and the actual irrigation water source need confirmation first.

## Measurements to bring back before demo configuration

For the selected tank: sensor-face-to-bottom datum, maximum operating water depth, intake/outlet height, full-tank air gap, clear mounting area and any lid movement. Define low-water and restart depths from the actual setup. Diameter/shape and volume calibration matter only if reporting litres; do not delay depth-only monitoring to invent a volume model.

Photos cannot supply these measurements. Until configured, firmware can log distance but blocks water-availability permission. Aim to measure/configure before demo day, then validate on the same mounting position.
