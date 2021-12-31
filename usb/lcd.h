#ifndef LCD_H_
#define LCD_H_
#include <stdint.h>

#define WR (1<<1)
#define RTS (1<<2)
#define DC (1<<3)
#define CTLDDR DDRD
#define CTLPRT PORTD

void LCD_reset();
void LCD_cmd(uint8_t data);
void LCD_data(uint8_t data);
void LCD_init();
#endif