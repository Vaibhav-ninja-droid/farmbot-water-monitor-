#include <assert.h>
#include <stdio.h>
#include "calibration_store.h"
int main(void){
 uint16_t v[5],out[5];uint8_t record[CAL_RECORD_SIZE];
 assert(cal_parse("700,100,30,60,90",v));cal_encode(v,record);assert(cal_decode(record,out));
 for(int i=0;i<5;i++)assert(v[i]==out[i]);
 for(int i=0;i<CAL_RECORD_SIZE;i++){for(int bit=0;bit<8;bit++){record[i]^=1<<bit;assert(!cal_decode(record,out));record[i]^=1<<bit;}}
 const char *bad[]={"","700,100,30","700,100,30,60,1","700,100,30,60x","+700,100,30,60","700,100,30, 60","999999999999,100,30,60","400,100,30,60","700,100,60,30","700,100,0,60","700,100,30,101","5000,100,30,60"};
 for(unsigned i=0;i<sizeof(bad)/sizeof(*bad);i++)assert(!cal_parse(bad[i],v));
 uint8_t legacy[15]={'W','C',1,188,2,100,0,30,0,60,0,0,0,255,255};
 uint16_t crc=0xffff;
 for(int i=0;i<11;i++){crc^=(uint16_t)legacy[i]<<8;for(int j=0;j<8;j++)crc=crc&0x8000?(crc<<1)^0x1021:crc<<1;}
 legacy[11]=crc&255;legacy[12]=crc>>8;
 assert(cal_decode_v1(legacy,out));assert(out[0]==700&&out[4]==90);
 legacy[5]^=1;assert(!cal_decode_v1(legacy,out));
 assert(!cal_parse("700,100,30,60,59",v));assert(!cal_parse("700,100,30,60,101",v));
 puts("PASS: EEPROM record round-trip, every single-bit corruption and invalid USB geometry");
}
