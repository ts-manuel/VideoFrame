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


bool FMAN_FindNext(char* new_path, const char* old_path);

#endif /* INC_FILE_MANAGER_H_ */
