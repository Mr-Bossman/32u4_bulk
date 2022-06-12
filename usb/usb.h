#ifndef USB_H_
#define USB_H_

#include <stdint.h>

uint8_t usb_config_status;

int usb_init();
uint8_t get_usb_config_status();
uint8_t usb_send(uint8_t * in, uint16_t count);
uint8_t usb_read(uint8_t* data, uint16_t count);
uint8_t usb_data();
#define GET_STATUS 0x00
#define CLEAR_FEATURE 0x01
#define SET_FEATURE 0x03
#define SET_ADDRESS 0x05
#define GET_DESCRIPTOR 0x06
#define GET_CONFIGURATION 0x08
#define SET_CONFIGURATION 0x09
#define GET_INTERFACE 0x0A
#define SET_INTERFACE 0x0B
#define GET_LUN 0xFE

#define idVendor 0x13fe
#define idProduct 0x4300
#define ENDPOINT_NUM_OUT 0x02
#define ENDPOINT_NUM_IN 0x01

#define CONFIG_SIZE 35

#endif