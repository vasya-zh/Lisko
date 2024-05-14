#pragma once
#include <stdbool.h>
#include <stdio.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_usart.h"
#include "em_eusart.h"
#include "utils.h"
#include "em_ldma.h"

#include "dmadrv.h" 
#include <em_timer.h>
#include <em_gpcrc.h>


#define NFC_PORT gpioPortA
#define NFC_PIN 8
	
#define DMA_FIFO_SIZE 4096 //4096 is 2x 2048 - max for EFM32 DMA
#define NFC_DECODED_MAX 512 

//Pulse Width Constants
#define AMIN	113
#define AMAX	143
	
#define BMIN	203
#define BMAX 	243
	
#define CMIN 	291
#define CMAX	331

#define MMIN    1500
#define MMAX    3000

#define MLMAX   210
#define MLMIN   130

#define MSMAX   129
#define MSMIN   65


//#define AMIN	160
//#define AMAX	200
//	
//#define BMIN	250
//#define BMAX 	290
//	
//#define CMIN 	340
//#define CMAX	380

typedef enum NFCState_t
{
	Ready,
	Started,
	Received,
	Manchester
		
} NFCStates;

typedef enum State_t
{
	State_X,
	State_Y,
	State_Z
} RxStates;

struct NfcFifo_t
{
	uint16_t Buf[DMA_FIFO_SIZE];  
	uint16_t Tail;
	uint16_t Head;
	uint16_t Size;
	uint16_t LastVal;
	bool TimeOut;
	NFCStates State;
};


#define LDMA_DESCRIPTOR_LINKREL_P2M_HALF( src, dest, count, linkjmp ) \
{                                                                     \
  .xfer =                                                             \
  {                                                                   \
    .structType   = ldmaCtrlStructTypeXfer,                           \
    .structReq    = 0,                                                \
    .xferCnt      = ( count ) - 1,                                    \
    .byteSwap     = 0,                                                \
    .blockSize    = ldmaCtrlBlockSizeUnit1,                           \
    .doneIfs      = 1,                                                \
    .reqMode      = ldmaCtrlReqModeBlock,                             \
    .decLoopCnt   = 0,                                                \
    .ignoreSrec   = 0,                                                \
    .srcInc       = ldmaCtrlSrcIncNone,                               \
    .size         = ldmaCtrlSizeHalf,                                 \
    .dstInc       = ldmaCtrlDstIncOne,                                \
    .srcAddrMode  = ldmaCtrlSrcAddrModeAbs,                           \
    .dstAddrMode  = ldmaCtrlDstAddrModeAbs,                           \
    .srcAddr      = (uint32_t)(src),                                  \
    .dstAddr      = (uint32_t)(dest),                                 \
    .linkMode     = ldmaLinkModeRel,                                  \
    .link         = 1,                                                \
    .linkAddr     = ( linkjmp ) * 4                                   \
  }                                                                   \
} 



	


void NfcInit();
bool GetBit(bool bit);
bool NfcDecode();
size_t NfcWork();
void NfcDecodeReset();























