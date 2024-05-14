
#include "tests.h"



#define ROWS 20 // you can change height and width of table with ROWS and COLS 
#define COLS 15
#define TRUE 1
#define FALSE 0

uint32_t Page;
uint8_t Buf[256];
extern char EpdPrint[];
extern bool Touched;

extern uint16_t TouchX;
extern uint16_t TouchY;

uint16_t RomAdrMap[4096];

int depth = 9; 

uint8_t BtnLeft;
uint8_t BtnRight;
uint8_t BtnTurn;
uint8_t BtnDown;

extern volatile struct NfcFifo_t NfcFifo;
extern volatile unsigned int DmaChannelTim0;
	
extern volatile double MainVoltage;

Shape current;

char Table[ROWS][COLS] = { 0 };
int score = 0;
char GameOn = TRUE;

extern bool DispState;
QRCode qrcode;

extern uint64_t uniqueId;

bool RamTest()
{

	uint8_t i, r, x;
	uint32_t CrcIn, CrcOut;
	
	uint8_t qrcodeData[qrcode_getBufferSize(5)];	
	
	
		
	EPD_Text(0, 80, "Testing RAM...", 1, 0);
	EPD_PartWriteFull(true);
	EPD_DeepSleep();

	
	GPCRC_Init_TypeDef init_c = GPCRC_INIT_DEFAULT;
	CMU_ClockEnable(cmuClock_GPCRC, true);
	
	GPCRC_Init(GPCRC, &init_c);
	GPCRC_Start(GPCRC);
	
	x = 7;
	for (Page = 0; Page < 256; Page++) 
	{
		srand(EfmGetRandom());
		for (i = 0;; i++)
		{
			r = rand();
			GPCRC->INPUTDATABYTE = r;
			Buf[i] = r;
			if (i == 255) break;
		}
		PsramDmaWrite(Page, Buf);
		
//		if (!DISP_BUSY && DispState)
//		{
//			EPD_DeepSleepNoWait();
//			DispState = false;
//		}
//		if (!(Page % 0x0FFF))
//		{
//			EPD_DrawFilledRectangle(x, 135, 16, 15, 1);
//			EPD_PartWriteFullNoWait(true);
//			DispState = true;
//			
//			x += 18;
//		}
		
	}
	CrcIn = GPCRC->DATA;
	//sprintf(EpdPrint, "CRC: %08X  ", CrcIn);
	//EPD_Text(0, 135, EpdPrint, 1, 0);
	
	//EPD_Text(0, 115, "Checking   ", 1, 0);

	//while (DISP_BUSY) ;
	
	//EPD_PartWriteFull(true);
	//EPD_DeepSleepNoWait();
	
	
	GPCRC_Start(GPCRC);
	x = 7;
	for (Page = 0; Page < 256; Page++) 
	{
		PsramDmaRead(Page, Buf);
		
		for (i = 0;; i++)
		{
			GPCRC->INPUTDATABYTE = Buf[i];
			if (i == 255) break;
		}
		
//		if (!DISP_BUSY && DispState)
//		{
//			EPD_DeepSleepNoWait();
//			DispState = false;
//		}
		
//		if (!(Page % 0x0FFF))
//		{
//
//			EPD_DrawFilledRectangle(x, 155, 16, 15, 1);
//			EPD_PartWriteFullNoWait(true);
//			DispState = true;
//			x += 18;
//		}
	}
	
	CrcOut = GPCRC->DATA;
	
	//while (DISP_BUSY) ;
	char rep[16];
	
	//sprintf(EpdPrint, "CRC: %08X  ", CrcOut);
//	EPD_Text(0, 80, EpdPrint, 1, 0);
//	if (CrcOut == CrcIn)
//	{
//		sprintf(rep, " RAM TEST OK");
//	}
//	else 
//	{
//		sprintf(rep, " RAM TEST NG");
//	}
//
//	EPD_Text(0, 80, rep, 1, 0);
//	sprintf(EpdPrint,
//		"%s\nID: %08X%08X",
//		rep + 1,
//		((uint32_t)((uniqueId >> 32) & 0xFFFFFFFF)),
//		((uint32_t)(uniqueId & 0xFFFFFFFF)));
//	
//	qrcode_initText(&qrcode, qrcodeData, 5, 1, EpdPrint);
//	
//	for (uint8_t y = 0; y < qrcode.size; y++) 
//	{
//		for (uint8_t x = 0; x < qrcode.size; x++) 
//		{
//			EPD_setPoint(x + 5, y + 200, qrcode_getModule(&qrcode, x, y));
//		}
//	}

	
//
//	EPD_HW_Init_Fast();
//	
//	EPD_WriteScreen_Buff_Fast();
//	while (DISP_BUSY) ;	
//	EPD_DeepSleep();
	

	
	return (CrcOut == CrcIn);
}


