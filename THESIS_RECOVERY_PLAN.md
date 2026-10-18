# FarmBot water monitoring: focused thesis and evidence plan

Prepared 3 October 2026.

Sensor update, 15 October: IMG_1044 identifies MB7389, 30mm1.5 housing. Active Uno input now uses its TTL serial output on sensor pin 5 to D8; MB1000 pulse input remains optional. See docs/DEMO_TWO_DAY_PLAN.md.

Implementation update, 14 October: the active firmware now targets Freenove Uno using Arduino C++ with a shared C monitoring core under `arduino/FarmbotWaterMonitor/`. Zephyr files are retained under `legacy/zephyr/`. Older implementation references below describe the initial platform; use README.md for current wiring and build steps. This is a working research plan, not a completed thesis or a claim of experimental results. Recommendations and proposed targets below require confirmation against the actual tank, sensor and supervisor expectations.

## 1. Recommended thesis

**Working title:** Design and experimental evaluation of a reliable water-availability monitoring subsystem for FarmBot.

**Research question:** How accurately and reliably can the available ultrasonic sensors estimate usable water in the existing FarmBot tank, and how do validation, filtering and hysteresis affect low-water detection and irrigation permission under disturbances and sensor or communication failures?

**Contribution to investigate:** A calibrated embedded monitor, a measured comparison of sensing/processing alternatives, and an irrigation permission interface that handles low water and unavailable data explicitly. The contribution is the justified design and experimental evidence specific to this installation. Do not claim that tank sensing itself is new or that FarmBot universally lacks it.

Core scope: water level, usable-water threshold, reading quality and freshness, local alert, structured telemetry, and one demonstrated interface to the irrigation controller. Keep soil moisture, plant-growth optimisation, machine learning, automatic refill, water quality and a large dashboard outside the core scope. Add optional work only after the core experiments are complete.

Water level answers whether water is available. Soil moisture answers whether plants need water. Your subsystem should supply a permission condition to the existing irrigation decision owner. It should not take over crop scheduling.

## 2. What the supplied material actually establishes

The proposal describes soil moisture, temperature, humidity, water level, irrigation optimisation and a dashboard. Its evaluation names accuracy and responsiveness without defining thresholds, protocols or baselines. Claims that simple thresholds are inherently inadequate, or that an RTOS ensures tasks cannot experience delays, need revision. An RTOS is an implementation choice whose timing still requires measurement.

The seminar has a more appropriate water-tank scope. Slides 12 and 18 identify useful requirements and metrics, but not numerical acceptance criteria or a completed experiment. Slide 17 records scope refinement and site inspection. Slide 19 still proposes sensor selection and prototype construction. This supports a diagnosis of limited demonstrated progress; it does not establish the examiner's exact reasoning without their feedback.

The proposal bibliography includes apparent duplicate entries [8]/[15] and [7]/[16], and [12] names a journal inconsistent with its linked DOI venue. Audit every title, author, year, venue and supporting claim before reusing references. This review has not verified every entry.

The photos establish:

| Evidence | Identification and consequence |
|---|---|
| IMG_0982 | Nordic nRF52840 DK, PCA10056 label. Suitable for continuing the current Zephyr prototype. |
| IMG_0984 | MB1000 LV-MaxSonar-EZ0. A clear model identification and useful bench comparison sensor. |
| IMG_0983 | HRXL-MaxSonar-WRC family. Handwriting appears to identify MB7389, but the exact model/revision must be checked on the unit or confirmed with the lender. |
| IMG_0985 | Freenove FNK0034 starter-kit case. Contents are not visible; do not assume a particular float switch or module is present. |
| Loan-message screenshot | Board/sensor/prototyping equipment offered; enclosure depends on final dimensions. It does not confirm that the supplied float option is a continuous level sensor. |
| Seminar tank photos | Two different tank arrangements and internal objects, tubing and a moving lid assembly are visible. Dimensions, plumbing and electrical functions cannot be inferred reliably from photographs. |

Ask whether the existing floater is a mechanical refill valve, electrical switch or another student's mechanism. Coordinate the sensor mount with the tank-cover student so lid movement does not invalidate calibration. Confirm whether both tanks feed irrigation, whether they are connected hydraulically, and which tank defines available water.

## 3. Assessment priorities and remaining time

