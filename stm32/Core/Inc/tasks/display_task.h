/**
 ******************************************************************************
 * @file      display_task.h
 * @author    ts-manuel
 * @brief     Updates the display
 *
 ******************************************************************************
 */

#ifndef INC_TASKS_DISPLAY_TASK_H_
#define INC_TASKS_DISPLAY_TASK_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "hardware/display.h"
#include "jpeg/decoder.h"
#include "fatfs.h"

#define _FLAG_DISPLAY_UPDATE 1

typedef struct
{
	osMessageQueueId_t message_queue;
} DisplayTaskArgs_t;

typedef enum
{
	e_DisplaySolid,
	e_DisplayBlocks,
	e_DisplayStripes,
	e_DisplayLines,
	e_DisplayGradient,
	e_DisplayJPEG,
	e_DisplayBMP
} DisplayAction_e;

typedef struct
{
	DisplayAction_e action;	//Action to be performed
	uint8_t color;			//Color to be displayed
	FIL* fp;				//Jpeg file to be displayed
	uint8_t* bmp;			//600x448 bitmap
} DisplayMessage_t;

void StartDisplayTask(void *_args);


#endif /* INC_TASKS_DISPLAY_TASK_H_ */