/************************/
/************************/
/************************/
/************************/
/************************/
/************************/


bool RomTest()
{
	
	uint8_t i, r, x;
	uint32_t CrcIn, CrcOut;
	uint16_t AdrR, aRnd, aBuf;
	
//	uint8_t qrcodeData[qrcode_getBufferSize(5)];	
	
	
	EPD_Text(0, 96, "Testing ROM...", 1, 0);
	EPD_PartWriteFull(true);
	EPD_DeepSleepNoWait();
	

	
	GPCRC_Init_TypeDef init_c = GPCRC_INIT_DEFAULT;
	CMU_ClockEnable(cmuClock_GPCRC, true);
	
	GPCRC_Init(GPCRC, &init_c);
	GPCRC_Start(GPCRC);

	SpiFlashExitSleep();
	SpiFlashExitSleep();
	
	srand(EfmGetRandom());
	for (AdrR = 0; AdrR < 4096; AdrR++)
	{
		aRnd = rand() & 0x03;
		RomAdrMap[AdrR] = (AdrR << 2) + aRnd;
	}
	
	srand(EfmGetRandom());
	for (uint16_t d = 0; d < 10000; d++)
	{
		aRnd = rand() & 0x0FFF;
		AdrR = rand() & 0x0FFF;
		
		aBuf = RomAdrMap[aRnd];
		RomAdrMap[aRnd] = RomAdrMap[AdrR]; 
		RomAdrMap[AdrR] = aBuf;
	}
	
	
	EPD_Text(0, 96, "Flashing       ", 1, 0);
	EPD_PartWriteFull(true);
	EPD_DeepSleep(); //EPD_DeepSlee
	x = 7;
	
	for (AdrR = 0; AdrR < 128; AdrR++)
	{

		srand(EfmGetRandom());
		for (i = 0;; i++)
		{
			r = rand();
			GPCRC->INPUTDATABYTE = r;
			Buf[i] = r;
			if (i == 255) break;
		}
		
		while (SpiFlashReadStatus() & SPI_STATUS_BSY) ; //wait for ready 
		SpiFlashCmd(SPICMD_WREN);
		Page = RomAdrMap[AdrR] << 8;
		SpiFlashErasePage(Page); //erase page

		while (SpiFlashReadStatus() & SPI_STATUS_BSY) ; //wait for ready 
		SpiFlashCmd(SPICMD_WREN);
		SpiFlashWritePage(Buf, Page);		
//	
//		if (!DISP_BUSY && DispState)
//		{
//			EPD_DeepSleepNoWait();
//			DispState = false;
//		}
//		
//		if (!(AdrR % 0x4F))
//		{
//			EPD_DrawFilledRectangle(x, 135, 16, 15, 1);
//			EPD_PartWriteFullNoWait(true);
//			DispState = true;
//			x += 18;
//		}
	
	}
	
	CrcIn = GPCRC->DATA;
//	sprintf(EpdPrint, "CRC: %08X  ", CrcIn);
//	EPD_Text(0, 135, EpdPrint, 1, 0);
//	
//	
	EPD_Text(0, 96, "Checking   ", 1, 0);


	EPD_PartWriteFull(true);
	EPD_DeepSleep(); //EPD_DeepSlee
	
	GPCRC_Start(GPCRC);

	
	x = 7;
	for (AdrR = 0; AdrR < 128; AdrR++)
	{
		Page = RomAdrMap[AdrR] << 8;
		SpiFlashReadPage(Buf, Page);
		for (i = 0;; i++)
		{
			GPCRC->INPUTDATABYTE = Buf[i];
			if (i == 255) break;
		}
		
//		if (!DISP_BUSY && DispState)
//		{
//			EPD_DeepSleepNoWait();
//			DispState = false;
//		}
//		
//		if (!(AdrR % 0x1FF))
//		{
//			EPD_DrawFilledRectangle(x, 155, 16, 15, 1);
//			EPD_PartWriteFullNoWait(true);
//			DispState = true;
//			x += 18;
//		}
		
		
	}
	
		
	CrcOut = GPCRC->DATA;
	
//	while (DISP_BUSY) ;
//	char rep[16];
	
	
//	
//	sprintf(EpdPrint, "CRC: %08X  ", CrcOut);
//	EPD_Text(0, 155, EpdPrint, 1, 0);
//	if (CrcOut == CrcIn)
//	{
//		sprintf(rep, " ROM TEST OK");
//	}
//	else 
//	{
//		sprintf(rep, " ROM TEST NG");
//	}
//
//	EPD_Text(0, 178, rep, 1, 0);
//	sprintf(EpdPrint,
//		"%s\nID: %08X%08X",
//		rep + 1,
//		((uint32_t)((uniqueId >> 32) & 0xFFFFFFFF)),
//		((uint32_t)(uniqueId & 0xFFFFFFFF)));
//	
//	qrcode_initText(&qrcode, qrcodeData, 5, 1, EpdPrint);
//	
//	for (uint8_t y = 0; y < qrcode.size; y++) 
//	{
//		for (uint8_t x = 0; x < qrcode.size; x++) 
//		{
//			EPD_setPoint(x + 135, y + 200, qrcode_getModule(&qrcode, x, y));
//		}
//	}


	

//	EPD_HW_Init_Fast();
//	
//	EPD_WriteScreen_Buff_Fast();
//	while (DISP_BUSY) ;	
//	EPD_DeepSleep();
	

	SpiFlashUDPD(); //Ultra-Deep Power-Down of Flash chip
	
	return (CrcOut == CrcIn);
	
}

