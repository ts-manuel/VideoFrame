/**
 ******************************************************************************
 * @file      power.h
 * @author    ts-manuel
 * @brief     Power management and backup
 *
 ******************************************************************************
 */

#ifndef INC_HARDWARE_POWER_H_
#define INC_HARDWARE_POWER_H_

#include <main.h>
#include <fatfs.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "cmsis_os.h"


typedef enum
{
	PWR_3V3,
	PWR_SD
} Device_e;


void PWR_Enable(Device_e dev);

void PWR_Disable(Device_e dev);

void PWR_EnterStandBy(void);

#endif /* INC_HARDWARE_POWER_H_ */
