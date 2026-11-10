#ifndef WATER_LCD_DISPLAY_H
#define WATER_LCD_DISPLAY_H
#include <Wire.h>
#include "src/lcd_status.h"
// PCF8574 backpack: P0=RS P1=RW P2=E P3=backlight P4..7=data.
// Fits first 16 columns and first two rows of common 1602/2004 displays.
static uint8_t lcdAddress;
static bool lcdHealthy;
static uint8_t lcdPosition;
static uint32_t lcdLastRefresh;
static char lcdWanted[2][17], lcdShown[2][17];
static bool lcdPort(uint8_t value) {
    Wire.beginTransmission(lcdAddress);
    Wire.write(value);
    if (Wire.endTransmission() != 0 || Wire.getWireTimeoutFlag()) {
        Wire.clearWireTimeoutFlag();
        lcdHealthy = false;
        return false;
    }
    return true;
}
static void lcdNibble(uint8_t n) {
    if (!lcdHealthy)
        return;
    if (!lcdPort(n | 0x08))
        return;
    if (!lcdPort(n | 0x0c))
        return;
    delayMicroseconds(1);
    lcdPort(n | 0x08);
    delayMicroseconds(50);
}
static void lcdByte(uint8_t value, bool data) {
    lcdNibble((value & 0xf0) | (data ? 1 : 0));
    lcdNibble(((value << 4) & 0xf0) | (data ? 1 : 0));
}
static void lcdBegin() {
    Wire.begin();
    Wire.setClock(100000);
    Wire.setWireTimeout(3000, true);
    // Only scan the two PCF8574 address ranges; no writes to arbitrary devices.
    for (uint8_t i = 0; i < 16; i++) {
        uint8_t addr = i < 8 ? 0x20 + i : 0x38 + i - 8;
        Wire.beginTransmission(addr);
        uint8_t result = Wire.endTransmission();
        if (Wire.getWireTimeoutFlag()) {
            Wire.clearWireTimeoutFlag();
            break;
        }
        if (result == 0) {
            lcdAddress = addr;
            break;
        }
    }
    lcdHealthy = lcdAddress != 0;
    if (!lcdHealthy) {
        Serial.println(F("# lcd,not_found,check_GND_VCC_SDA_A4_SCL_A5"));
        return;
    }
    delay(50);
    lcdNibble(0x30);
    delay(5);
    lcdNibble(0x30);
    delayMicroseconds(150);
    lcdNibble(0x30);
    lcdNibble(0x20);
    lcdByte(0x28, false);
    lcdByte(0x08, false);
    lcdByte(0x01, false);
    delay(2);
    lcdByte(0x06, false);
    lcdByte(0x0c, false);
    memset(lcdShown, 0, sizeof(lcdShown));
    lcdPosition = 0;
    Serial.print(F("# lcd,address=0x"));
    Serial.println(lcdAddress, HEX);
}
static void lcdUpdate(const monitor &m) {
    if (!lcdHealthy)
        return;
    uint32_t now = millis();
    if (!lcdPosition) {
        if ((uint32_t)(now - lcdLastRefresh) < 500)
            return;
        lcdLastRefresh = now;
        lcd_status_lines(&m, lcdWanted);
    }
    // At most one changed character per loop, leaving the sensor UART serviced.
    uint8_t row = lcdPosition / 16, col = lcdPosition % 16;
    if (lcdShown[row][col] != lcdWanted[row][col]) {
        lcdByte(0x80 | (row ? 0x40 : 0) | col, false);
        lcdByte(lcdWanted[row][col], true);
        if (!lcdHealthy) {
            Serial.println(F("# lcd,bus_error,power_off_check_wiring_restart"));
            return;
        }
        lcdShown[row][col] = lcdWanted[row][col];
    }
    lcdPosition = (lcdPosition + 1) % 32;
}
#endif