/************************/
/************************/
/************************/
/************************/
/************************/
/************************/

void DispTest()
{
	
	uint8_t x, y, i;
	

	Touched = false;
	while (true)
	{

		EPD_Text(0, 248, "Touch to reset", 1, 0);
	
		EPD_Text(0, 115, " Normal Clear", 1, 0);
		EPD_PartWriteFull(true);
		EPD_DeepSleep();
		
		Delay(1000);
	
		if (Touched) NVIC_SystemReset();
		
		srand(EfmGetRandom());
	
		EPD_SetRotate(0);
		
		EPD_HW_Init(); //Electronic paper initialization	
		EPD_WhiteScreen_White();	
		EPD_clear(1); //buf init

		EPD_HW_Init_Fast();

	
		EPD_Text(0, 248, "Touch to reset", 1, 0);
	
		EPD_Text(0, 115, "  Fast Write", 1, 0);
		EPD_WriteScreen_Buff_Fast();
		while (DISP_BUSY) ;	
		EPD_Text(0, 135, "  Fast Write", 1, 0);
		EPD_WriteScreen_Buff_Fast();
		while (DISP_BUSY) ;	
		EPD_Text(0, 95, "  Fast Write", 1, 0);
		EPD_WriteScreen_Buff_Fast();
		while (DISP_BUSY) ;	
		EPD_clear(1); //buf init
	
		Delay(500);
		if (Touched) NVIC_SystemReset();
		EPD_WriteScreen_Buff_Fast();

		Delay(500);
		if (Touched) NVIC_SystemReset();
		EPD_Text(0, 248, "Touch to reset", 1, 0);
	
		EPD_Text(0, 105, "    Ultra", 1, 0);
		EPD_Text(0, 125, "  Fast Write", 1, 0);
		
		EPD_PartWriteFull(true);
		EPD_DeepSleep();
		
		for (i = 0; i < 10; i++)
		{
			x = 10 + (rand() % (X_SIZE - 20));
			y = 10 + (rand() % (Y_SIZE - 20));
			EPD_DrawFilledCircle(x, y, 10, 1);
			EPD_PartWriteFull(true);
			EPD_DeepSleep();
			if (Touched) NVIC_SystemReset();
		}
	
		for (uint16_t d = 0; d < 1000; d++)
		{
			i = 2 + (rand() % 8);
			x = 10 + (rand() % (X_SIZE - 20));
			y = 10 + (rand() % (Y_SIZE - 20));
			EPD_DrawCircle(x, y, i, 1);
		}

		EPD_PartWriteFull(true);
		EPD_DeepSleep();
	
		Delay(1000);
		if (Touched) NVIC_SystemReset();

		EPD_clear(1); //buf init
//		EPD_PartWriteFull(true);
//		EPD_DeepSleep();
	
		drawSierpinski(10, X_SIZE + 10, X_SIZE - 10, X_SIZE + 10, X_SIZE / 2, 40);

		EPD_DrawFilledCircle(X_SIZE / 2, Y_SIZE / 2 + 5, 16, 1);
		EPD_DrawCircle(X_SIZE / 2, Y_SIZE / 2 + 5, 20, 1);
	
		EPD_PartWriteFull(true);
		EPD_DeepSleep();
	
		Delay(3000);
		if (Touched) NVIC_SystemReset();
		
		EPD_HW_Init(); //Electronic paper initialization	
		EPD_WhiteScreen_White();	
		EPD_clear(1); //buf init
		
		EPD_DeepSleep();
		if (Touched) NVIC_SystemReset();

	}
}


