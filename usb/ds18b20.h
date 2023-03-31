
#ifndef DS18B20_H_
#define DS18B20_H_
#include <stdint.h>

#define SKIP_ADDR_CMND 0xCC
#define WRITE_SC_PAD   0x4E
#define RESOL_12BIT 0x7F
#define START_CONVERT 0x44
#define READ_SC_PAD 0xBE
void initds();
uint16_t rdtemp();
#endif /* DS18B20_H_ */