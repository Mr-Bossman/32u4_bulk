#ifndef ONEWIRE_H_
#define ONEWIRE_H_
#define F_CPU 16000000
#include <stdint.h>
#include <avr/io.h>

#define RST_TIME 480
#define HALF_TIME_SLOT 30
#define TIME_SLOT 60
#define RCVRY_TIME 20
#define WRITE_0_TIME 60
#define WRITE_1_TIME 10
#define READ_INIT_DLY 5
#define READ_VALID_DLY 5

#define OWPORT PORTD
#define OWDDR DDRD
#define OWPINS PIND
#define OWPIN (1<<6)


#include "onewire.h"
#include <stdint.h>
void owlow();
void owhigh();
uint8_t owrd();
void owwrbyte(uint8_t val);
uint8_t owrdbyte();
uint8_t initow();
#endif /* ONEWIRE_H_ */