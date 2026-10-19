#ifndef LCD_STATUS_H
#define LCD_STATUS_H
#include "monitor.h"
#ifdef __cplusplus
extern "C" {
#endif
void lcd_status_lines(const struct monitor *m, char lines[2][17]);
#ifdef __cplusplus
}
#endif
#endif
