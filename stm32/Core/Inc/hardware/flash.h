/**
 ******************************************************************************
 * @file      flash.h
 * @author    ts-manuel
 * @brief     Flash
 *
 ******************************************************************************
 */

#ifndef INC_HARDWARE_FLASH_H_
#define INC_HARDWARE_FLASH_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <main.h>

#include "settings.h"


bool FLASH_LoadFilePath(char* file_path);

void FLASH_StoreFilePath(const char* file_path);

void FLASH_Erase(void);

#endif /* INC_HARDWARE_FLASH_H_ */
