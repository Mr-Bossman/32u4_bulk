#ifndef SCSI_H_
#define SCSI_H_
#include <stdint.h>

void MSD_Prepare();
void MSD_recive(uint8_t* ucData, uint16_t ucLen);
void MSD_transmit();
uint16_t STR_read(uint8_t* data, uint16_t len);
uint16_t STR_write(uint8_t* data, uint16_t len);

enum {txsIdle, txsData, txsCSW};
enum {rxsCBW, rxsData};
#define INQUIRY                       (uint8_t) 0x12
#define READ_FORMAT_CAPACITY          (uint8_t) 0x23
#define READ_CAPACITY                 (uint8_t) 0x25
#define READ_10                       (uint8_t) 0x28
#define WRITE_10                      (uint8_t) 0x2a
#define REQUEST_SENSE                 (uint8_t) 0x03
#define MODE_SENSE                    (uint8_t) 0x1a
#define TEST_UNIT_READY               (uint8_t) 0x00
#define VERIFY                        (uint8_t) 0x2F
#define PREVENT_ALLOW_MEDIUM_REMOVAL  (uint8_t) 0x1e

typedef struct CInquiryResponse
{
	uint8_t Peripheral;
	uint8_t Removble;
	uint8_t Version;
	uint8_t Response_Data_Format;
	uint8_t AdditionalLength;
	uint8_t Sccstp;
	uint8_t bqueetc;
	uint8_t CmdQue;
	uint8_t vendorID[8];
	uint8_t productID[16];
	uint8_t productRev[4];
} CInquiryResponse;

typedef struct USB_MSD_CBW
{
	unsigned long dCBWSignature;
	unsigned long dCBWTag;
	unsigned long dCBWDataTransferLength;
	uint8_t bCBWFlags;
	uint8_t bCBWLUN;
	uint8_t bCBWCBLength;
	uint8_t CBWCB[16];
} USB_MSD_CBW;

typedef struct USB_MSD_CSW
{
	unsigned long dCSWSignature;
	unsigned long dCSWTag;
	unsigned long dCSWDataResidue;
	uint8_t bCSWStatus;
} USB_MSD_CSW;
#endif