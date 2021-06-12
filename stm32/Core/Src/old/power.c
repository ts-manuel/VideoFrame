/**
 ******************************************************************************
 * @file      power.c
 * @author    ts-manuel
 * @brief     Power management functions
 *
 ******************************************************************************
 */

#include "power.h"

static bool disable_3v3 = false;


/*
 * Enable power to the device
 * */
void PWR_Enable(PWR_Device_t dv)
{
	switch(dv)
	{
		case PWR_3V3:
			if(HAL_GPIO_ReadPin(PWR_3V3_EN_GPIO_Port, PWR_3V3_EN_Pin) == GPIO_PIN_RESET)
			{
				HAL_GPIO_WritePin(PWR_3V3_EN_GPIO_Port, PWR_3V3_EN_Pin, GPIO_PIN_SET);
				HAL_Delay(1);
			}
			disable_3v3 = false;
			break;

		case PWR_SD:
			PWR_Enable(PWR_3V3);

			if(HAL_GPIO_ReadPin(PWR_SD_EN_GPIO_Port, PWR_SD_EN_Pin) == GPIO_PIN_RESET)
			{
				HAL_GPIO_WritePin(PWR_SD_EN_GPIO_Port, PWR_SD_EN_Pin, GPIO_PIN_SET);
				HAL_Delay(1);
			}
			break;
	}
}


/*
 * Disable power to the device
 * */
void PWR_Disable(PWR_Device_t dv)
{
	switch(dv)
	{
		case PWR_3V3:
			if(HAL_GPIO_ReadPin(PWR_SD_EN_GPIO_Port, PWR_SD_EN_Pin) == GPIO_PIN_SET)
			{
				disable_3v3 = true;
			}
			else
			{
				HAL_GPIO_WritePin(PWR_3V3_EN_GPIO_Port, PWR_SD_EN_Pin, GPIO_PIN_RESET);
			}
			break;

		case PWR_SD:
			HAL_GPIO_WritePin(PWR_SD_EN_GPIO_Port, PWR_SD_EN_Pin, GPIO_PIN_RESET);
			if(disable_3v3)
			{
				disable_3v3 = false;
				HAL_GPIO_WritePin(PWR_3V3_EN_GPIO_Port, PWR_SD_EN_Pin, GPIO_PIN_RESET);
			}
			break;
	}
}
