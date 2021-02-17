/**
 ******************************************************************************
 * @file      sd_driver.c
 * @author    ts-manuel
 * @brief     Driver for the SD-Card
 *
 ******************************************************************************
 */

#include "sd_driver.h"

extern SD_HandleTypeDef hsd;
extern FATFS fs;


/*
 * Initialize SD-Card
 * */
bool SD_Init(void)
{
	int i = 0;

	//Enable power

	//Wait for power to stabilize
	HAL_Delay(1);

	//Initialize SD card
	if(HAL_SD_Init(&hsd) != HAL_OK)
	{
		return false;
	}

	//Mount SD card
	while(f_mount(&fs, "", 1) != FR_OK && i < _MAX_MOUNT_RETRY)
	{
		i++;
	}

	if(i == _MAX_MOUNT_RETRY)
		return false;

	return true;
}

/*
 * Remove power to the SD-Card
 * */
bool SD_Sleep(void)
{
	bool res;

	//Unmount drive
	res = f_mount(0, "", 0) == FR_OK;

	//Remove power

	return res;
}