void drawSierpinski(float x1, float y1, float x2, float y2, float x3, float y3) 
{ 
	//Draw the 3 sides of the triangle as black lines 
	EPD_line(x1, y1, x2, y2, 1); 
	EPD_line(x1, y1, x3, y3, 1); 
	EPD_line(x2, y2, x3, y3, 1); 
 
	//Call the recursive function that'll draw all the rest. The 3 corners of it are always the centers of sides, so they're averages 
	subTriangle 
	( 
	  1, //This represents the first recursion 
		(x1 + x2) / 2, //x coordinate of first corner 
		(y1 + y2) / 2, //y coordinate of first corner 
		(x1 + x3) / 2, //x coordinate of second corner 
		(y1 + y3) / 2, //y coordinate of second corner 
		(x2 + x3) / 2, //x coordinate of third corner 
		(y2 + y3) / 2  //y coordinate of third corner 
	  ); 
} 
 
//The recursive function that'll draw all the upside down triangles 
void subTriangle(int n, float x1, float y1, float x2, float y2, float x3, float y3) 
{ 
	//Draw the 3 sides as black lines 
	EPD_line(x1, y1, x2, y2, 1); 
	EPD_line(x1, y1, x3, y3, 1); 
	EPD_line(x2, y2, x3, y3, 1); 
 
	//Calls itself 3 times with new corners, but only if the current number of recursions is smaller than the maximum depth 
	if (n < depth) 
	{ 
		//Smaller triangle 1 
		subTriangle 
		( 
		  n+1, //Number of recursions for the next call increased with 1 
			(x1 + x2) / 2 + (x2 - x3) / 2, //x coordinate of first corner 
			(y1 + y2) / 2 + (y2 - y3) / 2, //y coordinate of first corner 
			(x1 + x2) / 2 + (x1 - x3) / 2, //x coordinate of second corner 
			(y1 + y2) / 2 + (y1 - y3) / 2, //y coordinate of second corner 
			(x1 + x2) / 2, //x coordinate of third corner 
			(y1 + y2) / 2  //y coordinate of third corner 
		  ); 
		//Smaller triangle 2 
		subTriangle 
		( 
		  n+1, //Number of recursions for the next call increased with 1 
			(x3 + x2) / 2 + (x2 - x1) / 2, //x coordinate of first corner 
			(y3 + y2) / 2 + (y2 - y1) / 2, //y coordinate of first corner 
			(x3 + x2) / 2 + (x3 - x1) / 2, //x coordinate of second corner 
			(y3 + y2) / 2 + (y3 - y1) / 2, //y coordinate of second corner 
			(x3 + x2) / 2, //x coordinate of third corner 
			(y3 + y2) / 2  //y coordinate of third corner 
		  ); 
		//Smaller triangle 3 
		subTriangle 
		( 
		  n+1, //Number of recursions for the next call increased with 1 
			(x1 + x3) / 2 + (x3 - x2) / 2, //x coordinate of first corner 
			(y1 + y3) / 2 + (y3 - y2) / 2, //y coordinate of first corner 
			(x1 + x3) / 2 + (x1 - x2) / 2, //x coordinate of second corner 
			(y1 + y3) / 2 + (y1 - y2) / 2, //y coordinate of second corner 
			(x1 + x3) / 2, //x coordinate of third corner 
			(y1 + y3) / 2  //y coordinate of third corner 
		  ); 
	} 
}




/************************/
/************************/
/************************/
/************************/
/************************/
/************************/

//Tetris


void Game()
{
	score = 0;
	int c;
	srand(EfmGetRandom());
	
	SetNewRandomShape();
	PrintTable();
	

	Touched = false;
	while (GameOn) 
	{

		if (Touched)
		{
			//uPrintf("%u %u\r\n", TouchX, TouchY);
			
			
			if (TouchX > 120 && TouchY > 255)
			{
				GameOn = false;
				DeleteShape(current);
				
				EPD_Text(20, 245, "              ", 1, 0);
				EPD_HW_Init_Fast();
				EPD_WriteScreen_Buff_Fast();
				while (DISP_BUSY) ;	
				EPD_PartWriteFull(true);

				return;
			}
			
			if (TouchY > 176) 
			{
				BtnTurn = 1;
			}
			else if (TouchY <= 88)
			{
				BtnDown = 1;
			}
			else
			{
				if (TouchX < 88)
				{
					BtnRight = 1;
				}
				else BtnLeft = 1;
			}
			Touched = false;
		}
		
		
		if (BtnTurn)
		{
			BtnTurn--;
			BtnLeft = 0;
			BtnRight = 0;
			BtnDown = 0;
			ManipulateCurrent('w');

		}
			
		else if (BtnLeft)
		{
			BtnLeft--;
			BtnTurn = 0;
			BtnRight = 0;
			BtnDown = 0;
			ManipulateCurrent('a');

		}
			
		else if (BtnRight)
		{
			BtnRight--;
			BtnTurn = 0;
			BtnLeft = 0;
			BtnDown = 0;
			ManipulateCurrent('d');

		}
		else if (BtnDown)
		{
			BtnRight = 0;
			BtnTurn = 0;
			BtnLeft = 0;
			ManipulateCurrent('s');

		}
		else
		{
			ManipulateCurrent('s');
		}
		
	}
	DeleteShape(current);
	EPD_PartWriteFull(true);
	
}


