/**
 ******************************************************************************
 * @file      decoder.h
 * @author    ts-manuel
 * @brief     JPEG decoder
 *
 ******************************************************************************
 */

#ifndef INC_DECODER_H_
#define INC_DECODER_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stddef.h>
#include "bit_buffer.h"
#include "fatfs.h"
#include "display_driver.h"

#define _DEBUG_PRINT	0	//0 = no debug output, 1 = print only header, 2 = print header and tables
#define _GAMMA_CORRECT	0	//0 = no gamma correction, 1 = gamma correct decoded image

//JPEG Markers
#define _SOI	0xd8	//(Start Of Image) must be the first marker of the file
#define _APP0	0xe0	//(Application specific) followed by 2byte size of payload
#define _APP1	0xe1
#define _APP2	0xe2
#define _APP3	0xe3
#define _APP4	0xe4
#define _APP5	0xe5
#define _APP6	0xe6
#define _APP7	0xe7
#define _APP8	0xe8
#define _APP9	0xe9
#define _APP10	0xea
#define _APP11	0xeb
#define _APP12	0xec
#define _APP13	0xed
#define _APP14	0xee
#define _APP15	0xef
#define _DQT	0xdb	//(Define Quantization Tables)
#define _SOF0	0xc0	//(Start of Frame 0) Baseline DCT, only one huffman coded bitstream, most used
#define _SOF1	0xc1	//(Start of Frame 1) Extended sequential DCT
#define _SOF2	0xc2	//(Start of Frame 2) Progressive DCT, many huffman coded bitstream, second most used
#define _SOF3	0xc3	//(Start of Frame 3) Lossless (sequential)
#define _SOF5	0xc5	//(Start of Frame 5) Differential sequential DCT
#define _SOF6	0xc6	//(Start of Frame 6) Differential progressive DCT
#define _SOF7	0xc7	//(Start of Frame 7) Differential lossless DCT
#define _SOF9	0xc9	//(Start of Frame 9) Extended sequential DCT
#define _SOF10	0xca	//(Start of Frame 10) Progressive DCT
#define _SOF11	0xcb	//(Start of Frame 11) Lossless (sequential)
#define _SOF13	0xcd	//(Start of Frame 13) Differential sequential DCT
#define _SOF14	0xce	//(Start of Frame 14) Differential progressive DCT
#define _SOF15	0xcf	//(Start of Frame 15) Differential lossless (sequential)
#define _DRI	0xdd	//(Define Restart Interval)
#define _DHT	0xc4	//(Define Huffman Tables)
#define _SOS	0xda	//(Start Of Scan)
#define _EOI	0xd9	//(End Of Image) must be the last byte of the file
#define _RST0	0xd0 	//(Restart markers)
#define _RST1	0xd1
#define _RST2	0xd2
#define _RST3	0xd3
#define _RST4	0xd4
#define _RST5	0xd5
#define _RST6	0xd6
#define _RST7	0xd7
#define _JPG	0xc8 	//(JPEG extensions)
#define _DAC	0xcc 	//(Define Arithmetic Coding)
#define _DNL	0xdc	//(Define Number of Lines)
#define _DHP	0xde	//(Define Hierarchical Progression)
#define _EXP	0xdf	//(Expand Reference Components)
#define _JPG0	0xf0 	//(Reserved for future versions)
#define _JPG1	0xf1
#define _JPG2	0xf2
#define _JPG3	0xf3
#define _JPG4	0xf4
#define _JPG5	0xf5
#define _JPG6	0xf6
#define _JPG7	0xf7
#define _JPG8	0xf8
#define _JPG9	0xf9
#define _JPG10	0xfa
#define _JPG11	0xfb
#define _JPG12	0xfc
#define _JPG13	0xfd
#define _COM	0xfe	//(Comment)
#define _TEM	0x01	//() no length marker

typedef struct {
	int Y[64*4];
	int Cb[64];
	int Cr[64];
	uint16_t x;
	uint16_t y;
} MCU_t;

#define MCU_COMP(mcu, i) ((i) == 0 ? mcu.Y : ((i) == 1 ? mcu.Cb : mcu.Cr))

typedef struct {
	uint16_t table[64];
	bool used;
} QTable_t;

typedef struct {
	uint8_t symbol;
	uint8_t length;
} HCode_t;

typedef struct {
	HCode_t shortLUT[1024];
	HCode_t longLUT[1024];
	bool used;
} HTable_t;

typedef struct {
	QTable_t* qTable;
	HTable_t* hTableDC;
	HTable_t* hTableAC;
	bool used;
} RGB16_t;

typedef struct {
	MCU_t mcu;
	uint8_t compNum;		//Zero based current color component
	uint16_t indx;			//Index of the next coefficient to write
	int16_t previousDc[3];	//Previous DC coefficient
	uint32_t blockCounter;
} Decode_t;

typedef struct {
	QTable_t QTables[4];
	HTable_t HTablesDC[4];
	HTable_t HTablesAC[4];
	RGB16_t colorComp[3];

	uint16_t heigth;
	uint16_t width;
	uint16_t numMCUs;
	uint8_t numComp;
	uint16_t restartInterval;
	uint8_t horizontalSamplingFactor;
	uint8_t verticalSamplingFactor;
	bool valid;
	Decode_t decode;
} JPG_t;


bool JPG_decode(FIL* fp, JPG_t* jpg);

#endif /* INC_DECODER_H_ */
