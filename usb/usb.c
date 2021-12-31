#define F_CPU 16000000
#include "usb.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/io.h>


static const uint8_t device_descriptor[] PROGMEM = {
	// Stored in PROGMEM (Program Memory) Flash, freeing up some SRAM where
	// variables are usually stored
	18,  // bLength - The total size of the descriptor
	1,   // bDescriptorType - The type of descriptor - 1 is device
	0x00,
	0x02,  // bcdUSB - The USB protcol supported - Refer to USB 2.0
	// Chapter 9.6.1
	0,   // bDeviceClass - The Device Class, 0 indicating that the HID interface
	// will specify it
	0,   // bDeviceSubClass - 0, HID will specify
	0,   // bDeviceProtocol - No class specific protocols on a device level, HID
	// interface will specify
	32,  // bMaxPacketSize0 - 32 byte packet size; control endpoint was
	// configured in UECFG1X to be 32 bytes
	(idVendor & 255),
	((idVendor >> 8) &
	255),  // idVendor - Vendor ID specified by USB-IF (To fit the 2 bytes, the
	// ID is split into least significant and most significant byte)
	(idProduct & 255),
	((idProduct >> 8) & 255),  // idProduct - The Product ID specified by USB-IF
	// - Split in the same way as idVendor
	0x00,
	0x01,  // bcdDevice - Device Version Number
	1,  // iManufacturer - The String Descriptor that has the manufacturer name
	// -
	// Specified by USB 2.0 Table 9-8
	2,  // iProduct - The String Descriptor that has the product name -
	// Specified
	// by USB 2.0 Table 9-8
	3,  // iSerialNumber - The String Descriptor that has the serial number of
	// the product - Specified by USB 2.0 Table 9-8
	1   // bNumConfigurations - The number of configurations of the device, most
	// devices only have one
};


static const uint8_t configuration_descriptor[CONFIG_SIZE] PROGMEM = {
	9,  // bLength
	2,  // bDescriptorType - 2 is device
	(CONFIG_SIZE & 255),
	((CONFIG_SIZE >> 8) &
	255),  // wTotalLength - The total length of the descriptor tree
	1,      // bNumInterfaces - 1 Interface
	1,      // bConfigurationValue
	0,      // iConfiguration - We have no string descriptors
	0x80,   // bmAttributes - Set the device power source
	50,     // bMaxPower - 50 x 2mA units = 100mA max power consumption
	// Refer to Table 9-10 for the descriptor structure - Configuration
	// Descriptors have interface descriptors, interface descriptors have
	// endpoint descriptors along with a special HID descriptor
	9,     // bLength
	4,     // bDescriptorType - 4 is interface
	0,     // bInterfaceNumber - This is the 0th and only interface
	0,     // bAlternateSetting - There are no alternate settings
	2,     // bNumEndpoints - This interface only uses one endpoint
	0x08,  // bInterfaceClass - 0x03 (specified by USB-IF) is the interface
	// class code for HID
	6,  // bInterfaceSubClass - 1 (specified by USB-IF) is the constant for
	// the boot subclass - this keyboard can communicate with the BIOS,
	// but is limited to 6KRO, as are most keyboards
	0x50,  // bInterfaceProtocol - 0x01 (specified by USB-IF) is the protcol
	// code for keyboards
	0,     // iInterface - There are no string descriptors for this
	// Endpoint Descriptor - Example can be found in the HID spec table E.5
	7,     // bLength
	0x05,  // bDescriptorType
	ENDPOINT_NUM_OUT,  // Set keyboard endpoint to IN endpoint, refer to table
	0x02,      // bmAttributes - Set endpoint to interrupt
	0x20, 0x00,      // wMaxPacketSize - The size of the keyboard banks
	0x00,       // wInterval - Poll for new data 1000/s, or once every ms
	7,     // bLength
	0x05,  // bDescriptorType
	ENDPOINT_NUM_IN|0x80,  // Set keyboard endpoint to IN endpoint, refer to table
	0x02,      // bmAttributes - Set endpoint to interrupt
	0x20, 0x00,      // wMaxPacketSize - The size of the keyboard banks
	0x00       // wInterval - Poll for new data 1000/s, or once every ms
};

