/**
 ******************************************************************************
 * @file      file_manager.h
 * @author    ts-manuel
 * @brief     File Manager
 *
 ******************************************************************************
 */

#ifndef INC_FILE_MANAGER_H_
#define INC_FILE_MANAGER_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "fatfs.h"
#include "sd_driver.h"
#include "display_driver.h"
#include "decoder.h"

bool FMAN_CheckFile(const char* folder, const char* file);
bool FMAN_FindNext(char* folder, char* file);

#endif /* INC_FILE_MANAGER_H_ */
