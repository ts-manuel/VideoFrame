/**
 ******************************************************************************
 * @file      power.h
 * @author    ts-manuel
 * @brief     Power management functions
 *
 ******************************************************************************
 */

#ifndef INC_POWER_H_
#define INC_POWER_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "main.h"

typedef enum
{
	PWR_3V3,
	PWR_SD
} PWR_Device_t;

void PWR_Enable(PWR_Device_t dv);
void PWR_Disable(PWR_Device_t dv);

#endif /* INC_POWER_H_ */
