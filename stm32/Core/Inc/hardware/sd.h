/**
 ******************************************************************************
 * @file      sd.h
 * @author    ts-manuel
 * @brief     Driver for the SD-Card
 *
 ******************************************************************************
 */

#ifndef INC_HARDWARE_SD_H_
#define INC_HARDWARE_SD_H_

#include <stdint.h>
#include <stdbool.h>
#include "fatfs.h"
#include "stm32f4xx_hal.h"
#include "hardware/power.h"
#include "settings.h"


HAL_StatusTypeDef SD_Init(void);
HAL_StatusTypeDef SD_Sleep(void);

#endif /* INC_HARDWARE_SD_H_ */
