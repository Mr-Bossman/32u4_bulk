#include "onewire.h"
#include <stdint.h>
#include <util/delay.h>

void owlow(){
	OWPORT &= ~OWPIN;//zero
	OWDDR |= OWPIN;//output
}

void owhigh(){
	OWPORT |= OWPIN;//pull-up
	OWDDR &= ~OWPIN;// input
}

uint8_t owrd(){
	return (OWPINS&OWPIN)?1:0;
}

void owwrbyte(uint8_t val){
	for(uint8_t shift = 0;shift < 8; shift++){
		_delay_us(RCVRY_TIME);
		owlow();
		if(val&1)
			_delay_us(WRITE_1_TIME);
		else
			_delay_us(WRITE_0_TIME);	
		owhigh();
		val >>= 1;
	}
}

uint8_t owrdbyte(){
	uint8_t val = 0;
	for(uint8_t shift = 0;shift < 8; shift++){
		_delay_us(RCVRY_TIME);
		owlow();
		_delay_us(READ_INIT_DLY);
		owhigh();
		_delay_us(READ_VALID_DLY);
		val |= owrd()<<shift;
	}
	return val;
}

uint8_t initow(){
	owlow();
	_delay_us(RST_TIME);
	owhigh();
	_delay_us(HALF_TIME_SLOT);
	return owrd();
}