The linked profile lists poster/demo **19–23 October 2026 (25%)** and thesis **9 November 2026, 3 pm (50%)**. Proposal and seminar weigh 10% and 15%. It specifies a satisfactory Thesis Plan as a condition for a thesis-report grade of 5 or higher, and 40% minimums for the proposal, seminar, demo and report to receive a course grade of 4 or higher. Confirm your actual marks and hurdle status in Blackboard. [Course profile](https://course-profiles.uq.edu.au/course-profiles/REIT4841-20412-7620#assessment).

Using your approximate marks, the completed contribution is 0.10 × 50 + 0.15 × 55 = **13.25/100**. With D = demo mark and T = report mark, the estimated total is **13.25 + 0.25D + 0.50T**, subject to hurdles and confirmed marks. Equal marks on the remaining items would need approximately 69% for a 65% overall result and 82.3% for a 75% overall result. These are arithmetic targets, not grade guarantees.

The publicly linked thesis marksheet allocates 45% to approach/execution, 20% to background, 15% to conclusions, and 10% each to scope and writing. Prioritise evidence, analysis and justified decisions. [Thesis marksheet](https://eecs.uq.edu.au/files/4965/Marksheet_thesis.pdf).

The linked demo sheet assigns 35% to excellence/innovation and 30% to completeness. A functioning, tested subsystem and clear results deserve substantial preparation time. [Demo marksheet](https://eecs.uq.edu.au/files/4950/Marksheet_demo.pdf). Verify that Blackboard uses these versions.

## 4. Sensor selection must follow tank geometry

The MB1000 has approximately 25.4 mm resolution and reports targets nearer than about 152 mm at its minimum range. Its pulse-width scale is 147 microseconds/inch, matching the conversion in your code. [MB1000 product](https://maxbotix.com/products/ultrasonic_sensors_mb1000), [LV datasheet](https://maxbotix.com/pages/lv-maxsonar-ez-datasheet).

The HRXL family uses 1 microsecond/mm for pulse width, not the MB1000 conversion. If the unit is MB7389, its nominal minimum reported distance is 300 mm, with model-specific timing and internal processing. A 1 mm output resolution is not a promise of 1 mm tank-level accuracy. [HRXL datasheet](https://maxbotix.com/pages/hrxl-maxsonar-wr-datasheet), [MB7389 specifications](https://maxbotix.com/products/mb7389).

Measure the air gap at maximum fill before selecting the final sensor. A weather-resistant unit can still be unsuitable if the water surface enters its minimum-range region. Test minimum-range saturation rather than interpreting it as an accurate full reading. The HRXL datasheet also cautions about close-range reliability. Leave a justified mounting margin.

Start with a flat target at known distances to verify units and wiring. Then test a water surface and finally the real tank. Compare both sensors over their common valid range, and assess their usable coverage separately. Operate one ultrasonic unit at a time unless triggering is coordinated to avoid acoustic interference.

Check the sensor pinout, DK VDD rail and signal levels before connecting. A board's USB/5 V supply capability does not imply 5 V GPIO tolerance. Keep sensor outputs within the nRF52840 GPIO operating limits, using suitable supply/interface circuitry where needed. [Nordic GPIO specification](https://docs.nordicsemi.com/r/bundle/ps_nrf52840/page/gpio.html). Do not infer wire functions from colour.

## 5. Level, volume and usable water

Measure sensor-face height above a physical datum, maximum operating water depth, pump intake/outlet height and inaccessible residual water. Record the reference instrument and uncertainty.

For a fixed overhead sensor, water depth is **h = H − d**, where H is sensor-face height above the tank-bottom datum and d is measured distance to the surface. Normalised depth is **100h/h_max**, not necessarily 100h/H when the sensor sits above the maximum fill plane.

Depth percentage and volume percentage are equal only for suitable constant-cross-section geometry. The photographed large tank appears shaped, and internal objects may displace water. If litres matter, build a measured height-versus-volume table with known added volumes and interpolate it. Keep usable volume above the intake distinct from total stored volume.

Determine the low-water trip level from intake constraints, uncertainty and worst-case consumption during decision/communication/actuation delay. A proposed margin is **h_trip ≥ h_intake + U_h + r_max × L_total + installation margin**, where r_max is measured maximum drawdown rate. This is a design rule to investigate, not a validated bound. Define restart above the trip level to create hysteresis.

## 6. Original code review and order of changes

Update: the later implementation replaces the original processing path with `arduino/FarmbotWaterMonitor/src/monitor.c`. Original filter/tank/irrigation modules are preserved in `experiments/original_baseline/` and excluded from the build. The findings below describe the original code, not a claim that the replacements have been hardware-validated.

Existing modules implement pulse capture, a five-sample moving average, depth/state calculation and a Boolean irrigation recommendation printed to the console. They do not implement physical irrigation control, FarmBot communication, continuous fault supervision or an experimental logger. Firmware build/flash and hardware timing remain unverified; `west` was not available on the review shell's PATH.

| Priority | Finding | Required development |
|---|---|---|
| 1 | `sonar.c` fixes the conversion to MB1000. | Explicit sensor configuration with correct conversion, range and expected update period. Record exact SDK/Zephyr version and matching board target. |
| 1 | `main.c` simply skips processing when no new reading arrives. | A timestamp and timeout evaluated independently of sample arrival. Start in UNKNOWN/BLOCKED. Expire old permission at the consumer too. |
| 1 | `tank_process(0)` returns FULL and allows irrigation. | Reject impossible pulse/distance values and minimum-range ambiguity before reporting a valid estimate. |
| 1 | Filtering precedes raw validity checking. | Validate each input before admitting it to the estimator. Track fault duration and sample age; do not conceal loss of validity by retaining a healthy average indefinitely. |
| 2 | Geometry is a temporary 1000 mm constant; thresholds are arbitrary. | Measured configuration and low-water rationale based on the actual intake. |
| 2 | Threshold state has no memory. | Hysteresis and explicitly defined recovery confirmation. Compare stability against response delay. |
| 2 | ISR/thread communication uses a volatile flag and shared pulse value. | Consistent sample transfer with timestamp/sequence number, paired-edge validation, and a bounded queue or suitable synchronisation. Volatile alone does not guarantee a complete sample transaction. |
| 2 | Any falling edge can use the previous rising timestamp. | Reject unpaired edges, malformed widths and out-of-range periods. Measure timing error against a scope or known pulse source. |
| 3 | Console output has no machine-readable timestamps or fault records. | CSV/structured output, configuration identifier, sample counters and reason codes. Log missing data as events. |

Host reproduction using the existing C modules confirmed that zero distance produces FULL/permission true. Four 500 mm readings followed by an invalid 1100 mm reading produce a 620 mm average, OK/permission true. This demonstrates that raw rejection currently occurs too late. It does not prove an actual sensor generates those inputs.

Keep validity (VALID, STALE, INVALID, UNKNOWN) separate from level state (LOW, OK, FULL). A fresh but stuck plausible reading is another failure class: timestamps alone cannot detect it. Test obstruction and stuck-value limitations and consider an independent float switch if available. A disconnected switch is not automatically detectable without an appropriate circuit.

## 7. FarmBot compatibility and integration decision

FarmBot publishes hardware designs, software and developer resources. Use them as interface references, while identifying the UQ installation's actual hardware and software versions. [Open-source entry point](https://farm.bot/pages/open-source).

Choose one integration route early:

1. **Custom university controller:** agree a telemetry and permission interface with the irrigation/data-services student. This is likely the simplest path if the group uses nRF/Zephyr subsystems rather than stock FarmBot OS.
2. **Stock FarmBot deployment:** first demonstrate telemetry reception through an available gateway, then implement a supported control path for the installed version. Official examples show remote RPC over MQTT, and software documentation describes pin-based sensors and sequence commands. Creating an API sensor record alone does not create an irrigation interlock. [Official remote-control example](https://github.com/FarmBot-Labs/remote-control-example), [sensor/peripheral commands](https://software.farm.bot/v15/app/sequences/sequence-commands/peripherals-and-sensors.html).

Recommended initial transport is the DK's available serial connection to a laptop or gateway. BLE is optional if the group already requires it. Verify USB/UART configuration rather than assuming `CONFIG_SERIAL` creates every desired USB interface.

Proposed message fields: schema version, device ID, boot/session ID, sequence number, uptime, sensor model, raw distance, estimated depth, optional calibrated volume, validity, sample age, level state, permission and reason. A receiver can use its own monotonic reception clock plus boot/sequence checks to supervise freshness; do not compare unrelated device uptime to wall time.

Irrigation demand remains with the irrigation controller. Actuation requires demand AND valid water permission. The consumer defaults to blocked after startup, timeout or gateway failure. Test ongoing irrigation supervision as well as checks before watering starts. A check made only at the beginning can miss depletion during a long watering sequence.

Use an LED or simulated load to verify the full command path first. Actual equipment integration must use the agreed controller interface and appropriate driver circuitry. Console text that says BLOCKED establishes only a software decision. Report separately: monitor decision, gateway receipt, controller decision and observed actuation. Communication-triggered inhibition cannot guarantee protection during all controller/power failures without an independent mechanism.

## 8. Experiments that supply thesis substance

Freeze calibration and processing parameters before final validation. Use independent refill/drain runs for validation. Thousands of consecutive readings at one unchanged level are not thousands of independent experiments.

| Experiment | Procedure | Evidence and metrics |
|---|---|---|
| E1: Sensor/units verification | Flat target at 5–8 distances covering the intended range, including near-range limits. Repeat after remounting. | Distance bias, spread, missing readings and conversion correctness. |
| E2: Tank calibration | Approximately 8–10 known levels across the usable range, extra points around the trip level. At least 3 independent fill/drain runs where practical. Record a 30–60 s block per stable level. | Reference versus measured depth, MAE/RMSE, worst error and per-level bias. Report reference uncertainty. |
| E3: Sensor and mounting comparison | Compare supplied sensors at the same reference levels. Test a clear vertical mount and a plausible constrained mount. Keep other variables fixed. | Valid coverage, false echoes, dropout rate, cost/mounting tradeoff. Record power and model details. |
| E4: Processing comparison | Apply raw threshold, current moving average, and proposed validation/median/hysteresis variants to identical recorded traces. Tune on calibration traces; evaluate on held-out runs. | Accuracy, false permission, false block, number of state changes and response latency. Separate filtering effects from hysteresis effects with ablations. |
| E5: Real disturbances | Still water, filling, pump drawdown, gentle surface agitation, lid positions and internal-object obstruction. Use controlled, repeatable conditions. | Time traces, settling time, errors and failures. Avoid wetting electronics outside their rating. |
| E6: Fault and recovery | Disconnect sensor signal, suppress pulses, replay malformed/zero/out-of-range pulses, interrupt gateway link, restart controller, and test a plausible stuck reading. Repeat each scenario. | Fault-to-block latency, false permissions, recovery sequence and documented undetectable failures. Distinguish injected faults from physical faults. |
| E7: End-to-end permission | Demand watering with sufficient water, lower water below trip, restore water above restart, then interrupt telemetry. | Timestamped decision/acknowledgement/output changes. Demonstrate on LED/simulator first, then actual agreed interface if available. |
| E8: Soak | At least a few hours on the bench, followed by 24–48 h if access permits. | Valid-data fraction, longest outage, resets and drift. A short soak does not establish seasonal outdoor reliability. |

Define false permission as allowing irrigation while reference usable water is below the agreed trip boundary, or while validity rules require blocking. Define false block relative to the intended policy, including the hysteresis recovery band, rather than a single instantaneous threshold.

**Proposed initial acceptance targets, to agree before testing:** block on boot and expired data; block within 1 s after a declared sensor/link fault; zero false permissions in a declared finite fault/low-water test set; enough accuracy that the measured uncertainty fits inside the chosen intake margin. Adjust the 1 s target from actual update rate and hydraulic risk before freezing it. Report failures rather than changing targets after seeing final results.

For a numerical level-error target, choose a physically justified tolerance after geometry inspection. A blanket 5 mm requirement is not credible for the MB1000's one-inch resolution. Report a 95th-percentile error or run-level confidence interval where justified, not just a favourable average. With zero failures in N independent trials, the approximate 95% upper failure-rate bound is 3/N; zero observed failures does not prove zero risk.

## 9. Literature that informs decisions

Build a comparison table with columns for sensing principle, environment, tested range, reference method, reported errors, fault handling, limitation and design consequence for your tank. Separate manufacturer specifications from independent research.

Useful starting leads identified during this review:

- [Evaluation of Water Level in Flowing Channels Using Ultrasonic Sensors](https://www.mdpi.com/2071-1050/14/9/5512): investigate its disturbance tests and error reporting. A channel is different from your enclosed tank.
- [A Novel Temperature Drift Compensation Algorithm for Liquid-Level Measurement Systems](https://www.mdpi.com/2072-666X/16/1/24): examine temperature effects and reference measurement, checking its acoustic path before applying its method to an air-gap sensor.
- [Water Quality Monitoring and Management of Building Water Tank Using Industrial Internet of Things](https://www.mdpi.com/2071-1050/13/15/8452): examine tank-monitoring architecture and calibration; distinguish level monitoring from its wider water-quality scope.

These are reading leads, not a completed critical review. Publisher search results were available but full-text retrieval was unsuccessful in this session. Verify full methods, numbers and bibliographic details before citing findings. Do not copy performance claims between different sensing principles.

Compare ultrasonic sensing with a discrete float switch, continuous float-based transducer and pressure sensing conceptually. A single float switch supplies a threshold state, not continuous depth. You need not purchase every alternative: justify the comparison from sources and test the equipment you actually have.

Replace generic claims about 'intelligent agriculture' with evidence that directly motivates a design choice: mounting clearance motivates sensor selection; splash traces motivate filtering; false toggles motivate hysteresis; loss of pulses motivates timeout supervision.

## 10. Schedule from 3 October

Dates are Brisbane local dates and assume the linked course schedule applies.

| Dates | Deliverable |
|---|---|
| 3–5 Oct | Confirm Thesis Plan/hurdles, obtain feedback, agree narrow scope and integration owner, identify HRXL model, measure tank/intake/lid clearances. Verify known-distance readings. |
| 6–9 Oct | Implement validity/freshness, measured geometry, hysteresis and structured logging. Complete first calibration and mounting trial. Write methods while doing them. |
| 10–13 Oct | Run held-out sensor/filter/disturbance comparisons and repeated fault tests. Establish permission interface on a simulated load or agreed controller. |
| 14–16 Oct | Finish initial end-to-end tests and soak. Produce real plots, results table, conclusions and limitation statements. Freeze demo firmware. |
| 17–18 Oct | Rehearse normal, low-water, disconnected-sensor and recovery cases. Prepare A3 poster, backup video and raw logs. |
| 19–23 Oct | Scheduled demo; submit poster by the profile's stated deadline. Record examiner feedback. |
| 24–30 Oct | Resolve important gaps, complete remaining validation, write results/discussion. Avoid adding major features. |
| 31 Oct–5 Nov | Complete thesis draft, references, figures, appendices and reproducibility package. Seek supervisor review in time. |
| 6–8 Nov | Revise, check every claim against evidence, verify submission format and files. |
| 9 Nov | Submit before the listed 3 pm deadline. |

If actual FarmBot access is delayed, continue the bench calibration, processing comparisons and fault experiments immediately. Describe a simulated interface honestly. If no usable tank/sensor geometry exists by 7 October, agree a revised rig or scope with the supervisor instead of losing the remaining experiment window.

## 11. Thesis and demonstration structure

Suggested thesis chapters: problem and measurable objectives; focused background and alternatives; installation requirements; sensor/firmware/interface design and reasons; reproducible experimental methods; measured results; discussion of tradeoffs/failures and integration limits; conclusions mapped to each objective.

Essential figures: dimensioned tank/mount sketch; system/interface diagram; state transition diagram; calibration plot with identity line; error versus level; disturbed-water raw/processed traces; low-water and disconnect latency traces; end-to-end event trace. Label every plot with units, sensor model, condition and run count.

An effective demonstration story: show the real constraint, explain your chosen design and comparison, display measured outcomes, then demonstrate adequate water, low water, lost signal and recovery. The poster should state what you actually achieved. If actuation is simulated, label it clearly.

Your research becomes substantive when you can answer: Why this sensor? Why this mount? Why this filter and timeout? How much error remains? Which failures does the system detect? What happens during watering if the data stops? What evidence supports each answer?

## 12. Evidence records and immediate next actions

Use `docs/experiment_log_template.csv` for runs and `docs/sample_log_schema.csv` as a proposed telemetry/log schema. These files contain headers only, not results. Keep original traces, reference readings, setup photographs, configuration files, firmware identifiers and analysis scripts together. Record failed runs too.

Next bench session:

1. Identify the exact HRXL model and read its pinout; inspect kit contents.
2. Verify signal voltage, ground and pin mapping; verify distance conversion with a ruler/flat target.
3. Measure actual tank geometry and minimum full-tank air gap.
4. Capture the first labelled raw-data run with an independent reference.
5. Establish what the irrigation controller will do with LOW, UNKNOWN and STALE messages.




-------------------------

python3 -m http.server 8765 --bind 127.0.0.1 --directory dashboard/dist
-------------------------