const Shape ShapesArray[7] = {
	{ (char *[]){ (char[]){ 0, 1, 1 }, (char[]){ 1, 1, 0 }, (char[]){ 0, 0, 0 } }, 3 },
	//S shape     
	{ (char *[]){ (char[]){ 1, 1, 0 }, (char[]){ 0, 1, 1 }, (char[]){ 0, 0, 0 } }, 3 },
	//Z shape     
	{ (char *[]){ (char[]){ 0, 1, 0 }, (char[]){ 1, 1, 1 }, (char[]){ 0, 0, 0 } }, 3 },
	//T shape     
	{ (char *[]){ (char[]){ 0, 0, 1 }, (char[]){ 1, 1, 1 }, (char[]){ 0, 0, 0 } }, 3 },
	//L shape     
	{ (char *[]){ (char[]){ 1, 0, 0 }, (char[]){ 1, 1, 1 }, (char[]){ 0, 0, 0 } }, 3 },
	//flipped L shape    
	{ (char *[]){ (char[]){ 1, 1 }, (char[]){ 1, 1 } }, 2 },
	//square shape
{
	(char *[]) {
	(char[]){ 0, 0, 0, 0 },
	(char[]){ 1, 1, 1, 1 }, 
	(char[]){ 0, 0, 0, 0 },
	(char[]){ 0, 0, 0, 0 }
},
	4
} //long bar shape
// you can add any shape like it's done above. Don't be naughty.
};

Shape CopyShape(Shape shape) {
	Shape new_shape = shape;
	char **copyshape = shape.array;
	new_shape.array = (char**)malloc(new_shape.width*sizeof(char*));
	int i, j;
	for (i = 0; i < new_shape.width; i++) {
		new_shape.array[i] = (char*)malloc(new_shape.width*sizeof(char));
		for (j = 0; j < new_shape.width; j++) {
			new_shape.array[i][j] = copyshape[i][j];
		}
	}
	return new_shape;
}

void DeleteShape(Shape shape) {
	int i;
	for (i = 0; i < shape.width; i++) {
		free(shape.array[i]);
	}
	free(shape.array);
}

int CheckPosition(Shape shape) {
	//Check the position of the copied shape
	char **array = shape.array;
	int i, j;
	for (i = 0; i < shape.width; i++) {
		for (j = 0; j < shape.width; j++) {
			if ((shape.col + j < 0 || shape.col + j >= COLS || shape.row + i >= ROWS)) {
				//Out of borders
				if (array[i][j]) //but is it just a phantom?
				return FALSE;
				
			}
			else if (Table[shape.row + i][shape.col + j] && array[i][j])
				return FALSE;
		}
	}
	return TRUE;
}

void SetNewRandomShape() 
{
	//updates [current] with new shape
	Shape new_shape = CopyShape(ShapesArray[rand() % 7]);

	new_shape.col = rand() % (COLS - new_shape.width + 1);
	new_shape.row = 0;
	DeleteShape(current);
	current = new_shape;
	if (!CheckPosition(current)) {
		GameOn = FALSE;
	}
	
	

	
	
}

void RotateShape(Shape shape) {
	//rotates clockwise
	Shape temp = CopyShape(shape);
	int i, j, k, width;
	width = shape.width;
	for (i = 0; i < width; i++) {
		for (j = 0, k = width - 1; j < width; j++, k--) {
			shape.array[i][j] = temp.array[k][i];
		}
	}
	DeleteShape(temp);
}

void WriteToTable() {
	int i, j;
	for (i = 0; i < current.width; i++) {
		for (j = 0; j < current.width; j++) {
			if (current.array[i][j])
				Table[current.row + i][current.col + j] = current.array[i][j];
		}
	}
}

