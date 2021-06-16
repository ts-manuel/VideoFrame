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


HAL_StatusTypeDef SD_Init(SD_HandleTypeDef* hsd, FATFS* fs);
HAL_StatusTypeDef SD_Sleep(SD_HandleTypeDef* hsd);

#endif /* INC_HARDWARE_SD_H_ */
