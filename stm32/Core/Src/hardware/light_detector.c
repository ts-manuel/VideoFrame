/**
 ******************************************************************************
 * @file      light_detector.c
 * @author    ts-manuel
 * @brief     Light sensor with light dependent resistor.
 *
 *             o VDD
 *             |
 *             <
 *             > LDR
 *             <
 *             |
 *             o---WWW---o LDR_SIG
 *             |
 *            ===
 *             |
 *             o LDR_GND
 *
 *            The light intensity is determined by timing the rise time of the
 *            RC circuit. The capacitor is discharged by grounding LDR_SIG than
 *            it charges through the LDR. The time it takes the capacitor to
 *            charge from 0V to VDD/2 is measured.
 *
 *            20 mS  -> Complete darkness
 *            200 us -> Ambient light
 *
 ******************************************************************************
 */

#include "hardware/light_detector.h"
#include "main.h"


bool LDR_IsDark(void)
{
	//Discharge the capacitor
	HAL_GPIO_WritePin(LDR_SIG_GPIO_Port, LDR_SIG_Pin, GPIO_PIN_RESET);
	HAL_Delay(1);

	//Release the LDR_SIG pin
	HAL_GPIO_WritePin(LDR_SIG_GPIO_Port, LDR_SIG_Pin, GPIO_PIN_SET);
	uint32_t start = HAL_GetTick();
	uint32_t time = 0;

	//Wait until the LDR_SIG rises above the threshold level
	while(HAL_GPIO_ReadPin(LDR_SIG_GPIO_Port, LDR_SIG_Pin) == GPIO_PIN_RESET && time < 255)
	{
		time = HAL_GetTick() - start;
	}

	return time >= _DARK_THRESHOLD;
}
