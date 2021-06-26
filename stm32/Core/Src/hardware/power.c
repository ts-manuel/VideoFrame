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


static void update_checksum(State_t* data);
static bool checksum(const State_t* data);


/*
 * Read data from backup ram or flash
 * */
void PWR_ReadBackupData(State_t* data)
{
	taskENTER_CRITICAL();

	//Discriminate between reset/standby and power cycle
	if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
	{
		//Recovering from reset/standby
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
		__HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF);

		//Reload variables from backup ram
		__HAL_RCC_BKPSRAM_CLK_ENABLE();
		HAL_PWR_EnableBkUpAccess();
		HAL_PWREx_EnableBkUpReg();
		memcpy(data, (void*)BKPSRAM_BASE, sizeof(State_t));
		data->data_valid = checksum(data);
		data->power_cycle = false;
		__HAL_RCC_BKPSRAM_CLK_DISABLE();
	  }
	  else
	  {
		  //Recovering from power cycle

		  data->power_cycle = true;

		  //Reload data from flash
#warning "TODO: Load data from FLASH"
	  }



	taskEXIT_CRITICAL();
}


/*
 * Store data to backup ram or flash
 * */
void PWR_WriteBacupData(State_t* data, bool write_to_flash)
{
	taskENTER_CRITICAL();

	//Update checksum
	update_checksum(data);

	if(write_to_flash)
	{
		//Write data to flash
#warning "TODO: Write data to FLASH"
	}
	else
	{
		//Write data to backup ram
		__HAL_RCC_BKPSRAM_CLK_ENABLE();
		HAL_PWR_EnableBkUpAccess();
		HAL_PWREx_EnableBkUpReg();
		memcpy((void*)BKPSRAM_BASE, data, sizeof(State_t));
		__HAL_RCC_BKPSRAM_CLK_DISABLE();
	}

	taskEXIT_CRITICAL();
}


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


/*
 * Updates the data checksum
 * */
static void update_checksum(State_t* data)
{
	uint8_t sum = 0;
	data->checksum = 0;

	for(int i = 0; i < sizeof(State_t); i++)
		sum += ((uint8_t*)data)[i];

	data->checksum = ~sum+1;
}



/*
 * checks the checksum
 * */
static bool checksum(const State_t* data)
{
	uint8_t sum = 0;

	for(int i = 0; i < sizeof(State_t); i++)
		sum += ((uint8_t*)data)[i];

	return sum == 0;
}
