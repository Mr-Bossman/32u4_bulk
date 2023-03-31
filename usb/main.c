#define F_CPU 16000000

#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include "usb.h"
#include "scsi.h"
#include "lcd.h"
#include "ds18b20.h"

volatile uint16_t last = 0;
uint16_t STR_read(uint8_t* data, uint16_t len)
{
	memset(data,0,512);
	((uint16_t*)data)[0] = last;
	return 0;
}
uint16_t STR_write(uint8_t* data, uint16_t len){
	if(len == 2ull*160ull*128ull){
		OCR0B = data[0];
		OCR0A = data[1];
		return 0;
	}
	if(len > 2ull*160ull*128ull){
		return 0;
	}
	for(uint16_t i = 0; i < 512; i++)
		LCD_data(data[i]);
	PORTB =0;
	return 0;
}
void timer0_init(){
	OCR0B = 0;
	OCR0A = 0;
	TCCR0B = 0x8|5;
	TCCR0A = 0x33;
	TCNT0=0;
}
int main() {
	DDRD |= 1;
	PORTC |= (1<<6);
	DDRC |= ((1 << 7) | (1 << 6));
	clock_prescale_set(clock_div_32);
	_delay_ms(1);
	clock_prescale_set(clock_div_1);
	initds();
	timer0_init();
	LCD_init();
	MSD_Prepare();
	usb_init();
	while (!get_usb_config_status()) {
		PORTC |= (1 << 7);
		_delay_ms(100);
		PORTC &= ~(1 << 7);
		_delay_ms(100);
	}
	uint16_t ucLen = 0;
	uint8_t ucData[72] = {0};
	while(1){
		uint16_t i = 0;
		do{
			MSD_transmit();
			if(i == 60000){
				last = rdtemp();
				i = 0;
			}
			i++;
		}while(!usb_data());
		ucLen = usb_read(ucData,sizeof(ucData));
		MSD_recive(ucData, ucLen);
	}
}