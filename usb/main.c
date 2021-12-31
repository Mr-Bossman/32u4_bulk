#define F_CPU 16000000

#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include "usb.h"
#include "scsi.h"
#include "lcd.h"
uint16_t STR_read(uint8_t* data, uint16_t len)
{

	return 0;
}
uint16_t STR_write(uint8_t* data, uint16_t len){
	for(uint16_t i = 0; i < 512; i++)
		LCD_data(data[i]);
	return 0;
}
volatile uint16_t _tx_delay = 0;

void initSerial(){
	_tx_delay = 0;
	uint16_t bit_delay = (F_CPU / 1200) / 4;
	if (bit_delay > 15 / 4)
	_tx_delay = bit_delay - (15 / 4);
	else
	_tx_delay = 1;
}
void Send (uint8_t b) {
	uint8_t mask = 1<<6;
	uint8_t imask = ~mask;
	cli();
	PORTC &= imask;
	_delay_loop_2(_tx_delay);
	for (uint8_t i = 8; i > 0; --i) {
		if (b & 1)
		PORTC |= mask;
		else
		PORTC &= imask;
		_delay_loop_2(_tx_delay);
		b >>= 1;
	}
	PORTC |= mask;
	_delay_loop_2(_tx_delay);
	sei();
}
void printd(unsigned int num){
	do{
		Send(num%10 + '0');
		num/=10;
	}while(num);
	Send('\n');
	Send('\r');
}
void printx(uint8_t num){
	Send((num>>4) + (((num>>4)>9)?'W':'0'));
	Send((num&0xf) + (((num&0xf)>9)?'W':'0'));
	Send(',');
	Send(' ');
}
void nl(){
	Send('\n');
	Send('\r');
}
int main() {
	PORTC |= (1<<6);
	DDRC |= ((1 << 7) | (1 << 6));
	clock_prescale_set(clock_div_32);
	_delay_ms(100);
	clock_prescale_set(clock_div_1);
	LCD_init();
	initSerial();
	MSD_Prepare();
	usb_init();
	while (!get_usb_config_status()) {
		PORTC |= (1 << 7);
		_delay_ms(100);
		PORTC &= ~(1 << 7);
		_delay_ms(100);
	}
	uint16_t ucLen = 0;
	uint8_t ucData[512] = {0};
	while(1){
		do{
			MSD_transmit();
		}while(!usb_data());
		ucLen = usb_read(ucData,sizeof(ucData));
		MSD_recive(ucData, ucLen);
	}
}