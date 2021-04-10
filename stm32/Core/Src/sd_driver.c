/**
 ******************************************************************************
 * @file      sd_driver.c
 * @author    ts-manuel
 * @brief     Driver for the SD-Card
 *
 ******************************************************************************
 */

#include "sd_driver.h"

#define _MAX_INIT_RETRY		100


/*
 * Initialize SD-Card
 * */
HAL_StatusTypeDef SD_Init(SD_HandleTypeDef* hsd, FATFS* fs)
{
	//Enable power
	PWR_Enable(PWR_SD);

	//Initialize SD card
	HAL_StatusTypeDef init_res;
	FRESULT mnt_res;
	int i = 0;

	do
	{
		//Initialize SD
		init_res = HAL_SD_Init(hsd);

		//Mount drive
		if(init_res == HAL_OK)
			mnt_res = f_mount(fs, "", 1);
		else
			HAL_Delay(1);

	} while(++i < _MAX_INIT_RETRY && ((init_res != HAL_OK) || (mnt_res != FR_OK)));

	if(i == _MAX_INIT_RETRY)
		return HAL_ERROR;

	//Configure card to 4bit mode
	return HAL_SD_ConfigWideBusOperation(hsd, SDIO_BUS_WIDE_4B);
}


/*
 * Remove power to the SD-Card
 * */
HAL_StatusTypeDef SD_Sleep(SD_HandleTypeDef* hsd)
{
	//Unmount drive
	if(f_mount(0, "", 0) != FR_OK)
		return HAL_ERROR;

	//Power off SD
	if(HAL_SD_DeInit(hsd) != HAL_OK)
		return HAL_ERROR;

	//Remove power
	PWR_Disable(PWR_SD);

	return HAL_OK;
}
