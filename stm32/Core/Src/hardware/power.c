/**
 ******************************************************************************
 * @file      power.c
 * @author    ts-manuel
 * @brief     Power management and backup
 *
 ******************************************************************************
 */

#include "hardware/power.h"


extern RTC_HandleTypeDef hrtc;


/*
 * Enable power to the device
 * */
void PWR_Enable(Device_e dev)
{
	switch(dev)
	{
		case PWR_3V3:
			HAL_GPIO_WritePin(PWR_3V3_EN_GPIO_Port, PWR_3V3_EN_Pin, GPIO_PIN_SET);
			break;

		case PWR_SD:
			HAL_GPIO_WritePin(PWR_SD_EN_GPIO_Port, PWR_SD_EN_Pin, GPIO_PIN_SET);
			break;
	}
}


/*
 * Disable power to the device
 * */
void PWR_Disable(Device_e dev)
{
	switch(dev)
	{
		case PWR_3V3:
			HAL_GPIO_WritePin(PWR_3V3_EN_GPIO_Port, PWR_3V3_EN_Pin, GPIO_PIN_RESET);
			break;

		case PWR_SD:
			HAL_GPIO_WritePin(PWR_SD_EN_GPIO_Port, PWR_SD_EN_Pin, GPIO_PIN_RESET);
			break;
	}
}


/*
 * Enter standby mode
 * */
void PWR_EnterStandBy(void)
{
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
	__HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF);
	HAL_PWR_EnterSTANDBYMode();
}

