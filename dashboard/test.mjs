import assert from "node:assert/strict";
import { parseLine, presentation, FIELDS } from "./dist/core.mjs";
const packet = parseLine("205,2,1,0,300,0,0,0,-1,UNCONFIGURED,0,CONFIG,0");
assert.equal(presentation(packet.row).kind, "warning");
assert.match(presentation(packet.row).title, /too close/);
const unconfigured = parseLine(
  "12599,85,1,0,1707,0,0,0,-1,UNCONFIGURED,0,CONFIG,0",
).row;
assert.equal(presentation(unconfigured).levelValid, false);
assert.match(presentation(unconfigured).title, /not configured/);
const valid = parseLine("900,6,1,0,1000,1000,500,50,0,VALID,1,READY,0").row;
assert.equal(presentation(valid).allowed, true);
assert.equal(presentation(valid, {}, false).allowed, false);
assert.equal(presentation({ ...valid, sample_age_ms: 1000 }).levelValid, false);
assert.match(presentation({ ...valid, raw_mm: 5000 }).title, /No usable/);
assert.equal(parseLine("garbage"), null);
assert.equal(parseLine(FIELDS.join(",")).type, "header");
assert.equal(parseLine("900,6,1,0,1000,1000,500,50,0,VALID,2,READY,0"), null);
assert.equal(
  parseLine("900,6,1,0,1000,1000,500,50,0,<script>,1,READY,0"),
  null,
);
const config = parseLine(
  "# water-monitor,v1,sensor=HRXL,calibrated=0,height=0",
);
assert.equal(config.config.calibrated, "0");
assert.equal(
  parseLine(
    "2026-10-16T00:00:00Z,1,900,6,1,0,1000,1000,500,50,0,VALID,1,READY,0",
    ["host_receive_time_utc", "session", ...FIELDS],
  ).row.raw_mm,
  1000,
);
console.log(
  "PASS: firmware/capture CSV parsing, configuration metadata, close/no-target warnings, depth validity, stale data and malformed input",
);

const { validateCalibration, CalibrationPreview, generateConfig } =
  await import("./dist/calibration.mjs");
const bucket = {
  height: 1000,
  max_depth: 400,
  trip: 100,
  restart: 150,
  high: 350,
};
assert.deepEqual(validateCalibration(bucket).errors, []);
for (const c of [
  { ...bucket, height: "" },
  { ...bucket, height: "1e3" },
  { ...bucket, height: 600 },
  { ...bucket, trip: 150 },
  { ...bucket, restart: 401 },
])
  assert.ok(validateCalibration(c).errors.length);
assert.throws(() => generateConfig({ ...bucket, height: "1000;evil()" }));
assert.match(generateConfig(bucket), /c.calibrated = true;/);
assert.match(generateConfig(bucket), /c.sensor_height_mm = 1000;/);
const preview = new CalibrationPreview(bucket);
const raw = (d) => ({ ...unconfigured, raw_mm: d, sample_event: 1 });
for (let i = 0; i < 4; i++) preview.update(raw(800), i * 150);
assert.equal(preview.result.allowed, false);
preview.update(raw(800), 600);
assert.equal(preview.result.depth, 200);
assert.equal(preview.result.percent, 50);
assert.equal(preview.result.allowed, true);
preview.update(raw(900), 750);
assert.equal(preview.result.label, "LOW WATER");
assert.equal(preview.result.allowed, false);
preview.update(raw(880), 900);
assert.equal(preview.result.allowed, false);
for (let i = 0; i < 5; i++) preview.update(raw(800), 1050 + i * 150);
assert.equal(preview.result.allowed, true);
preview.tick(2650);
assert.equal(preview.result.valid, false);
preview.update(raw(800), 2800);
assert.equal(preview.result.allowed, false);
preview.update(raw(300), 2950);
assert.equal(preview.result.valid, false);
preview.update(raw(5000), 3100);
assert.equal(preview.result.valid, false);
preview.update(
  { ...raw(800), sample_event: 0, validity: "INVALID", reason: "CAPTURE" },
  3250,
);
assert.equal(preview.result.valid, false);
preview.reset();
for (let i = 0; i < 5; i++) preview.update(raw(800), i * 150);
preview.update(raw(800), 3000);
assert.equal(preview.result.allowed, false); // gap must restart confirmation
console.log(
  "PASS: measured geometry validation, generated firmware settings, browser depth/percentage, low-water trip, hysteresis, recovery, invalid inputs and expiry",
);

const { savedCalibrationMatches } = await import("./dist/calibration.mjs");
const reported = {
  usb_cal: "2",
  storage: "EEPROM",
  calibrated: "1",
  height: "1000",
  max_depth: "400",
  trip: "100",
  restart: "150",
  high: "350",
};
assert.equal(savedCalibrationMatches(reported, bucket, true), true);
assert.equal(savedCalibrationMatches(reported, bucket, false), false);
assert.equal(
  savedCalibrationMatches({ ...reported, height: "999" }, bucket, true),
  false,
);
assert.equal(
  savedCalibrationMatches({ ...reported, storage: "DEFAULT" }, bucket, true),
  false,
);
assert.equal(
  savedCalibrationMatches({ ...reported, usb_cal: undefined }, bucket, true),
  false,
);
console.log(
  "PASS: save confirmation requires acknowledgement, capability, EEPROM storage and all matching fields",
);

assert.ok(validateCalibration({ ...bucket, high: 149 }).errors.length);
assert.ok(validateCalibration({ ...bucket, high: 401 }).errors.length);
assert.match(generateConfig(bucket), /c.high_depth_mm = 350;/);
const hp = new CalibrationPreview(bucket);
for (let i = 0; i < 5; i++) hp.update(raw(800), i * 150);
hp.update(raw(650), 750);
assert.equal(hp.result.label, "HIGH WATER");
assert.equal(hp.result.allowed, true);
hp.update(raw(660), 900);
assert.equal(hp.result.label, "HIGH WATER");
for (let i = 0; i < 5; i++) hp.update(raw(680), 1050 + i * 150);
assert.equal(hp.result.label, "Water available");
hp.tick(3000);
assert.equal(hp.result.valid, false);
const highRow = parseLine("900,6,1,0,650,650,350,87,0,VALID,1,HIGH,0").row;
assert.match(presentation(highRow).title, /HIGH WATER/);
assert.equal(presentation(highRow).allowed, true);
assert.equal(presentation(highRow, {}, false).levelValid, false);
assert.equal(
  savedCalibrationMatches({ ...reported, high: "340" }, bucket, true),
  false,
);
console.log(
  "PASS: high warning validation, raw trip, clearing hysteresis, stale handling, parsing and save confirmation",
);
