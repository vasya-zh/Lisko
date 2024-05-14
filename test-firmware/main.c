

#include <em_chip.h>
#include <em_cmu.h>
#include <em_core.h>
#include <em_timer.h>




#include "main.h"

#include "utils.h" 

#include "epd27.h"
#include "FT6336.h"
#include "em_ldma.h"

#include "dmadrv.h" 
#include "config\dmadrv_config.h"

#include "AT25.h"
#include "psram.h"

#include "NfcDecoder.h"
#include "sl_status.h"
#include <math.h>
#include "alzlogo.h"
#include "em_acmp.h"
#include "tests.h"
#include "code128.h"

#include "qrcode.h"

#define SHOWPACK



uint8_t FlashtoEpd[1024];
uint16_t FlashEpdP;
uint32_t FBp;
uint32_t PicBase;
bool DispState;

#define LDMA_CH_MASK        (1 << 0)
unsigned int channelRX;
uint8_t PrintPos = 0;
uint8_t AddDataPos = 0;
bool PackLoaded = false;
uint16_t PsramPage;

extern uint8_t FrameBuffer[BUFSIZE];
uint64_t uniqueId;
//char PrintBuf[256];

uint8_t cnnt = 0;

char PassEntered[16];
uint8_t PassLen = 0;

extern volatile uint32_t TickCount;
uint32_t TickOld;

char TcVal[4][4] =
{
	'7',
	'8',
	'9',
	'F',
	'4',
	'5',
	'6',
	'E',
	'1',
	'2',
	'3',
	'D',
	'0',
	'A',
	'B',
	'C'
};

char MenuVal[10][16] =
{
	"RAM",
	"ROM",
	"DISP",
	"NFC",
	"SENS",
	"FREQ",
	"DEMO",
	"TEST",
	"GAME"
};

char PassIs[] = { "EDB0C9" };


char ch[16];
bool PassOk = false;



extern volatile double MainVoltage;
extern bool AllPacksReceived;


bool PowerState = false;

char ReceivedChar;

char EpdPrint[128];

struct 
{
	uint8_t Touches;
	uint16_t X1;
	uint16_t Y1;
	uint16_t X2;
	uint16_t Y2;
	
} SelfDest;


extern uint8_t NfcData[NFC_DECODED_MAX]; 
size_t NfcDataLen;


#define RX_BUFFER_SIZE      1024 /* Even numbers only */

/* RX Ring Buffer */
struct 
{
	char buffer[RX_BUFFER_SIZE];
	int startIndex;
	int stopIndex; 
	int counts; 
} rxBuffer = { .startIndex = 0, .stopIndex = 0 };


//Touch
bool TouchFound;
uint8_t TouchModel;
bool Touched;
uint16_t TouchX;
uint16_t TouchY;
uint8_t TouchArr[16];


//touch panel int
void IntWork()
{
	uint32_t intval = GPIO_IntGet();

	if (PowerState)
	{
	
		switch (intval)
		{
	
		case 1 << INT_PIN: //touch
			
			if (FT6336_ReadLocation(TouchArr))
			{
				if (!Touched)
				{
					Touched = true;
					TouchX = TouchArr[0];
					TouchY = (TouchArr[1] << 8) + TouchArr[2];
						
						
				}
			}
							
			break;
	
		
		default:
			break;
		}
	}
	
	GPIO_IntClear(intval);

}



void GPIO_EVEN_IRQHandler(void) 
{
	IntWork();
}



void GPIO_ODD_IRQHandler(void) 
{
	IntWork(); 
}




/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/
/******************************/



void dispw(uint8_t d)
{
	
	cnnt++;
	if (cnnt < 23)
	{
		
		FlashtoEpd[FlashEpdP++] = d;
		if (FlashEpdP > 255)
		{
			FlashEpdP = 0;
			while (SpiFlashReadStatus() & SPI_STATUS_BSY) ; //wait for ready 
			SpiFlashCmd(SPICMD_WREN);
			SpiFlashErasePage(FBp); //erase page

			while (SpiFlashReadStatus() & SPI_STATUS_BSY) ; //wait for ready 
			SpiFlashCmd(SPICMD_WREN);
			SpiFlashWritePage(FlashtoEpd, FBp);
			FBp += 0x100;
		}
	}
	if (cnnt > 49) cnnt = 0;

}




void ShowButtons()
{
	const uint16_t Ys = 88;
	uint8_t i, x, y;
	char ch[2] = { 0, 0 };	
	
	//	for (i = 0; i < 176; i += 44)
	//	{
	//		EPD_line(i, 0 + Ys, i, 175 + Ys, 1);
	//		EPD_line(0, i + Ys, 175, i + Ys, 1);
	//	}
	//	EPD_line(175, 0 + Ys, 175, 175 + Ys, 1);
	//	EPD_line(0, 175 + Ys, 175, 175 + Ys, 1);

	for (y = 0; y < 4; y++)
	{
		for (x = 0; x < 4; x++)
		{
			ch[0] = TcVal[y][x];
			EPD_Text(x * 44 + 16, y * 44 + 15 + Ys, ch, 1, 0);
			EPD_DrawCircle(x * 44 + 21, y * 44 + 21 + Ys, 16, 1);

		}
	}
		
	EPD_PartWriteFull(true);
}




void ShowMenu()
{
	const uint16_t Ys = 88;
	uint8_t i, x, y;
	char ch[2] = { 0, 0 };	
	
	for (i = 0; i < 176; i += 59)
	{
		EPD_line(i, 0 + Ys, i, 175 + Ys, 1);
		EPD_line(0, i + Ys, 175, i + Ys, 1);
	}
	EPD_line(175, 0 + Ys, 175, 175 + Ys, 1);
	EPD_line(0, 175 + Ys, 175, 175 + Ys, 1);

	EPD_Text(11, 110, MenuVal[0], 1, 0);
	EPD_Text(70, 110, MenuVal[1], 1, 0);
	EPD_Text(122, 110, MenuVal[2], 1, 0);

	EPD_Text(11, 170, MenuVal[3], 1, 0);
	EPD_Text(64, 170, MenuVal[4], 1, 0);
	EPD_Text(123, 170, MenuVal[5], 1, 0);

	EPD_Text(6, 228, MenuVal[6], 1, 0);
	EPD_Text(64, 228, MenuVal[7], 1, 0);
	EPD_Text(123, 228, MenuVal[8], 1, 0);
	
	
	EPD_PartWriteFull(true);
}


