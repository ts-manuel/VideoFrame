/**
 ******************************************************************************
 * @file      sd.c
 * @author    ts-manuel
 * @brief     Driver for the SD-Card
 *
 ******************************************************************************
 */

#include <hardware/sd.h>


extern SD_HandleTypeDef hsd;
extern FATFS fs;


/*
 * Initialize SD-Card
 * */
HAL_StatusTypeDef SD_Init(void)
{
	FRESULT fs_res;

	//Enable power
	PWR_Enable(PWR_SD);
	HAL_Delay(100);

	//Initialize SD and Mount drive
	fs_res = f_mount(&fs, "", 1);
	if(fs_res != FR_OK)
	{
		return HAL_ERROR;
	}

	return HAL_OK;
}


/*
 * Remove power to the SD-Card
 * */
HAL_StatusTypeDef SD_Sleep(void)
{
	//Unmount drive
	if(f_mount(0, "", 0) != FR_OK)
		return HAL_ERROR;

	//Power off SD
	if(HAL_SD_DeInit(&hsd) != HAL_OK)
		return HAL_ERROR;

	//Remove power
	PWR_Disable(PWR_SD);

	return HAL_OK;
}
