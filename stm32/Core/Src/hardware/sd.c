/**
 ******************************************************************************
 * @file      sd.c
 * @author    ts-manuel
 * @brief     Driver for the SD-Card
 *
 ******************************************************************************
 */

#include <hardware/sd.h>

uint8_t buffer[512];

/*
 * Initialize SD-Card
 * */
HAL_StatusTypeDef SD_Init(SD_HandleTypeDef* hsd, FATFS* fs)
{
	HAL_StatusTypeDef sd_res;
	FRESULT fs_res;

	//Enable power
	PWR_Enable(PWR_SD);
	HAL_Delay(100);

/*	//Initialize SD card
	sd_res = HAL_SD_Init(hsd);
	if(sd_res != HAL_OK)
		return sd_res;

	//Configure card to 4bit mode
	sd_res = HAL_SD_ConfigWideBusOperation(hsd, SDIO_BUS_WIDE_4B);
	if(sd_res != HAL_OK)
		return sd_res;*/

	//SD_SetDeviceMode(SD_POLLING_MODE);

	//Mount drive
	fs_res = f_mount(fs, "", 1);
	if(fs_res != FR_OK)
	{
		printf("f_mount error, returned %d\n", fs_res);
		return HAL_ERROR;
	}

	return HAL_OK;



#if 0
	//Enable power
	PWR_Enable(PWR_SD);
	HAL_Delay(100);
	PWR_Disable(PWR_SD);
	HAL_Delay(100);
	PWR_Enable(PWR_SD);
	HAL_Delay(100);


	//Initialize SD card
	HAL_StatusTypeDef init_res;
	FRESULT mnt_res;
	int i = 0;

	do
	{
		//printf("TRY0: %d\n", i);

		//Initialize SD
		init_res = HAL_SD_Init(hsd);

		//printf("TRY1: %d\n", i);

		//Mount drive
		/*if(init_res == HAL_OK)
		{
			printf("TRY2: %d\n", i);
			mnt_res = f_mount(fs, "", 1);
		}
		else
		{
			printf("TRY3: %d\n", i);
			HAL_Delay(1);
		}*/
		//mnt_res = FR_OK;

		//printf("TRY4: %d\n", i);

	} while(++i < _MAX_INIT_RETRY && ((init_res != HAL_OK) || (mnt_res != FR_OK)));

	if(i == _MAX_INIT_RETRY)
		return HAL_ERROR;

	//printf("Card state: %d\n", (int)HAL_SD_GetCardState(hsd));



	//uint8_t buffer[512];
	HAL_SD_ReadBlocks(hsd, buffer, 0, 1, 100);
	printf("Block 0x00\n");
	for(int i = 0; i < 512; i+=16)
	{
		printf("%03X: ", i);

		for(int j = 0; j < 16; j++)
		{
			printf("%02X ", buffer[i+j]);
		}
		printf("\n");
	}

	printf("-------------------------------------------\n");
	HAL_SD_ConfigWideBusOperation(hsd, SDIO_BUS_WIDE_4B);

	HAL_SD_ReadBlocks(hsd, buffer, 0xfd, 1, 100);
	printf("Block 0xfd\n");
	for(int i = 0; i < 512; i+=16)
	{
		printf("%03X: ", i);

		for(int j = 0; j < 16; j++)
		{
			printf("%02X ", buffer[i+j]);
		}
		printf("\n");
	}

	printf("Card state: %d\n", (int)HAL_SD_GetCardState(hsd));


	//Configure card to 4bit mode
	return HAL_SD_ConfigWideBusOperation(hsd, SDIO_BUS_WIDE_4B);
#endif
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
