#include "lcd.h"
#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// CS must be low
// RTS is reset
//RS is DC
void LCD_reset(){
	CTLPRT &= ~RTS;
	_delay_ms(100);
	CTLPRT |= RTS;
	_delay_ms(120);
}
void LCD_data(uint8_t data){
	PORTB = data;
	CTLPRT &= ~WR;
	CTLPRT |= WR;
}
void LCD_cmd(uint8_t data){
	PORTB = data;
	CTLPRT &= ~DC;
	CTLPRT &= ~WR;
	CTLPRT |= WR;
	CTLPRT |= DC;
}

void LCD_init(){
	DDRB = 0xff;
	CTLDDR |= RTS;
	CTLPRT &= ~RTS;
	CTLDDR |= WR;
	CTLPRT |= WR;
	CTLDDR |= DC;
	CTLPRT |= DC;
	_delay_ms(20);
	CTLPRT |= RTS;
	_delay_ms(10);

	LCD_cmd(0xfe);
	LCD_cmd(0xfe);
	LCD_cmd(0xef);

	LCD_cmd(0xb3);
	LCD_data(0x03);
	
	LCD_cmd(0xb6);
	LCD_data(0x01);
	
	LCD_cmd(0xa3);
	LCD_data(0x11);
	
	LCD_cmd(0x21);
	
	LCD_cmd(0x36);
	LCD_data(0xd0); //08
	
	LCD_cmd(0x3a);
	LCD_data(0x05);
	
	LCD_cmd(0xb4);
	LCD_data(0x21);
	
	LCD_cmd(0xF0);
	LCD_data(0x31);
	LCD_data(0x4C);
	LCD_data(0x24);
	LCD_data(0x58);
	LCD_data(0xA8);
	LCD_data(0x26);
	LCD_data(0x28);
	LCD_data(0x00);
	LCD_data(0x2C);
	LCD_data(0x0C);
	LCD_data(0x0C);
	LCD_data(0x15);
	LCD_data(0x15);
	LCD_data(0x0f);

	LCD_cmd(0xF1);
	LCD_data(0x0E);
	LCD_data(0x2D);
	LCD_data(0x24);
	LCD_data(0x3E);
	LCD_data(0x99);
	LCD_data(0x12);
	LCD_data(0x13);
	LCD_data(0x00);
	LCD_data(0x0A);
	LCD_data(0x0D);
	LCD_data(0x0D);
	LCD_data(0x14);
	LCD_data(0x13);
	LCD_data(0x0f);

	LCD_cmd(0xfe);
	LCD_cmd(0xff);

	LCD_cmd(0x11);
	_delay_ms(12);
	LCD_cmd(0x29);
	LCD_cmd(0x2C);
	_delay_ms(10);
		
	for(uint16_t i = 128ul*160ul*2ul; i ;i--)LCD_data(0x00);

}