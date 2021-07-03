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
#include "file_manager.h"
#include "hardware/power.h"
#include "hardware/sd.h"
#include "tasks/display_task.h"
#include "cmsis_os.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "EPD_5in65f.h"


typedef struct
{
	UART_HandleTypeDef* huart;
	State_t* state;
	osThreadId_t displayTaskId;
	osMessageQueueId_t display_message_queue;
	volatile bool* sleep_cmd_disabled;
} ConsoleTaskArgs_t;


bool InitCMDBuffer(const char* str);
void StartConsoleTask(void *args);


#endif /* INC_TASKS_CONSOLE_TASK_H_ */
