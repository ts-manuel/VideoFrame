/**
 ******************************************************************************
 * @file      battery_task.h
 * @author    ts-manuel
 * @brief     Battery Task
 * 				reads the battery voltage every second
 *
 ******************************************************************************
 */

#ifndef INC_TASKS_BATTERY_TASK_H_
#define INC_TASKS_BATTERY_TASK_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "settings.h"
#include "hardware/power.h"
#include "cmsis_os.h"

void StartBatteryTask(void *arg);


#endif /* INC_TASKS_BATTERY_TASK_H_ */
