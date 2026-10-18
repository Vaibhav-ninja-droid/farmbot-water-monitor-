#include <assert.h>
#include <stdio.h>
#include "calibration_store.h"
int main(void){
 uint16_t v[4],out[4];uint8_t record[CAL_RECORD_SIZE];
 assert(cal_parse("700,100,30,60",v));cal_encode(v,record);assert(cal_decode(record,out));
 for(int i=0;i<4;i++)assert(v[i]==out[i]);
 for(int i=0;i<CAL_RECORD_SIZE;i++){for(int bit=0;bit<8;bit++){record[i]^=1<<bit;assert(!cal_decode(record,out));record[i]^=1<<bit;}}
 const char *bad[]={"","700,100,30","700,100,30,60,1","700,100,30,60x","+700,100,30,60","700,100,30, 60","999999999999,100,30,60","400,100,30,60","700,100,60,30","700,100,0,60","700,100,30,101","5000,100,30,60"};
 for(unsigned i=0;i<sizeof(bad)/sizeof(*bad);i++)assert(!cal_parse(bad[i],v));
 puts("PASS: EEPROM record round-trip, every single-bit corruption and invalid USB geometry");
}
