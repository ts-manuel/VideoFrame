/**
 ******************************************************************************
 * @file      cmd_parser.h
 * @author    ts-manuel
 * @brief     Handles commands received by the serial port
 *
 ******************************************************************************
 */


#ifndef INC_CMD_PARSER_H_
#define INC_CMD_PARSER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include "stm32f4xx_hal.h"
#include "display_driver.h"
#include "sd_driver.h"

#define _MAX_CMD_LENGTH 64	//Maximum number of characters in a command

void CMD_Init(UART_HandleTypeDef* huart);
void CMD_TryParse(void);

#endif /* INC_CMD_PARSER_H_ */