void RemoveFullRowsAndUpdateScore() {
	int i, j, sum, count = 0;
	for (i = 0; i < ROWS; i++) {
		sum = 0;
		for (j = 0; j < COLS; j++) {
			sum += Table[i][j];
		}
		if (sum == COLS) {
			count++;
			int l, k;
			for (k = i; k >= 1; k--)
				for (l = 0; l < COLS; l++)
					Table[k][l] = Table[k - 1][l];
			for (l = 0; l < COLS; l++)
				Table[k][l] = 0;
		}
	}
	score += 100*count;
}

void PrintTable() {
	char Buffer[ROWS][COLS] = { 0 };
	int i, j;
	char PrintBuf[32];
	
	for (i = 0; i < current.width; i++) {
		for (j = 0; j < current.width; j++) {
			if (current.array[i][j])
				Buffer[current.row + i][current.col + j] = current.array[i][j];
		}
	}
	//clear();

	EPD_clear(1); //buf init
//	sprintf(txbuf, "%02X  ", i);
	
	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLS; j++) {
			
			if (Table[i][j] + Buffer[i][j])
			{
				EPD_DrawFilledRectangle(6 + j * 11, 20 + i * 11, 9, 9, 1);	
			}
			else 
			{
				EPD_DrawRectangle(6 + j * 11, 20 + i * 11, 9, 9, 1);	
			}
			
		}
	}
	
	sprintf(PrintBuf, "Score: %d", score);
	EPD_Text(20, 245, PrintBuf, 1, 0);
	EPD_Text(0, 0, "EXIT", 1, 0);
	EPD_PartWriteFull(true);
	EPD_DeepSleepNoWait();

}

void ManipulateCurrent(int action) {
	Shape temp = CopyShape(current);
	switch (action) {
	case 's':
		temp.row++; //move down
		if (CheckPosition(temp))
			current.row++;
		else {
			if (BtnDown) PrintTable();
			EPD_HW_Init_Fast();
			EPD_WriteScreen_Buff_Fast();
			while (DISP_BUSY) ;	
			EPD_DeepSleep(); //EPD_DeepSlee
			
			WriteToTable();
			RemoveFullRowsAndUpdateScore();
			SetNewRandomShape();
			
			BtnRight = 0;
			BtnTurn = 0;
			BtnLeft = 0;
			BtnDown = 0;
			Delay(1000); //display needs
		}
		break;
	case 'd':
		temp.col++; //move right
		if (CheckPosition(temp))
			current.col++;
		else BtnRight = 0;
		break;
	case 'a':
		temp.col--; //move left
		if (CheckPosition(temp))
			current.col--;
		else BtnLeft = 0;
		break;
	case 'w':
		RotateShape(temp); // rotate clockwise
		if (CheckPosition(temp))
			RotateShape(current);
		break;
	}
	DeleteShape(temp);
	if (!BtnDown) PrintTable();
}


/************************/
/************************/
/************************/
/************************/
/************************/
/************************/

void SensTest()
{
	uint8_t ResCnt = 0;
	EPD_Text(0, 105, "  Hold 3 sec", 1, 0);
	EPD_Text(0, 125, "   to Reset", 1, 0);
	EPD_PartWriteFull(true);
	EPD_DeepSleep();
	
	Delay(500);
	
	EPD_HW_Init();
	EPD_clear(0); //buf init
	
	EPD_PartWriteFull(true);
	EPD_DeepSleep();
	
	EPD_clear(1); //buf init
	EPD_PartWriteFull(true);
	EPD_DeepSleep();	
	Touched = false;
	
	EPD_HW_Init();
	while (true)
	{
		
		if (Touched)
		{
			uPrintf(">%u\r\n", ResCnt);
			Touched = false;			
			
			
			EPD_DrawCircle(X_SIZE - TouchX - 1, Y_SIZE - TouchY - 1, 5, 1);
			EPD_PartWriteFull(true);
			
			ResCnt++;
			if (ResCnt > 7) NVIC_SystemReset();

			continue;
		}
		ResCnt = 0;
	
	}
	
}


/************************/
/************************/
/************************/
/************************/
/************************/
/************************/


