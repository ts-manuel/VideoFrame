/**
 ******************************************************************************
 * @file      light_detector.h
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
 *            The light intensity is determined by timing the time constant of
 *            the RC circuit.
 *
 ******************************************************************************
 */

#ifndef INC_HARDWARE_LIGHT_DETECTOR_H_
#define INC_HARDWARE_LIGHT_DETECTOR_H_

#include <stdio.h>
#include <stdbool.h>

#define _DARK_THRESHOLD	10

bool LDR_IsDark(void);

#endif /* INC_HARDWARE_LIGHT_DETECTOR_H_ */