uint8_t usb_send(uint8_t * in, uint16_t count) {
	if (!usb_config_status)
	return -1;  // Why are you even trying
	cli();
	UENUM = ENDPOINT_NUM_IN;

	uint16_t len = count;
	while (!(UEINTX & (1 << RWAL)));  // Wait for banks to be ready
	while(len--)
		UEDATX = *in++;
	UEINTX = 0b00111010;
	sei();
	return count-len;
}
uint8_t usb_data(){
	cli();
	UENUM = ENDPOINT_NUM_OUT;
	uint8_t len = UEBCLX;
	sei();
	return len;
}
uint8_t usb_read(uint8_t* dat, uint16_t count)
{
	if (!usb_config_status)
	return -1;  // Why are you even trying
	cli();
	UENUM = ENDPOINT_NUM_OUT;
	while (!(UEINTX & (1 << RWAL)));
	uint16_t len = UEBCLX;
	if(count < len){
		len = count;
	}
	count = len;
	volatile uint8_t * data = dat;
	while(len--)
		*data++ = UEDATX;
	if(!UEBCLX)
		UEINTX = 0x6B;
	sei();
	return count;
}
int usb_init() {
	cli();  // Global Interrupt Disable

	UHWCON |= (1 << UVREGE);  // Enable USB Pads Regulator

	PLLCSR |= 0x12;  // Configure to use 16mHz oscillator

	while (!(PLLCSR & (1 << PLOCK)))
	;  // Wait for PLL Lock to be achieved

	USBCON |=
	(1 << USBE) | (1 << OTGPADE);  // Enable USB Controller and USB power pads
	USBCON &= ~(1 << FRZCLK);          // Unfreeze the clock

	UDCON = 0;  // FULL SPEED MODE

	USBCON &= ~(1 << DETACH);  // Connect
	UDIEN |= (1 << EORSTE) |
	(1 << SOFE);  // Re-enable the EORSTE (End Of Reset) Interrupt so we
	// know when we can configure the control endpoint
	usb_config_status = 0;
	sei();  // Global Interrupt Enable
	return 0;
}

uint8_t get_usb_config_status() {
	return usb_config_status;
}

ISR(USB_GEN_vect) {
	uint8_t udint_temp = UDINT;
	UDINT = 0;

	if (udint_temp & (1 << EORSTI)) {  // If end of reset interrupt
		// Configure Control Endpoint
		UENUM = 0;             // Select Endpoint 0, the default control endpoint
		UECONX = (1 << EPEN);  // Enable the Endpoint
		UECFG0X = 0;      // Control Endpoint, OUT direction for control endpoint
		UECFG1X |= 0x22;  // 32 byte endpoint, 1 bank, allocate the memory
		usb_config_status = 0;

		if (!(UESTA0X &
		(1 << CFGOK))) {  // Check if endpoint configuration was successful
			return;
		}

		UERST = 1;  // Reset Endpoint
		UERST = 0;

		UEIENX =
		(1 << RXSTPE);  // Re-enable the RXSPTE (Receive Setup Packet) Interrupt
		return;
	}
}

