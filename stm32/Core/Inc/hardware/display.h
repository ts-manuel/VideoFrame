/**
 ******************************************************************************
 * @file      display.h
 * @author    ts-manuel
 * @brief     Driver for the E-Paper display
 *
 ******************************************************************************
 */

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_

#include <stdint.h>
#include <stdbool.h>
#include "EPD_5in65f.h"

#define _DITHER 1			//0 = no dither, 1 = Floyd–Steinberg
#define _DISPLAY_WIDTH	EPD_5IN65F_WIDTH
#define _DISPLAY_HEIGHT	EPD_5IN65F_HEIGHT

#define _NUM_COLORS 7
typedef struct{
	int16_t r;
	int16_t g;
	int16_t b;
} RGB16_t;
extern const RGB16_t display_colors[_NUM_COLORS+1];


void DISP_Init(void);
void DISP_Sleep(void);
void DISP_BeginUpdate(void);
void DISP_EndUpdate(void);
void DISP_SendData(uint8_t data);
void DISP_SetStripeHeight(int h);
void DISP_WritePixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);

#endif /* INC_DISPLAY_H_ */