void ClearMenu(uint8_t b)
{
	memset(FrameBuffer  + 1936, b, sizeof(FrameBuffer) - 1936);
}








void Shuffle(char * arr)
{
	uint8_t RandV;
	char bc, idx;
	for (uint8_t i = 0; i < 16; i++)
	{
		RandV = EfmGetRandom();
		idx = RandV & 0x04;
		bc = arr[i];
		arr[i] = arr[idx];
		arr[idx] = bc;
	}
}


int fts_ctpm_auto_clb()
{
	uint8_t uc_temp;
	uint8_t i;

	FT6336_WriteReg(0, 0x0); //return to normal mode 
	Delay(300);
	
	uPrintf("[FT6336] start auto CLB.\r\n");
	FT6336_WriteReg(0, 0x40);  

	Delay(100); //make sure already enter factory mode=Test mode
	FT6336_WriteReg(2, 0x4); //write command to start calibration
	uc_temp = FT6336_ReadReg(0x02);
	uPrintf("Cmd: %02x\r\n", uc_temp);
	Delay(300);
	uc_temp = FT6336_ReadReg(0x02);
	uPrintf("Cmd: %02x\r\n", uc_temp);
	
	for (i = 0; i < 50; i++) //llf 20130220
	{
		if (!(i % 10))
		{
			FT6336_WriteReg(2, 0x4); //write command to start calibration
		}
		uc_temp = FT6336_ReadReg(0x0);
		if ((uc_temp & 0x70) == 0x0)  //return to normal mode, calibration finish
		{
			break;
		}
		Delay(200);
		uPrintf("[FT6336] waiting calibration %02X %d\r\n", uc_temp, i);
	}
	FT6336_WriteReg(0, 0x40);  
	Delay(100); //make sure already enter factory mode=Test mode
	FT6336_WriteReg(2, 0x4); //write command to start calibration
	Delay(300);
	for (i = 0; i < 50; i++) //llf 20130220
	{
		uc_temp = FT6336_ReadReg(0x0);
		if ((uc_temp & 0x70) == 0x0)  //return to normal mode, calibration finish
		{
			break;
		}
		Delay(200);
		uPrintf("[FT6336] waiting calibration %02X %d\r\n", uc_temp, i);

	}
	uPrintf("[FT6336] calibration OK.\r\n");

	Delay(300);
	FT6336_WriteReg(0, 0x40); //goto factory mode
	Delay(100); //make sure already enter factory mode
	FT6336_WriteReg(2, 0x5); //store CLB result
	Delay(300);
	FT6336_WriteReg(0, 0x0); //return to normal mode 
	Delay(300);
	uPrintf("[FT6336] store CLB result OK.\r\n");

	return 0;
}

void FullClear()
{
	EPD_clear(0); //buf init
	EPD_PartWriteFull(true);
	EPD_clear(1); //buf init
}

void PadClear()
{
	EPD_PartWriteFull(true);
	ClearMenu(0x00);
	EPD_PartWriteFull(true);
	ClearMenu(0xFF);
}

