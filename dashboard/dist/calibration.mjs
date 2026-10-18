// Browser calculations are previews. They never write settings to the Arduino.
export function validateCalibration(values) {
  const errors=[];
  const c={};
  for(const key of ['height','max_depth','trip','restart']) {
    const text=String(values[key]??'').trim();
    c[key]=Number(text);
    if(!/^\d+$/.test(text)||!Number.isSafeInteger(c[key])||c[key]<=0||c[key]>4999)
      errors.push(`${({height:'Sensor height',max_depth:'Maximum depth',trip:'Low-water depth',restart:'Recovery depth'})[key]} must be a whole number from 1 to 4999 mm.`);
  }
  if(!errors.length) {
    if(c.height-c.max_depth<=300) errors.push('The air gap at maximum fill must be greater than 300 mm. Raise the sensor or reduce the maximum operating fill.');
    if(c.trip>=c.restart) errors.push('Recovery depth must be higher than low-water depth.');
    if(c.restart>c.max_depth) errors.push('Recovery depth cannot exceed maximum water depth.');
  }
  return {config:c,errors};
}
export class CalibrationPreview {
  constructor(config){this.config=config;this.reset();}
  reset(){this.samples=[];this.allowed=false;this.recovery=0;this.lastSample=null;this.result={valid:false,allowed:false,label:'Waiting for fresh samples',depth:null,percent:null};}
  block(label){this.samples=[];this.recovery=0;this.allowed=false;this.result={valid:false,allowed:false,label,depth:null,percent:null};return this.result;}
  update(row,now){
    this.tick(now);
    if(!['VALID','UNCONFIGURED'].includes(row.validity))return this.block('Firmware fault — check sensor');
    if(!row.sample_event)return this.result;
    const d=row.raw_mm,c=this.config;
    if(!['VALID','UNCONFIGURED'].includes(row.validity)||!['CONFIG','READY','LOW','RECOVERING'].includes(row.reason))return this.block('Firmware fault — check sensor');
    if(d<=300||d>=5000||d>c.height||c.height-d>c.max_depth)return this.block('Outside the calibrated measurement range');
    this.lastSample=now;
    this.samples.push(d);this.samples=this.samples.slice(-5);
    const sorted=[...this.samples].sort((a,b)=>a-b);
    const depth=c.height-sorted[Math.floor(sorted.length/2)],rawDepth=c.height-d;
    let label;
    if(rawDepth<=c.trip||depth<=c.trip){this.allowed=false;this.recovery=0;label='LOW WATER';}
    else if(!this.allowed){this.recovery=rawDepth>=c.restart&&depth>=c.restart?this.recovery+1:0;this.allowed=this.recovery>=5;label=this.allowed?'Water available':'Waiting for recovery';}
    else label='Water available';
    this.result={valid:true,allowed:this.allowed,label,depth,percent:Math.floor(depth*100/c.max_depth)};
    return this.result;
  }
  tick(now){if(this.lastSample!==null&&now-this.lastSample>=1000)return this.block('Sensor data expired');return this.result;}
}
export function generateConfig(c){
  const checked=validateCalibration(c);if(checked.errors.length)throw new Error(checked.errors.join(' '));
  return `#ifndef WATER_CONFIG_H
#define WATER_CONFIG_H
#include "src/monitor.h"
// Generated for Uno + MB7389 TTL serial. Verify physical measurements before use.
#ifndef WATER_USE_HRXL_SERIAL
#define WATER_USE_HRXL_SERIAL 1
#endif
static const uint8_t SENSOR_SERIAL_RX_PIN = 8;
static const uint8_t SENSOR_SERIAL_UNUSED_TX_PIN = 9;
static const uint8_t SENSOR_PW_PIN = 2;
static const uint8_t PERMISSION_LED_PIN = 13;
static const uint32_t SERIAL_BAUD = 115200;
static const uint32_t HEARTBEAT_MS = 250;
static inline monitor_config water_config() {
    monitor_config c = {};
    c.sensor = SENSOR_HRXL;
    c.filter = FILTER_MEDIAN;
    c.calibrated = true;
    c.min_distance_mm = 300;
    c.max_distance_mm = 4999;
    c.offset_mm = 0;
    c.sensor_height_mm = ${c.height};
    c.max_depth_mm = ${c.max_depth};
    c.trip_depth_mm = ${c.trip};
    c.restart_depth_mm = ${c.restart};
    c.timeout_ms = 1000;
    c.recovery_samples = 5;
    return c;
}
#endif
`;
}

export function savedCalibrationMatches(config, values, acknowledged) {
  return acknowledged && config.usb_cal==='1' && config.storage==='EEPROM' && config.calibrated==='1' &&
    ['height','max_depth','trip','restart'].every(k=>Number(config[k])===values[k]);
}
