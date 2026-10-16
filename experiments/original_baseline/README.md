# Original processing baseline

These modules preserve the original moving average, tank classification and state-only permission logic for experimental comparison. They are excluded from the firmware build. Known limitations include arbitrary geometry, accepting zero distance as full, and no freshness supervision. Do not use their Boolean permission function for equipment control.

The new production path is `arduino/FarmbotWaterMonitor/src/monitor.c`, called by the Uno sketch. The RAW/MEAN/MEDIAN build options compare estimators under the same validity and permission policy; they do not reproduce every defect in the old implementation.
