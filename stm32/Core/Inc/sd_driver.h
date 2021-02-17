/**
 ******************************************************************************
 * @file      sd_driver.h
 * @author    ts-manuel
 * @brief     Driver for the SD-Card
 *
 ******************************************************************************
 */

#ifndef INC_SD_DRIVER_H_
#define INC_SD_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>
#include "fatfs.h"
#include "stm32f4xx_hal.h"

#define _MAX_MOUNT_RETRY 10

bool SD_Init(void);
bool SD_Sleep(void);

#endif /* INC_SD_DRIVER_H_ */