void NfcTest()
{
	
	uint16_t AccC;
	uint16_t x;
	uint16_t pVal[3], pTol[3];
	
	uint16_t fPeak, fFix, fPd, tMin, tMax;
	uint8_t i;
	
	NfcInit();
//	EPD_SetRotate(0);
//
//	EPD_Text(0, 0, "Touch to reset", 1, 0);
//	uint8_t qrcodeData[qrcode_getBufferSize(5)];	
//	
//	sprintf(EpdPrint,
//		" ID: %08X%08X",
//		((uint32_t)((uniqueId >> 32) & 0xFFFFFFFF)),
//		((uint32_t)(uniqueId & 0xFFFFFFFF)));
//	
//	qrcode_initText(&qrcode, qrcodeData, 5, 1, EpdPrint);
//	
//	for (uint8_t y = 0; y < qrcode.size; y++) 
//	{
//		for (uint8_t x = 0; x < qrcode.size; x++) 
//		{
//			EPD_setPoint(x + 5, y + 30, qrcode_getModule(&qrcode, x, y));
//		}
//	}
	
	
//	EPD_PartWriteFull(true);
//	EPD_DeepSleep();
//	
//	EPD_SetRotate(270);
	memset(RomAdrMap, 0, 4096);
	
	
	Touched = false;
	AccC = 0;
	while (true)
	{
		

		if (!DISP_BUSY && DispState)
		{
			EPD_DeepSleepNoWait();
			DispState = false;
		}
		
		
		if (Touched)
		{
			NVIC_SystemReset();
		}
		
		NfcFifo.Tail = ((LDMA->CH[DmaChannelTim0].DST - (uint32_t)&NfcFifo.Buf[0]) >> 1) & 0x0FFF;

	
		if (NfcFifo.Tail != NfcFifo.Head) //new data came
		{
			NfcFifo.LastVal = NfcFifo.Buf[NfcFifo.Head++]; //get tim val
			NfcFifo.Head &= 0x0FFF;
			
			
			if (NfcFifo.LastVal < 512) 
			{
				if (RomAdrMap[NfcFifo.LastVal] < 100) RomAdrMap[NfcFifo.LastVal]++;
				AccC++;
				
				if (AccC > 512) 
				{
					x = 0;
					for (i = 0; i < 3; i++)
					{
						fPeak = 0;
						fPd = 0;
						while (x < 512)
						{
							if (RomAdrMap[x] > fPeak)
							{
								fPeak = RomAdrMap[x];
								fFix = x;
								fPd = 0;
							}
							else if (fPeak < 10) fPd = 0;
							
							fPd++;
							if (fPd > 10) break; //found peak
							x++;
						}
						pVal[i] = fFix;
					}					
					
					for (uint8_t j = 0; j < 3; j++)
					{
						x = pVal[j];
						for (i = 0; i < 30; i++)
						{
							if (RomAdrMap[x] == 0) 
							{
								tMin = x;
								break;
							}
							x--;
						}
						x = pVal[j];
						for (i = 0; i < 30; i++)
						{
							if (RomAdrMap[x] == 0) 
							{
								tMax = x;
								break;
							}
							x++;
						}
						pTol[j] = (tMax - tMin) >> 1;
					
					}
					
					sprintf(EpdPrint, "A: %u +/- %u  ", pVal[0], pTol[0]);
					EPD_Text(0, 144, EpdPrint, 1, 0);

					sprintf(EpdPrint, "B: %u +/- %u  ", pVal[1], pTol[1]);
					EPD_Text(0, 160, EpdPrint, 1, 0);
					
					sprintf(EpdPrint, "C: %u +/- %u  ", pVal[2], pTol[2]);
					EPD_Text(0, 176, EpdPrint, 1, 0);
					
					
					
//					
//					for (x = 0; x < 512; x++)
//					{
//						EPD_setPoint((x >> 1), 170 - RomAdrMap[x], 1);
//					}
					
					EPD_PartWriteFullNoWait(true);
					DispState = true;
					memset(RomAdrMap, 0, 4096);
					AccC = 0;
				}
				
			}
		
			
		}
	
	}
	
	
}

