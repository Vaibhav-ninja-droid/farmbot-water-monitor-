#ifndef CALIBRATION_STORE_H
#define CALIBRATION_STORE_H
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
// Versioned little-endian record: magic/version, four uint16 values, CRC16.
#define CAL_RECORD_SIZE 13
static inline bool cal_geometry(const uint16_t *v) {
    return v[0] <= 4999 && v[1] > 0 && v[0] > v[1] && v[0]-v[1] > 300 &&
        v[2] > 0 && v[2] < v[3] && v[3] <= v[1];
}
static inline uint16_t cal_crc(const uint8_t *b) {
    uint16_t crc=0xffff;
    for(uint8_t i=0;i<11;i++){crc^=(uint16_t)b[i]<<8;for(uint8_t j=0;j<8;j++)crc=crc&0x8000?(crc<<1)^0x1021:crc<<1;}
    return crc;
}
static inline void cal_encode(const uint16_t *v,uint8_t *b) {
    b[0]='W';b[1]='C';b[2]=1;
    for(uint8_t i=0;i<4;i++){b[3+i*2]=v[i]&255;b[4+i*2]=v[i]>>8;}
    uint16_t crc=cal_crc(b);b[11]=crc&255;b[12]=crc>>8;
}
static inline bool cal_decode(const uint8_t *b,uint16_t *v) {
    if(b[0]!='W'||b[1]!='C'||b[2]!=1||cal_crc(b)!=(uint16_t)(b[11]|((uint16_t)b[12]<<8)))return false;
    for(uint8_t i=0;i<4;i++)v[i]=b[3+i*2]|((uint16_t)b[4+i*2]<<8);
    return cal_geometry(v);
}
// Strict decimal CSV; no whitespace, signs, extra fields or overflow.
static inline bool cal_parse(const char *s,uint16_t *v) {
    for(uint8_t i=0;i<4;i++){
        uint32_t n=0;uint8_t digits=0;
        while(*s>='0'&&*s<='9'){if(++digits>4)return false;n=n*10+(*s++-'0');}
        if(!digits||n>4999)return false;
        v[i]=(uint16_t)n;
        if(i<3){if(*s++!=',')return false;}else if(*s)return false;
    }
    return cal_geometry(v);
}
#endif
