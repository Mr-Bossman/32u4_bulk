#include "scsi.h"
#include "usb.h"
#include <string.h>

static USB_MSD_CSW csw;
static USB_MSD_CBW cbw;

const unsigned char usb_format_capacity[] = {
	0x00, 0x00, 0x00, 0x08,
	0x00, 0xF2, 0x38, 0x00,
	0x02,
	0x00, 0x02, 0x00
};

static const uint8_t usb_read_capacity[] = {0x00, 0x0f, 0x23, 0x7F, 0x00, 0x00, 0x02, 0x00};

static const uint8_t usb_mode_sense[] = {0x03, 0x00, 0x00, 0x00};


static const uint8_t usb_request_sense[] = {0x70, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00};

static const CInquiryResponse inq_resp = {
	0x00,                                                             // direct access block device, conne
	0x80,                                                             // device is removable
	0x02,                                                             // SPC-2 compliance
	0x02,                                                             // response data format
	0x1F,                                                             // response has 20h + 4 bytes
	0x00, 0x00, 0x00,                                                 // additional fields, none set
	{'A','T','m','e','g','a',' ','8'},                                  // 8 -byte T10-assigned Vendor ID
	{'V','-','U','S','B',' ','S','D','-','A','d','a','p','t','e','r'},  // 16-byte product identification
	{'0','0','0','0'}                                                // 4-byte product revision level
};

static uint8_t eTxStage = txsIdle;
static uint8_t eRxStage = rxsCBW;
static unsigned int uiRxBytes = 0x00;

static unsigned int ucPos = 0x00;
static unsigned int ucSize = 0x00;

static unsigned long ulAddr = 0x00;
static unsigned int uiBlockNum = 0x00;
static uint8_t ucDataPBP[512];

void MSD_Prepare()
{
	csw.dCSWSignature = 0x53425355;
}

void MSD_PareseCBW()
{
	// Set Command status wrap TAG namber as command block wrap TAG
	csw.dCSWTag = cbw.dCBWTag;

	//CSW status of input command OK (by default), clear rx bytes count
	csw.bCSWStatus = csw.dCSWDataResidue = uiRxBytes = 0x00;

	// Now CBW is recieved and we must transmit data
	eTxStage = txsData;

	switch(cbw.CBWCB[0])
	{
		case READ_10:
		ucSize = 512;

		uiBlockNum = ((unsigned int)cbw.CBWCB[7] << 8) + cbw.CBWCB[8] - 1;
		ulAddr = 512 * (((unsigned long)cbw.CBWCB[2] << 24) + ((unsigned long)cbw.CBWCB[3] << 16) + ((unsigned int)cbw.CBWCB[4] << 8) + cbw.CBWCB[5]);
		STR_read(ucDataPBP,ulAddr);
		break;
		
		case WRITE_10:
		eTxStage = txsIdle;
		eRxStage = rxsData;
		ucSize = 0x00;

		uiBlockNum = ((unsigned int)cbw.CBWCB[7] << 8) + cbw.CBWCB[8];
		ulAddr = 512 * (((unsigned long)cbw.CBWCB[2] << 24) + ((unsigned long)cbw.CBWCB[3] << 16) + ((unsigned int)cbw.CBWCB[4] << 8) + cbw.CBWCB[5]);
		break;
		
		case INQUIRY:
		memcpy((uint8_t*)&ucDataPBP, (const uint8_t*)&inq_resp, sizeof(inq_resp));
		ucSize = sizeof(inq_resp);
		break;

		case READ_CAPACITY:
		memcpy((uint8_t*)&ucDataPBP, (const uint8_t*)&usb_read_capacity, sizeof(usb_read_capacity));
		ucSize = sizeof(usb_read_capacity);
		break;

		case READ_FORMAT_CAPACITY:
		memcpy((uint8_t*)&ucDataPBP, (const uint8_t*)&usb_format_capacity, sizeof(usb_format_capacity));
		ucSize = sizeof(usb_format_capacity);
		break;

		case TEST_UNIT_READY:
		ucSize = 0x00;
		break;

		case MODE_SENSE:
		memcpy((uint8_t*)&ucDataPBP, (const uint8_t*)&usb_mode_sense, sizeof(usb_mode_sense));
		ucSize = sizeof(usb_mode_sense);
		break;

		case REQUEST_SENSE:
		memcpy((uint8_t*)&ucDataPBP, (const uint8_t*)&usb_request_sense, sizeof(usb_request_sense));
		ucSize = sizeof(usb_request_sense);
		break;

		default:
		ucSize = 0x00;
		csw.bCSWStatus = 0x01;
		break;
	}
}

void MSD_RxCBW(uint8_t* ucData, uint16_t ucLen)
{
	memcpy((uint8_t*)&cbw + uiRxBytes, ucData, ucLen);
	uiRxBytes+= ucLen;
	if (uiRxBytes == sizeof(USB_MSD_CBW))
		MSD_PareseCBW();
}

void MSD_RxData(uint8_t* ucData, uint16_t ucLen)
{
	memcpy((uint8_t*)&ucDataPBP + uiRxBytes, ucData, ucLen);
	uiRxBytes+=ucLen;

	if (512 == uiRxBytes)
	{

		uiRxBytes = 0;
		STR_write(ucDataPBP,ulAddr);
		ulAddr+=512;
		uiBlockNum--;
		if (uiBlockNum == 0x00)
		{
			eRxStage = rxsCBW;
			eTxStage = txsCSW;
		}
	}
}

void MSD_TxData()
{
	uint8_t ucBR = 32;
	uint8_t res = 0x00;
	unsigned int ucChunkSize = ucSize - ucPos;

	if (ucChunkSize < 32)
	{
		if (uiBlockNum)
		{
			ucPos = 0x00;
			ulAddr += 512;
			uiBlockNum--;
			STR_read(ucDataPBP,ulAddr);
		}
		else
		{
			ucBR = ucChunkSize;
			res = 1;
		}
	}
	
	if (ucBR)
	{
		usb_send((uint8_t*)&ucDataPBP[ucPos], ucBR);
	}

	if (res)
	{
		ucPos = ucSize = 0x00;
		eTxStage = txsCSW;
	}
	else
	{
		ucPos+= ucBR;
	}
}

void MSD_TxCSW()
{
	uint8_t ucBR = sizeof(csw);

	usb_send((uint8_t*)&csw, ucBR);
	eTxStage = txsIdle;
}

void MSD_recive(uint8_t* ucData, uint16_t ucLen){
	if (eRxStage == rxsCBW)
	MSD_RxCBW(ucData, ucLen);
	else if (eRxStage == rxsData)
	MSD_RxData(ucData, ucLen);
}
void MSD_transmit(){
	if (eTxStage == txsData)
	MSD_TxData();
	if (eTxStage == txsCSW)
	MSD_TxCSW();
}