/**
 ******************************************************************************
 * @file      power.h
 * @author    ts-manuel
 * @brief     Power management and backup
 *
 ******************************************************************************
 */

#ifndef INC_HARDWARE_POWER_H_
#define INC_HARDWARE_POWER_H_

#include <main.h>
#include <fatfs.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>


typedef struct
{
	volatile bool power_cycle;		//Set when the micro is first powered up
	volatile bool data_valid;		//Set if there is valid data in the backup ram or flash
	volatile float battery_voltage;	//Measured battery voltage

#warning "Consolidate file name and folder into one file path"
	volatile char file_name[_MAX_LFN+1];
	volatile char folder_name[_MAX_LFN+1];

	volatile uint8_t checksum;
} State_t;

typedef enum
{
	PWR_3V3,
	PWR_SD
} Device_e;


void PWR_ReadBackupData(State_t* data);

void PWR_WriteBacupData(State_t* data, bool write_to_flash);

void PWR_Enable(Device_e dev);

void PWR_Disable(Device_e dev);

void PWR_EnterStandBy(void);

#endif /* INC_HARDWARE_POWER_H_ */
