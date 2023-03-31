#include "ds18b20.h"
#include "onewire.h"
#include <stdint.h>
#include <util/delay.h>

void initds(){
	initow();
	_delay_ms(10);
	owwrbyte(SKIP_ADDR_CMND);
	owwrbyte(WRITE_SC_PAD);
	owwrbyte(0); // unused
	owwrbyte(0); // unused
	owwrbyte(RESOL_12BIT);
}

uint16_t rdtemp(){
	uint8_t scratchpad[2];
	uint16_t temp;
	float ret = 0;
	initow();
	_delay_us(100);
	owwrbyte(SKIP_ADDR_CMND);
	owwrbyte(START_CONVERT);
	while(!owrd());
	initow();
	_delay_ms(1);
	owwrbyte(SKIP_ADDR_CMND);
	owwrbyte(READ_SC_PAD);
	for(uint8_t i=0;i<9;i++){
		if(i < 2)
			scratchpad[i]=owrdbyte();
		else
				owrdbyte();
	}
	temp = (scratchpad[1]<<8)+scratchpad[0];
	ret = temp;
	ret/=16;// 2^-4 bit0
	return temp;
}