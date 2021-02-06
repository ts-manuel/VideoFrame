/**
 ******************************************************************************
 * @file      display_driver.h
 * @author    ts-manuel
 * @brief     Driver for the E-Paper display
 *
 ******************************************************************************
 */

#ifndef INC_DISPLAY_DRIVER_H_
#define INC_DISPLAY_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>
#include "EPD_5in65f.h"

void DISP_Init(void);
void DISP_Sleep(void);
void DISP_BeginUpdate(void);
void DISP_EndUpdate(void);
void DISP_Clear(uint8_t color);
void DISP_ShowBlocks(void);
void DISP_ShowStripes(void);
void DISP_ShowLines(void);

#endif /* INC_DISPLAY_DRIVER_H_ */
