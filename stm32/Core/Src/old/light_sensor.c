/**
 ******************************************************************************
 * @file      light_sensor.c
 * @author    ts-manuel
 * @brief     Light sensor
 *
 ******************************************************************************
 */

#include "light_sensor.h"

#define _MAX_TIME_MS 10

/*
 * Uses the LDR resistor to measure light and returns a value between 0 and 100
 * 0 = complete darkness, 10 = bright as hell
 * */
int LRD_MeasureLight(void)
{
	uint32_t tickstart;
	uint32_t time;
	GPIO_InitTypeDef gpio_Init;

	//Configure LDR pins as output HIGH to discharge capacitor
	gpio_Init.Pin = LDR_SIG_Pin;
	gpio_Init.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_Init.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(LDR_SIG_GPIO_Port, &gpio_Init);
	HAL_GPIO_WritePin(LDR_SIG_GPIO_Port, LDR_SIG_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LDR_GND_GPIO_Port, LDR_GND_Pin, GPIO_PIN_SET);

	HAL_Delay(1);

	//Configure LDR_SIG pin as input
	gpio_Init.Pin = LDR_SIG_Pin;
	gpio_Init.Mode = GPIO_MODE_INPUT;
	gpio_Init.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(LDR_SIG_GPIO_Port, &gpio_Init);

	//Drive LDR_GND LOW and time the rising edge on LRD_SIG
	tickstart = HAL_GetTick();
	HAL_GPIO_WritePin(LDR_GND_GPIO_Port, LDR_GND_Pin, GPIO_PIN_RESET);

	do
	{
		time = HAL_GetTick() - tickstart;
	}while(HAL_GPIO_ReadPin(LDR_SIG_GPIO_Port, LDR_SIG_Pin) == GPIO_PIN_RESET && time < _MAX_TIME_MS);


	HAL_GPIO_WritePin(LDR_GND_GPIO_Port, LDR_GND_Pin, GPIO_PIN_SET);

	//Clamp value
	return 10 - time;
}
