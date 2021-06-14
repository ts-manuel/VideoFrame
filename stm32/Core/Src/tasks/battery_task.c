/**
 ******************************************************************************
 * @file      battery_task.c
 * @author    ts-manuel
 * @brief     Battery Task
 * 				reads the battery voltage every second
 *
 ******************************************************************************
 */

#include "tasks/battery_task.h"


extern ADC_HandleTypeDef hadc1;


static float read_voltage(void);


/*
 * Battery Task
 *
 * Reads battery voltage every second
 * if the voltage is lower then _MINIMUM_VOLTAGE
 * the state is saved to FLASH and sleep mode
 * */
void StartBatteryTask(void *arg)
{
	State_t* data = (State_t*)arg;

	while(1)
	{
		//Read battery voltage
		data->battery_voltage = read_voltage();

		if(data->battery_voltage < _MINIMUM_VOLTAGE)
		{
			PWR_Disable(PWR_3V3);

			//Write state to FLASH
			PWR_WriteBacupData(data, true);

			printf("LOW BATTERY! Entering low power mode\n");

			//Enter sleep mode
			PWR_EnterStandBy();
		}

		//Delay 1 second
		osDelay(1000);
	}
}


/*
 * Read battery voltage
 * */
static float read_voltage(void)
{
	//Start conversion
	HAL_ADC_Start(&hadc1);

	//Wait for result
	HAL_ADC_PollForConversion(&hadc1, 100);

	//Return result
	return (float)HAL_ADC_GetValue(&hadc1) * ((3.3f*4.f)/256.f);
}