int main()
{
	uint8_t i, j;
	
	uint16_t x, y;
	float mfX, mfY;
	float xScreen, yScreen;
	float xCent, yCent;
	
	void *user;
	Ecode_t res; 

	
	uint8_t DispRead[8];

	QRCode qrcode;
	uint8_t qrcodeData[qrcode_getBufferSize(5)];
	
	CHIP_Init();
	ClockInit();
	FreqSet(2); //10
	
#ifndef SHOWPACK
	
	while (true)
	{
		// Enter EM2 sleep, woken by IADC interrupt
		EMU_EnterEM2(true);
		if (MainVoltage > 14.0f) break;
	}
#endif	

	
	
	SpiFlashDispInit(); //flash and disp SPI init
	SpiFlashUDPD(); //Ultra-Deep Power-Down of Flash chip

	DMADRV_Init(); //init DMA
	
	SpiDispInit(); //touch panel and disp lines init

	UartInit();
	PsramInit(); //Psram ports init	
	
	NfcInit();
	
	

	
	
	POW_2V8(1); // enable 2.8 volt
	//POW_I2C_T(1); // enable i2c level translator


	
	TOUCH_RES(1); //reset touch
	Delay(100);
	TOUCH_RES(0); //reset touch
	Delay(100);
	
	FT6336_WriteReg(FT6X36_REG_DEVICE_MODE, 0x00);
	FT6336_WriteReg(FT6X36_REG_THRESHHOLD, 0x50);
	FT6336_WriteReg(FT6X36_REG_FILTER_COEF, 0x00);
	FT6336_WriteReg(FT6X36_REG_TOUCHRATE_ACTIVE, 0x0E);

	
	uint16_t Fs = SYSTEM_GetFlashSize();
	uint8_t rev = SYSTEM_GetDevinfoRev();
	uint16_t Pn = SYSTEM_GetPartNumber();

	uPrintf("\r\nReady  Pn: %u, Rev: %u, Flash: %u\r\n", Pn, rev, Fs);
	
	uniqueId = readUniqueId();

	uPrintf("UniqueID %08lX%08lX\r\n",
		((uint32_t)((uniqueId >> 32) & 0xFFFFFFFF)),
		((uint32_t)(uniqueId & 0xFFFFFFFF)));
	
	

	
	
	ClockInit();

	EPD_clear(1); //buf init

	EPD_HW_Init_Fast();
	
	EPD_SetRotate(270);
	
	sprintf(EpdPrint,
		"[FNC1]%08X%08X",
		((uint32_t)((uniqueId >> 32) & 0xFFFFFFFF)),
		((uint32_t)(uniqueId & 0xFFFFFFFF)));

	size_t barcode_length = code128_estimate_len(EpdPrint);
	char *barcode_data = (char *) malloc(barcode_length);

	barcode_length = code128_encode_gs1(EpdPrint, barcode_data, barcode_length);

	/* barcode_length is now the actual number of "bars". */
	for (i = 0; i < barcode_length; i++) {
		if (barcode_data[i])
			EPD_line(i + 20, 0, i + 20, 50, 1);
	}
	
	sprintf(EpdPrint,
		" ID: %08X%08X",
		((uint32_t)((uniqueId >> 32) & 0xFFFFFFFF)),
		((uint32_t)(uniqueId & 0xFFFFFFFF)));
	
	EPD_Text(0, 55, EpdPrint, 1, 0);

	
	
	qrcode_initText(&qrcode, qrcodeData, 5, 1, EpdPrint);
	for (uint8_t y = 0; y < qrcode.size; y++) 
	{
		for (uint8_t x = 0; x < qrcode.size; x++) 
		{
			EPD_DrawFilledRectangle(2 * x + 98, 2 * y + 86, 1, 1, qrcode_getModule(&qrcode, x, y));
		}
	}
	
	Touched = false;
	PowerState = true;	
	EPD_WriteScreen_Buff_Fast();
	while (DISP_BUSY) ;	
	EPD_DeepSleep();

		
	
	EPD_clear(1); //buf init
	for (i = 0; i < 100; i++)
	{
		if (Touched) break;
		Delay(100);	
	}
	

	EPD_SetRotate(0);

	EPD_HW_Init(); //Electronic paper initialization	

//	i = EPD_ReadStatus();
//	if (i == 0x01)
//	{
//		uPrintf("EPD Found\r\n");
//	}
//	else
//	{
//		uPrintf("EPD not Found, state: %02X\r\n", i);
//	}
	
	EPD_WhiteScreen_White();	
	EPD_clear(1); //buf init
	EPD_DeepSleep(); //EPD_DeepSleep	
		
//	FT6336_ReadData(TouchArr);         
//	SelfDest.Touches = TouchArr[0];
//	
//	if (SelfDest.Touches == 2)
//	{
//		SelfDest.X1 = TouchArr[2];
//		SelfDest.Y1 = ((TouchArr[3] & 1) << 8) + TouchArr[4];
//		SelfDest.X2 = TouchArr[8];
//		SelfDest.Y2 = ((TouchArr[9] & 1) << 8) + TouchArr[10];
//
//		MCU_OK(0);
//		while (true)
//		{
//	         
//		}
//		
//	}
//	
	


	
	//	while (1)
	//	{
	//		EPD_putLogo(alzilogo176, 110, sizeof(alzilogo176));
	//	
	//		EPD_HW_Init_Fast();
	//		EPD_PartWriteFull();
	//		EPD_PartWriteFull();
	//		EPD_DeepSleep(); //EPD_DeepSleep
	//
	//		Delay(100);
	//	
	//		EPD_HW_Init_Fast();
	//		uPrintf("Disp Status: %02X\r\n", EPD_ReadStatus());
	//		EPD_clear(1); //buf init
	//
	//		EPD_PartWriteFull();
	//		EPD_PartWriteFull();
	//		uPrintf("Disp Status: %02X\r\n", EPD_ReadStatus());
	//		EPD_DeepSleep(); //EPD_DeepSlee
	//		
	//	}
		
	//	GPIO_PinModeSet(UART_RX_PORT, UART_RX_PIN, gpioModePushPull, 0); //
	//	
	//	
	//	
	//	CMU_ClockEnable(cmuClock_ACMP0, true);
	//	
	//	// Initialize with default settings
	//	ACMP_Init_TypeDef init = ACMP_INIT_DEFAULT;
	//	init.biasProg = 3;
	//	init.accuracy = acmpAccuracyHigh;
	//	init.vrefDiv = 55;
	//
	//	ACMP_Init(ACMP0, &init);
	//
	//	// Allocate CDODD0 to ACMP0 to be able to use the input
	//	GPIO->BBUSALLOC = GPIO_BBUSALLOC_BEVEN0_ACMP0;
	//	
	//	ACMP_ChannelSet(ACMP0, acmpInputVREFDIV1V25, acmpInputPB2);
	//	
	//	
	//	ACMP_GPIOSetup(ACMP0, gpioPortB, 6, true, false);
	//	
	//	while (!(ACMP0->IF & ACMP_IF_ACMPRDY)) ;
	//	
	//
	//	while (true)
	//	{
	//	         
	//	}
	//	
	
	
	
	
	
	//	

	
	
		//fts_ctpm_auto_clb();
	
	uPrintf("TH_DIFF: ");
	uPrintf("%02X\r\n", FT6336_ReadReg(FT6X36_REG_FILTER_COEF));
	uPrintf("CTRL: ");
	uPrintf("%02X\r\n", FT6336_ReadReg(FT6X36_REG_CTRL));
	uPrintf("TIMEENTERMONITOR: ");
	uPrintf("%02X\r\n", FT6336_ReadReg(FT6X36_REG_TIME_ENTER_MONITOR));
	uPrintf("PERIODACTIVE: ");
	uPrintf("%02X\r\n", FT6336_ReadReg(FT6X36_REG_TOUCHRATE_ACTIVE));
	uPrintf("PERIODMONITOR: ");
	uPrintf("%02X\r\n", FT6336_ReadReg(FT6X36_REG_TOUCHRATE_MONITOR));
	uPrintf("RADIAN_VALUE: ");
	uPrintf("%02X\r\n", FT6336_ReadReg(FT6X36_REG_RADIAN_VALUE));
	uPrintf("OFFSET_LEFT_RIGHT: ");
	uPrintf("%02X\r\n", FT6336_ReadReg(FT6X36_REG_OFFSET_LEFT_RIGHT));
	uPrintf("OFFSET_UP_DOWN: ");
	uPrintf("%02X\r\n", FT6336_ReadReg(FT6X36_REG_OFFSET_UP_DOWN));
	uPrintf("DISTANCE_LEFT_RIGHT: ");
	uPrintf("%02X\r\n", FT6336_ReadReg(FT6X36_REG_DISTANCE_LEFT_RIGHT));
	uPrintf("DISTANCE_UP_DOWN: ");
	uPrintf("%02X\r\n", FT6336_ReadReg(FT6X36_REG_DISTANCE_UP_DOWN));
	uPrintf("DISTANCE_ZOOM: ");
	uPrintf("%02X\r\n", FT6336_ReadReg(FT6X36_REG_DISTANCE_ZOOM));
	uPrintf("CIPHER: ");
	uPrintf("%02X\r\n", FT6336_ReadReg(FT6X36_REG_CHIPID));
	uPrintf("G_MODE: ");
	uPrintf("%02X\r\n", FT6336_ReadReg(FT6X36_REG_INTERRUPT_MODE));
	uPrintf("PWR_MODE: ");
	uPrintf("%02X\r\n", FT6336_ReadReg(FT6X36_REG_POWER_MODE));
	uPrintf("FIRMID: ");
	uPrintf("%02X\r\n", FT6336_ReadReg(FT6X36_REG_FIRMWARE_VERSION));
	uPrintf("FOCALTECH_ID: ");
	uPrintf("%02X\r\n", FT6336_ReadReg(FT6X36_REG_PANEL_ID));
	uPrintf("STATE: ");
	uPrintf("%02X\r\n", FT6336_ReadReg(FT6X36_REG_STATE));
	
	
	uPrintf("Threshold: %02X\r\n", FT6336_ReadReg(FT6X36_REG_THRESHHOLD));
	uPrintf("Filter: %02X\r\n", FT6336_ReadReg(FT6X36_REG_FILTER_COEF));
	uPrintf("Rate: %02X\r\n\r\n", FT6336_ReadReg(FT6X36_REG_TOUCHRATE_ACTIVE));
	
	
	AdcInit();
	
	

	
	//	while (true)
	//	{
	//		if (FT6336_ReadLocation(TouchArr))
	//		{
	//			TouchX = TouchArr[0];
	//			TouchY = (TouchArr[1] << 8) + TouchArr[2];
	//			uPrintf("X: %03u Y: %03u\r\n", TouchX, TouchY);
	//			Delay(100);
	//		}
	//			
	//	}
	//	
	
	
	
	TouchFound = true;
	TouchModel = FT6336_ReadId();
	if (TouchModel == FT6206_CHIPID) sprintf(EpdPrint, "FT6206 Touch");
	else if (TouchModel == FT6236_CHIPID) sprintf(EpdPrint, "FT6236 Touch");
	else if (TouchModel == FT6336_CHIPID) sprintf(EpdPrint, "FT6336 Touch");
	else 
	{
		TouchFound = false;
		sprintf(EpdPrint, "Touch Panel NG");
	}
	

	uPrintf("%s\r\n", EpdPrint);
	EPD_Text(0, 30, EpdPrint, 1, 0);


	
	SpiFlashExitSleep();
	SpiFlashExitSleep();
	uPrintf("ROM JEDEC Table:\r\n");
	size_t FlashSize = SpiFlashGetSize();
	
	if (FlashSize > 0 && (FlashSize & -FlashSize) == FlashSize) //check if pow of 2
	{
		sprintf(EpdPrint, "ROM %uM", FlashSize / 1024);
		
	}
	else  sprintf(EpdPrint, "ROM ERR");
	SpiFlashUDPD(); //Ultra-Deep Power-Down of Flash chip
	
	uPrintf("%s\r\n", EpdPrint);
	EPD_Text(0, 48, EpdPrint, 1, 0);
	
	
	
	PsramReset();
	Delay(100);
	PsramReadId(Psram.Buf);
	uPrintf("RAM ID: %02X %02X %02X\r\n", Psram.Buf[0], Psram.Buf[1], Psram.Buf[2]);
	if (Psram.Buf[0] == 0x0D && Psram.Buf[1] == 0x5D)
	{
		Psram.Size = (Psram.Buf[2] & 0xE0) << 7;
		Psram.Found = true;
		sprintf(EpdPrint, "RAM %uM", Psram.Size ? Psram.Size / 1024 : 2);
	}
	else 
	{
		Psram.Found = false;
		sprintf(EpdPrint, "RAM ERR");
	}
	
	uPrintf("%s\r\n", EpdPrint);
	EPD_Text(96, 48, EpdPrint, 1, 0);
	
	
	EPD_putLogo(alzilogo176, 0, sizeof(alzilogo176));
	
	EPD_HW_Init_Fast();
	
	//	ShowMenu();	
	
	if (Touched)
	{
		sprintf(EpdPrint, "Touch INT OK");

	}
	else 
	{
		sprintf(EpdPrint, "--Touch INT NA");
	}
	
	EPD_Text(0, 64, EpdPrint, 1, 0);
	EPD_PartWriteFull(true);
	EPD_DeepSleepNoWait();
	
	
	Touched = false;
	
	
	if (RamTest())
	{
		sprintf(EpdPrint, "RAM OK        ");
		
	}
	else 
	{
		sprintf(EpdPrint, "--RAM ERROR-- ");
	}
	
	EPD_Text(0, 80, EpdPrint, 1, 0);
	EPD_HW_Init_Fast();
	EPD_PartWriteFull(true);
	EPD_DeepSleepNoWait();
	
	
	if (RomTest())
	{
		sprintf(EpdPrint, "ROM OK        ");
		
	}
	else 
	{
		sprintf(EpdPrint, "--ROM ERROR-- ");
	}
	
	EPD_Text(0, 96, EpdPrint, 1, 0);
	//EPD_HW_Init_Fast();
	EPD_PartWriteFull(true);
	EPD_DeepSleepNoWait();
	
	AdcInit();
	
	IADC_command(IADC0, iadcCmdStartSingle);
	Delay(200);
	uint16_t Volt = 175 - (MainVoltage * 9.0f);
	
	sprintf(EpdPrint, "Voltage: %.1fV", MainVoltage);
	EPD_Text(0, 112, EpdPrint, 1, 0);
	EPD_PartWriteFull(true);
	EPD_DeepSleep();
	
	MCU_OK(0);
	Delay(200);
	MCU_OK(1);
	Delay(200);
	MCU_OK(0);
	Delay(200);
	MCU_OK(1);
	Delay(200);
	MCU_OK(0);
	Delay(200);
	MCU_OK(1);

		
	sprintf(EpdPrint, "NFC Data:");
	EPD_Text(0, 128, EpdPrint, 1, 0);
	EPD_PartWriteFull(true);
	EPD_DeepSleep();
	NfcTest();
	
	while (1) ;
	
	
	
	//Touched = false;
	
	//	EPD_DeepSleep(); //EPD_DeepSleep

	
	
		//Delay(100);
		//	
		//	EPD_HW_Init_Fast();
		//	
		//	EPD_clear(1); //buf init
		//
		//	EPD_PartWriteFull();
		//	
		//	
		//	Shuffle((char*)TcVal);
		//	
		//	EPD_putLogo(alzilogo176, 22, sizeof(alzilogo176));
		//	ShowButtons();
		//	EPD_PartWriteFull();
		//	
		//	EPD_DeepSleep(); //EPD_DeepSlee
	

		//	if (Touched)
		//	{
		//		
		//		if (TouchX < 50 && TouchY > 240) 
		//		{
		//			EPD_clear(1); //buf init
		//			sprintf(EpdPrint, "Calibration");
		//			EPD_Text(0, 0, EpdPrint, 1, 0);
		//			sprintf(EpdPrint, "Don't touch");
		//			EPD_Text(0, 20, EpdPrint, 1, 0);
		//
		//			EPD_HW_Init_Fast();
		//			EPD_PartWriteFull();
		//			EPD_DeepSleep(); //EPD_DeepSleep
		//			
		//			fts_ctpm_auto_clb();
		//
		//			EPD_HW_Init(); //Electronic paper initialization	
		//			EPD_WhiteScreen_White();
		//			EPD_clear(1); //buf init
		//			
		//			EPD_putLogo(alzilogo176, 22, sizeof(alzilogo176));
		//			ShowButtons();
		//			EPD_PartWriteFull();
		//	
		//			EPD_DeepSleep(); //EPD_DeepSleep
		//			
		//			
		//		}
	
	
	
			
	//PW
	
	
//	EPD_DeepSleep();
	
	
	j = 0;
	bool DispOk = true;
	bool NeedUpdate = false;
	TickOld = TickCount;
	DispState = false;
	
	PassOk = true;
	
	if (Touched && TouchY > 200)
	{
		
	
		FullClear();
		EPD_putLogo(alzilogo176, 0, sizeof(alzilogo176));

		
		for (i = 0; i < 6; i++)
		{
			EPD_DrawCircle(i * 26 + 22, 77, 12, 1);
		}
	
	
		Shuffle((char*)TcVal);			
		ShowButtons();
		
		while (true)
		{
			if (TickCount > TickOld) DispOk = true;
		
			if (!DISP_BUSY && DispState)
			{
				EPD_DeepSleepNoWait();
				DispState = false;
				TickOld = TickCount + 100;
			}

			if (NeedUpdate)
			{
				if (DispOk && !DispState)
				{
					EPD_PartWriteFullNoWait(true);
					DispState = true;
					DispOk = false;
					NeedUpdate = false;
				}	
			}
	
			if (Touched)
			{
				if (TouchY < 176)
				{
					TouchX = 175 - TouchX;
					TouchY = 265 - TouchY;
					
					xScreen = TouchX / 44.0f;
					yScreen = (TouchY - 88) / 44.0f;
					
					xCent = modff(xScreen, &mfX);
					yCent = modff(yScreen, &mfY);
						
					x = mfX; 
					y = mfY;
					
					if (((xCent > 0.2f && xCent < 0.8f) || x == 0 || x == 3) && 
						((yCent > 0.2f && yCent < 0.8f) || y == 0 || y == 3))
					{
						//button location is OK
					
						EPD_DrawFilledCircle(j * 26 + 22, 77, 12, 1);
						j++;
						NeedUpdate = true;

						Touched = false;
					
						//	EPD_HW_Init_Fast();
	
						ch[0] = TcVal[y][x];
						ch[1] = 0;
					
						//				uPrintf("X: %03u Y: %03u  %u-%u  %.2f  %.2f\r\n", 
						//					175 - TouchX, 265 - TouchY, x, y, 
						//					(175 - TouchX) / 44.0f, (265 - TouchY - 88) / 44.0f);
								
						//					EPD_DrawFilledRectangle(x * 44 + 2, y * 44 + 2 + 88, x < 3 ? 40 : 39, y < 3 ? 40 : 39, 1);
						//					EPD_Text(x * 44 + 16, y * 44 + 15 + 88, ch, 0, 1);
						//					
						//					EPD_putLogo(alzilogo176, 22, sizeof(alzilogo176));
						//					EPD_PartWriteFull(true);
						//					EPD_clear(1); //buf init
						//					EPD_putLogo(alzilogo176, 22, sizeof(alzilogo176));
					
						uPrintf("%c", ch[0]);
						PassEntered[PassLen++] = ch[0];
						if (PassLen == sizeof(PassIs) - 1) 
						{
							for (i = 0; i < PassLen; i++)
							{
								if (PassIs[i] != PassEntered[i])
								{
									//								EPD_clear(1); //buf init
									//								EPD_PartWriteFull(true);
									//								EPD_line(0, 0, 175, 263, 1);
									//								EPD_line(175, 0, 0, 263, 1);
									//								EPD_WriteScreen_Buff_Fast();
									//								
									//								EPD_DeepSleep();
									PassOk = false;
									uPrintf("\r\nPass err\r\n");
								
									break;
									//while (true) ;
								}
							}


							while (FT6336_ReadLocation(TouchArr))
							{
								Delay(10);
							}
							Touched = false;
							if (!PassOk) break;
							uPrintf("\r\nPass OK\r\n");						
							break;
						}
					
						while (FT6336_ReadLocation(TouchArr))
						{
							Delay(10);
						}
					
						//					Shuffle((char*)TcVal);			
						//					ShowButtons();
						//					EPD_DeepSleep(); 			
					}
					
				}

				Touched = false;
			
			}
		 
	
		}

		FullClear();
	

		if (PassOk) EPD_Text(4, 66, "PASS OK", 1, 0);
		else EPD_Text(4, 66, "PASS ERR", 1, 0);
	
	}
	else
	{
		//SN
	
		sprintf(EpdPrint,
			"%08X%08X",
			((uint32_t)((uniqueId >> 32) & 0xFFFFFFFF)),
			((uint32_t)(uniqueId & 0xFFFFFFFF)));
			
		EPD_Text(4, 66, EpdPrint + 2, 1, 0);
	}
	
	EPD_putLogo(alzilogo176, 0, sizeof(alzilogo176));
	ShowMenu();	
	
	Touched = false;
	
	while (1)
	{

		
		if (Touched)
		{
			if (TouchY < 176)
			{
				TouchX = 175 - TouchX;
				TouchY = 265 - TouchY;
								
				xScreen = TouchX / 58.67f;
				yScreen = (TouchY - 88) / 58.67f;
								
				xCent = modff(xScreen, &mfX);
				yCent = modff(yScreen, &mfY);
									
				x = mfX; 
				y = mfY;
								
				if (((xCent > 0.2f && xCent < 0.8f) || x == 0 || x == 3) && 
					((yCent > 0.2f && yCent < 0.8f) || y == 0 || y == 3))
				{
			
					uPrintf("Menu item: %u, %u\r\n", x, y);
					//EPD_HW_Init_Fast();
					EPD_DrawFilledRectangle(x * 59 + 2, y * 59 + 2 + 88, x < 2 ? 55 : 53, y < 2 ? 55 : 53, 1);
					EPD_PartWriteFull(true);
					
					//Select item
					
					
					if (x == 1 && y == 2)
					{
						FullClear();
						EPD_Text(0, 0, "               ", 0, 1);
						EPD_Text(0, 2, "   FULL TEST   ", 0, 1);
						
						//RAM
						EPD_Text(0, 25, "RAM TEST", 1, 0);
						if (RamTest())
						{
							EPD_Text(110, 25, "OK", 1, 0);
						}
						else 
						{
							EPD_Text(110, 25, "NG", 0, 1);
						}
							
						
						EPD_PartWriteFull(true);
						memset(FrameBuffer  + 1936, 0x00, sizeof(FrameBuffer) - 3344);
						EPD_PartWriteFull(true);
						memset(FrameBuffer  + 1936, 0xFF, sizeof(FrameBuffer) - 3344);
						
						//PadClear();
						
						//ROM		
						EPD_Text(0, 45, "ROM TEST", 1, 0);
						if (RomTest())
						{
							EPD_Text(110, 45, "OK", 1, 0);
						}
						else 
						{
							EPD_Text(110, 45, "NG", 0, 1);
						}
						
					}
					//Tetris
					else if(x == 2 && y == 2)
					{
						FullClear();
						Game();

					}
					
					//Touch
					else if(x == 1 && y == 1)
					{
						FullClear();
						EPD_PartWriteFull(true);
						SensTest();
					}

					//RamTest
					else if(x == 0 && y == 0)
					{
						PadClear();
						EPD_Text(0, 95, " RAM TEST", 1, 0);
						EPD_PartWriteFull(true);
						
						RamTest();

					}
					
					//ROM test
					else if(x == 1 && y == 0)
					{
						PadClear();
						EPD_Text(0, 95, " ROM TEST", 1, 0);
						EPD_PartWriteFull(true);

						RomTest();
					}
						
					//DISP test
					else if(x == 2 && y == 0)
					{
						FullClear();
						EPD_PartWriteFull(true);
						DispTest();

					}
						
					//NFC test
					else if(x == 0 && y == 1)
					{
						FullClear();
						EPD_PartWriteFull(true);
						NfcTest();

					}
					
					//FREQ test
					else if(x == 2 && y == 1)
					{
						FullClear();
						EPD_PartWriteFull(true);
						FreqVoltTest();

					}
					
					
					//DEMO test
					else if(x == 0 && y == 2)
					{
						FullClear();
						EPD_PartWriteFull(true);
						Demo();

					}
					
					EPD_Text(0, 248, "Touch to reset", 1, 0);
					EPD_PartWriteFull(true);
					EPD_DeepSleepNoWait();
					
					
					while (FT6336_ReadLocation(TouchArr))
					{
						Delay(10);
					}
					while (!FT6336_ReadLocation(TouchArr))
					{
						Delay(10);
					}
					Touched = false;
					//ShowMenu();
					
					
					NVIC_SystemReset();
			
				}
			}
			Touched = false;
		}
		
		

	}
	

	
	Delay(500);
	
	//	POW_2V8(0); 
	//	POW_I2C_T(0);
	//	PowerState = false;
	
		//	POW_2V8(1); // enable 2.8 volt
		//	POW_I2C_T(1); // enable i2c level translator
		//	PowerState = true;
		//	Delay(1000);
	
	
	
	SpiFlashExitSleep();
	EPD_HW_Init_Fast(); //EPD init
	Epaper_Write_Command(0x24); //write RAM for black(0)/white (1)	
	
	FlashEpdP = 0;
	FBp = 0;	
	for (uint16_t j = 0; j < 5808; j++)
	{
		if (!FlashEpdP)
		{
			SpiFlashReadPage(FlashtoEpd, FBp);
			FBp += 0x100;
		}
		Epaper_Write_Data(FlashtoEpd[FlashEpdP]);
		FlashEpdP++;
		if (FlashEpdP == 256) FlashEpdP = 0;
	}

		
	
	SpiFlashUDPD(); //Ultra-Deep Power-Down of Flash chip
	EPD_Update_Fast();	
	while (DISP_BUSY) ;	
	EPD_DeepSleep(); //EPD_DeepSleep

//
//	POW_2V8(0); 
//	POW_I2C_T(0);
//	PowerState = false;
	
	
	


	while (true)
	{
		PsramPage = 0;
		while (true)
		{
			
			if (Touched && AllPacksReceived)
			{
				SpiFlashExitSleep();
		
			
				Touched = false;
				PicBase += 0x10000;
				if (PicBase > 0x50000) PicBase = 0;
					
			
				EPD_HW_Init_Fast(); //EPD init
				Epaper_Write_Command(0x24); //write RAM for black(0)/white (1)
						
				FlashEpdP = 0;
				FBp = PicBase;	
				for (uint16_t j = 0; j < 5808; j++)
				{
					if (!FlashEpdP)
					{
						SpiFlashReadPage(FlashtoEpd, FBp);
						FBp += 0x100;
					}
					Epaper_Write_Data(FlashtoEpd[FlashEpdP]);
					FlashEpdP++;
					if (FlashEpdP == 256) FlashEpdP = 0;
				}
						
						
				EPD_Update_Fast();	
				EPD_DeepSleep(); //EPD_DeepSleep
			
				Touched = false;

			}
			
			
			
			if (AllPacksReceived && PsramPage) break;
			else AllPacksReceived = false;
		
			NfcDataLen = NfcWork();
			if (NfcDataLen)
			{
				if ((NfcDataLen & 0x0FFF) > 2) 
				{
					
					//if ((NfcDataLen & 0x0FFF) > 200) PsramPage++;
					//uPrintf("Big: %u\r\n", NfcDataLen);
					if (NfcDataLen & 0x8000)
					{
						
						uPrintf("Response: > ");
						for (uint32_t i = 0; i < (NfcDataLen & 0x0FFF); i++) 
						{
							//if (!(i % 32)) uPrintf("\r\n");
							uPrintf("%02X ", NfcData[i]);
						}
						uPrintf("<\r\n\r\n");
					}
					else 
					{
						
						if (NfcDataLen > 200) 
						{
							TIMER1->CNT = 0; //timeout reset
							
							AddDataPos = NfcData[4] << 1; //tail addr in 1-st page
							//uPrintf("%02u ", PsramPage);
							uPrintf(".");
							
							PsramDmaWrite(1 + PsramPage++, NfcData); //leave first page for pack tails
												
						}
						else if (NfcDataLen == 5 && NfcData[0] == 0x02 && PackLoaded) //if pack tail 2 bytes
						{
							PsramWrite(AddDataPos, NfcData + 1, 2); //tail write
							PackLoaded = false;
						}
					
						//uPrintf("Request: %u bytes", NfcDataLen & 0x0FFF);
						
//						for (uint32_t i = 0; i < (NfcDataLen); i++) 
//						{
//							if (!(i % 32)) uPrintf("\r\n");
//							uPrintf("%02X ", NfcData[i]);
//						}
						//uPrintf("\r\n");						
					}
				}

				NfcDecodeReset();
			}

			
		}

	
		//		for (uint16_t pg = 0; pg < PsramPage; pg++)
		//		{
		//			PsramDmaRead(pg, NfcData);
		//			for (uint32_t i = 0; i < 256; i++) 
		//			{
		//				if (!(i % 32)) uPrintf("\r\n");
		//				uPrintf("%02X ", NfcData[i]);
		//			}
		//			uPrintf("\r\n");
		//
		//			PsramDmaRead(pg + 0x4000, NfcData);
		//			for (uint32_t i = 0; i < 32; i++) 
		//			{
		//				if (!(i % 32)) uPrintf("\r\n");
		//				uPrintf("%02X ", NfcData[i]);
		//			}
		//			uPrintf("\r\n");
		//			
		//		}		
		
		uPrintf("\r\nTotal: %u Big Packs\r\n", PsramPage);
		
		SpiFlashExitSleep();
	
		FlashEpdP = 0;
		FBp = PicBase;
						
		if (PassOk)
		{
						
			for (uint16_t j = 0; j < 60; j++)
			{
				PsramDmaRead(j + 1, Psram.Buf);
				while (Psram.Busy) ;
				
				for (uint16_t i = 6; i < 254; i++)
				{
					dispw(Psram.Buf[i]);
								
				}
							
				PsramRead(j << 1, Psram.Buf, 2);
				dispw(Psram.Buf[0]);
				dispw(Psram.Buf[1]);
			}
			
			EPD_HW_Init_Fast(); //EPD init
			Epaper_Write_Command(0x24); //write RAM for black(0)/white (1)
						
			FlashEpdP = 0;
			FBp = PicBase;	
			for (uint16_t j = 0; j < 5808; j++)
			{
				if (!FlashEpdP)
				{
					SpiFlashReadPage(FlashtoEpd, FBp);
					FBp += 0x100;
				}
				Epaper_Write_Data(FlashtoEpd[FlashEpdP]);
				FlashEpdP++;
				if (FlashEpdP == 256) FlashEpdP = 0;
			}
						
						
			EPD_Update_Fast();	
			EPD_DeepSleep(); //EPD_DeepSleep
			
		
		}		
		
		

		
		
	}
		


	
	//	while (1)
	//	{
	//	
	//			
	//		
	//		//					
	//		#ifdef SHOWPACK
	//														
	//								if (VcdDataLen > 3)
	//								{
	//									uPrintf("Len %02u: > ", VcdDataLen);
	//									for (uint32_t i = 0; i < VcdDataLen; i++) 
	//									{
	//										uPrintf("%02X ", VcdData[i]);
	//									}
	//									uPrintf("<\r\n");
	//		//							while (true)
	//		//							{
	//		//	         
	//		//							}
	//								}
	//								else
	//								{
	//									uPrintf("%u ", VcdDataLen);
	//								}
	//		#endif						
	//								
	//								if (VcdDataLen > 200) 
	//								{
	//									AddDataPos = VcdData[4] << 1; //tail addr in 1-st page
	//									//uPrintf("%02u ", PsramPage);
	//									uPrintf(".");
	//									PsramDmaWrite(1 + PsramPage++, VcdData); //leave first page for pack tails
	//					
	//							
	//									PackLoaded = true; //next should be tail
	//								}
	//								else if (VcdDataLen == 5 && VcdData[0] == 0x02 && PackLoaded) //if pack tail 2 bytes
	//								{
	//									PsramWrite(AddDataPos, VcdData + 1, 2); //tail write
	//									PackLoaded = false;
	//								}
	//								else if(VcdDataLen > 40 && VcdDataLen < 100 && VcdData[11] == 0x72 && VcdData[12] == 0x75)
	//								{
	//									text test
	//															
	//															uPrintf("Len: %02u, ", VcdDataLen);
	//																
	//															for (uint8_t i = 0; i < VcdDataLen; i++) 
	//															{
	//																if (i < 32)
	//																{
	//																	uPrintf("%02X ", VcdData[i]);
	//																}
	//															}
	//															if (i >= 32) uPrintf(">>>");
	//															uPrintf("\r\n");
	//															
	//															
	//															
	//																PsramDmaWrite(PsramPage++, VcdData); //leave first page for pack tails
	//																NfcFifo.TimeOut = true; //rx timeout
	//
	//																return;
	//															
	//	}
	//						
	//						
	//							PrintPos = 0;
	//							for (uint8_t i = 0; i < ((VcdDataLen > 32) ? 32 : VcdDataLen); i++) 
	//							{
	//								PrintPos += sprintf(PrintBuf + PrintPos, "%02X ", VcdData[i]);
	//							}
	//							sprintf(PrintBuf + PrintPos, "\r\n");
	//							uPrintf(PrintBuf);
	//					
	//						
	//						
	//#ifndef SHOWPACK						
	//												//check if end of pack
	//	if (NfcData[0] == 0xE0 && NfcData[1] == 0x80 && NfcDataLen == 4) //end of data
	//	{
	//		uPrintf("<\r\n");
	//	}
	//#endif 
	//
	//					

		

	//
	//	//
		//				
		//				
		//				
		//				
		//				if (NfcFifo.TimeOut)
		//				{
		//					if (PsramPage > 1)
		//					{			
		//						uPrintf("Loaded %u pages\r\n", PsramPage);
								//PsramInit(); //Psram ports init	

						
						
								//						PsramDmaRead(1, Psram.Buf);
								//						
								//						for (uint8_t i = 0; i < 32; i++) 
								//						{
								//							uPrintf("%02X ", Psram.Buf[i]);
								//						}
								//						uPrintf("\r\n");
						

								//						
								//						cnnt = 0;
								//						
								//						SpiFlashExitSleep();
								//	
								//						FlashEpdP = 0;
								//						FBp = PicBase;
								//						
								//	
								//						
								//						for (uint16_t j = 0; j < 60; j++)
								//						{
								//							PsramDmaRead(j + 1, Psram.Buf);
								//							while (Psram.Busy) ;
								//				
								//							for (uint16_t i = 6; i < 254; i++)
								//							{
								//								dispw(Psram.Buf[i]);
								//								
								//							}
								//							
								//							PsramRead(j << 1, Psram.Buf, 2);
								//							dispw(Psram.Buf[0]);
								//							dispw(Psram.Buf[1]);
								//			
								//
								//						}
								//						
						
						
								//						POW_2V8(1); // enable 2.8 volt
								//						POW_I2C_T(1); // enable i2c level translator
								//						PowerState = true;
								//						Delay(1000);
						
	EPD_HW_Init_Fast(); //EPD init
	Epaper_Write_Command(0x24); //write RAM for black(0)/white (1)
						
	FlashEpdP = 0;
	FBp = PicBase;	
	for (uint16_t j = 0; j < 5808; j++)
	{
		if (!FlashEpdP)
		{
			SpiFlashReadPage(FlashtoEpd, FBp);
			FBp += 0x100;
		}
		Epaper_Write_Data(FlashtoEpd[FlashEpdP]);
		FlashEpdP++;
		if (FlashEpdP == 256) FlashEpdP = 0;
	}
						
						
	EPD_Update_Fast();	
	EPD_DeepSleep(); //EPD_DeepSleep

	//						POW_2V8(0); 
	//						POW_I2C_T(0);
	//						PowerState = false;
						
						
						
						
	//						NfcFifo.TimeOut = false; 
	//						PsramPage = 0;
						


		
		
		
	
		


	
	
	
	
	
	
	
	
	
	
	
	
	
	
	

	
	POW_2V8(1); // enable 2.8 volt
	PowerState = true;
	
	TOUCH_RES(1); //reset touch
	Delay(100);
	TOUCH_RES(0); //reset touch

	
	
	//start UART receive
	res = DMADRV_AllocateChannel(&channelRX, NULL);
	if (res == ECODE_OK)
	{
		res = DMADRV_PeripheralMemoryPingPong(channelRX,
			dmadrvPeripheralSignal_EUSART1_RXDATAV,
			rxBuffer.buffer,
			rxBuffer.buffer+(RX_BUFFER_SIZE >> 1),
			(void*)&(EUSART1->RXDATA),
			true,
			RX_BUFFER_SIZE >> 1,
			dmadrvDataSize1,
			NULL,
			NULL);
	}
	
	
	

		

	
	
	//	
	//	while (true)
	//	{
	//		
	//		//fifo
	//		rxBuffer.startIndex = LDMA->CH[channelRX].DST - (uint32_t)&rxBuffer.buffer[0];
	//		
	//		while (rxBuffer.stopIndex != rxBuffer.startIndex) 
	//		{
	//			rxBuffer.startIndex = LDMA->CH[channelRX].DST - (uint32_t)&rxBuffer.buffer[0];
	//			
	//			ReceivedChar = rxBuffer.buffer[rxBuffer.stopIndex++];
	//			if (rxBuffer.stopIndex == RX_BUFFER_SIZE) rxBuffer.stopIndex = 0;
	//			uPrintf("%c", ReceivedChar);
	//		}
	//		
	//		
	//	}
	

	
	
	
	Delay(60000);
	NVIC_SystemReset();
	
	
	
	

	char txbuf[128];
	while (1)
	{
		//		for (i = 0; i < 16; i++) 
		//		{
		//	    
		//			EPD_clear(1); //buf init
		//			sprintf(txbuf, "%02X  ", i);
		//			EPD_Text(i * 10, i * 15, txbuf, 1, 0);
		//	    
		//			EPD_Part_Write(0, 0, 0, 0);
		//			while (DISP_BUSY) ;	
		//			//EPD_WriteScreen_Buff_Fast();
		//			EPD_DeepSleep(); //EPD_DeepSlee
		//	    
		//			
		//			LEDT;
		//       
		//		}
		EPD_HW_Init(); //Electronic paper initialization	
		EPD_WhiteScreen_White();
		EPD_DeepSleep(); //EPD_DeepSlee
		Delay(15000);
 
	}
	
	
	
	return 0;
}






