ISR(USB_COM_vect) {
	UENUM = 0;
	if (UEINTX & (1 << RXSTPI)) {
		uint8_t bmRequestType = UEDATX;  // UEDATX is FIFO; see table in README
		uint8_t bRequest = UEDATX;
		uint16_t wValue = UEDATX;
		wValue |= UEDATX << 8;
		uint16_t wIndex = UEDATX;
		wIndex |= UEDATX << 8;
		uint16_t wLength = UEDATX;
		wLength |= UEDATX << 8;

		UEINTX &= ~(
		(1 << RXSTPI) | (1 << RXOUTI) |
		(1 << TXINI));  // Handshake the Interrupts, do this after recording
		// the packet because it also clears the endpoint banks
		if (bRequest == GET_DESCRIPTOR) {
			// The Host is requesting a descriptor to enumerate the device
			const uint8_t* descriptor;
			uint8_t descriptor_length;

			if (wValue == 0x0100) {  // Is the host requesting a device descriptor?
				descriptor = device_descriptor;
				descriptor_length = pgm_read_byte(descriptor);
				} else if (wValue == 0x0200) {  // Is it asking for a configuration descriptor?
				descriptor = configuration_descriptor;
				descriptor_length =
				CONFIG_SIZE;  // Configuration descriptor is comprised of many
				// different descriptors; the length is more than
				// bLength
				}  else {
				UECONX |=
				(1 << STALLRQ) | (1 << EPEN);  // Enable the endpoint and stall, the
				// descriptor does not exist
				return;
			}

			uint8_t request_length =
			wLength > 255 ? 255
			: wLength;  // Our endpoint is only so big; the USB Spec
			// says to truncate the response if the size
			// exceeds the size of the endpoint

			descriptor_length =
			request_length > descriptor_length
			? descriptor_length
			: request_length;  // Truncate to descriptor length at most

			while (descriptor_length > 0) {
				while (!(UEINTX & (1 << TXINI)))
				;  // Wait for banks to be ready for data transmission
				if (UEINTX & (1 << RXOUTI))
				return;  // If there is another packet, exit to handle it

				uint8_t thisPacket =
				descriptor_length > 32
				? 32
				: descriptor_length;  // Make sure that the packet we are
				// getting is not too big to fit in the
				// endpoint

				for (int i = 0; i < thisPacket; i++) {
					UEDATX = pgm_read_byte(
					descriptor +
					i);  // Send the descriptor over UEDATX, use pgmspace functions
					// because the descriptors are stored in flash
				}

				descriptor_length -= thisPacket;
				descriptor += thisPacket;
				UEINTX &= ~(1 << TXINI);
			}
			return;
		}

		if (bRequest == SET_CONFIGURATION &&
		bmRequestType ==
		0) {  // Refer to USB Spec 9.4.7 - This is the configuration request
			// to place the device into address mode
			usb_config_status = wValue;
			UEINTX &= ~(1 << TXINI);
			UENUM = ENDPOINT_NUM_IN;
			UECONX |= (1 << EPEN);
			UECFG0X = 0b10000001;  // EPTYPE Bulk 1  IN
			UECFG1X = 0b00110010;  // Single Bank Endpoint, 64 Bytes, allocate memory
			if (!(UESTA0X & (1 << CFGOK)))  usb_config_status = 0;
			UENUM = ENDPOINT_NUM_OUT;
			UECONX |= (1 << EPEN);
			UECFG0X = 0b10000000;  // EPTYPE Bulk 0 OUT
			UECFG1X = 0b00110010;  // Single Bank Endpoint, 64 Bytes, allocate memory
			if (!(UESTA0X & (1 << CFGOK)))  usb_config_status = 0;
			UERST = 0x1E;          // Reset all of the endpoints
			UERST = 0;
			return;
		}

		if (bRequest == SET_ADDRESS) {
			UEINTX &= ~(1 << TXINI);
			while (!(UEINTX & (1 << TXINI)))
			;  // Wait until the banks are ready to be filled

			UDADDR = wValue | (1 << ADDEN);  // Set the device address
			return;
		}

		if (bRequest == GET_CONFIGURATION &&
		bmRequestType == 0x80) {  // GET_CONFIGURATION is the host trying to get
			// the current config status of the device
			while (!(UEINTX & (1 << TXINI)))
			;  // Wait until the banks are ready to be filled
			UEDATX = usb_config_status;
			UEINTX &= ~(1 << TXINI);
			return;
		}

		if (bRequest == GET_STATUS) {
			while (!(UEINTX & (1 << TXINI)))
			;
			UEDATX = 0;
			UEDATX = 0;
			UEINTX &= ~(1 << TXINI);
			return;
		}
		if(bRequest == GET_LUN){
			while (!(UEINTX & (1 << TXINI)));
			UEDATX = 0;
			UEINTX &= ~(1 << TXINI);
			return;
		}

	}

	UECONX |= (1 << STALLRQ) |
	(1 << EPEN);  // The host made an invalid request or there was an
}