void FreqVoltTest()
{
	uint16_t x;
	uint16_t Volt = 0, VoltOld = 0;
	
	uint8_t frq = 0;
	uint8_t Gear = 1;
	

	uint8_t qrcodeData[qrcode_getBufferSize(5)];

	
	AdcInit();
	EPD_SetRotate(0);

	EPD_Text(0, 0, "Touch to reset", 1, 0);

	EPD_SetRotate(270);

	EPD_Text(15, 83, "2", 1, 0);
	EPD_Text(57, 83, "5", 1, 0);
	EPD_Text(91, 83, "10", 1, 0);
	EPD_Text(131, 83, "20", 1, 0);
	EPD_Text(174, 83, "39", 1, 0);
	EPD_Text(215, 83, "80", 1, 0);
	
	
	sprintf(EpdPrint,
		" ID: %08X%08X",
		((uint32_t)((uniqueId >> 32) & 0xFFFFFFFF)),
		((uint32_t)(uniqueId & 0xFFFFFFFF)));
	
	qrcode_initText(&qrcode, qrcodeData, 5, 1, EpdPrint);
	for (uint8_t y = 0; y < qrcode.size; y++) 
	{
		for (uint8_t x = 0; x < qrcode.size; x++) 
		{
			EPD_setPoint(x + 5, y + 135, qrcode_getModule(&qrcode, x, y));
		}
	}
	
	
	
	EPD_PartWriteFull(true);
	EPD_DeepSleep();
	
	Touched = false;
	for (x = 0; x < 247; x++) 
	{
		
		//if (x==)
		
		if (x == 238)
		{
			
			MCU_OK(0);
		}
		
		if (Touched)
		{
			NVIC_SystemReset();
		}
		
		
		if (!(x % 41))
		{
			FreqSet(frq++);	
			EPD_line(x, 77, x, 97, 1);
			Gear = 0;
		}
		
		Volt = 175 - (MainVoltage * 9.0f);

		IADC_command(IADC0, iadcCmdStartSingle);
		
		if (VoltOld) EPD_line(x, VoltOld, x, Volt, 1);
		
		if (VoltOld && !Gear) 
		{
			uPrintf("Voltage: %.2f\r\n", MainVoltage);
			EPD_PartWriteFull(true);
			EPD_DeepSleep();
			Gear = 3;
		}
		else
		{
			Delay(100);
			if (Gear) Gear--;
		}
		
		
		VoltOld = Volt;

		
	}
	
	Delay(100);
	
}



void Demo()
{
	
	uint8_t x, y, i;
	Touched = false;
	while (true)
	{


		if (Touched) NVIC_SystemReset();

	
		EPD_clear(1); //buf init

		EPD_HW_Init_Fast();
	
		srand(EfmGetRandom());

		EPD_putLogo(grob, 0, sizeof(grob));
		EPD_WriteScreen_Buff_Fast();
		while (DISP_BUSY) ;	
		EPD_DeepSleep();

		Delay(3000);
		if (Touched) NVIC_SystemReset();

	
		for (i = 0; i < 10; i++)
		{
			if (Touched) NVIC_SystemReset();

			for (uint16_t d = 0; d < 20000; d++)
			{
				if (!DISP_BUSY && DispState)
				{
					EPD_DeepSleepNoWait();
					DispState = false;
				}
				EPD_setPoint(rand() % X_SIZE, rand() % Y_SIZE, (rand() & 1));
			}
			EPD_PartWriteFullNoWait(true);
			DispState = true;
		}
	
		EPD_HW_Init_Fast();
		EPD_putLogo(TerPhil, 0, sizeof(TerPhil));
		EPD_WriteScreen_Buff_Fast();
		while (DISP_BUSY) ;	
		EPD_DeepSleep();
		Delay(3000);
	
		for (i = 0; i < 10; i++)
		{
			if (Touched) NVIC_SystemReset();

			for (uint16_t d = 0; d < 20000; d++)
			{
				if (!DISP_BUSY && DispState)
				{
					EPD_DeepSleepNoWait();
					DispState = false;
				}
				EPD_setPoint(rand() % X_SIZE, rand() % Y_SIZE, (rand() & 1));
			}
			EPD_PartWriteFullNoWait(true);
			DispState = true;
		}
		
		EPD_clear(1); //buf init
	
		for (uint16_t d = 0; d < 300; d++)
		{
			i = 2 + (rand() % 18);
			x = 20 + (rand() % (X_SIZE - 40));
			y = 20 + (rand() % (Y_SIZE - 40));
			EPD_DrawCircle(x, y, i, 1);
		}
	
		EPD_HW_Init_Fast();
		EPD_WriteScreen_Buff_Fast();
		while (DISP_BUSY) ;	
		EPD_DeepSleep();
		Delay(3000);
		if (Touched) NVIC_SystemReset();

		for (i = 0; i < 10; i++)
		{
			if (Touched) NVIC_SystemReset();

			for (uint16_t d = 0; d < 20000; d++)
			{
				if (!DISP_BUSY && DispState)
				{
					EPD_DeepSleepNoWait();
					DispState = false;
				}
				EPD_setPoint(rand() % X_SIZE, rand() % Y_SIZE, (rand() & 1));
			}
			EPD_PartWriteFullNoWait(true);
			DispState = true;
		}
		
		while (DISP_BUSY) ;	
		EPD_DeepSleep();
		
		EPD_HW_Init(); //Electronic paper initialization	
		EPD_WhiteScreen_White();
		EPD_DeepSleep();
		EPD_HW_Init(); //Electronic paper initialization	
		EPD_WhiteScreen_White();
		EPD_DeepSleep();
		EPD_HW_Init(); //Electronic paper initialization	
		EPD_WhiteScreen_White();
		EPD_DeepSleep();
		
	}
}