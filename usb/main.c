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
	(void)data;
	(void)len;
	return 0;
}
uint16_t STR_write(uint8_t* data, uint16_t len){
	for(uint16_t i = 0; i < 512; i++)
		LCD_data(data[i]);
	(void)len;
	return 0;
}

int main() {
	PORTC |= (1<<6);
	DDRC |= ((1 << 7) | (1 << 6));
	clock_prescale_set(clock_div_32);
	_delay_ms(100);
	clock_prescale_set(clock_div_1);
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
		do{
			MSD_transmit();
		}while(!usb_data());
		ucLen = usb_read(ucData,sizeof(ucData));
		MSD_recive(ucData, ucLen);
	}
}