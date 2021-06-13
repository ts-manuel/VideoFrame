/**
 ******************************************************************************
 * @file      console_task.h
 * @author    ts-manuel
 * @brief     Handles commands received from the serial port
 *
 ******************************************************************************
 */

#ifndef INC_TASKS_CONSOLE_TASK_H_
#define INC_TASKS_CONSOLE_TASK_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "settings.h"
#include "hardware/power.h"
#include "cmsis_os.h"


typedef struct
{
	UART_HandleTypeDef* huart;
	State_t* state;
} ConsoleTaskArgs_t;


void StartConsoleTask(void *args);


#endif /* INC_TASKS_CONSOLE_TASK_H_ */
