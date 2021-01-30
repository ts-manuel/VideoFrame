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

#include "stm32f4xx_hal.h"

#define _MAX_CMD_LENGTH 64	//Maximum number of characters in a command

void CMD_Init(UART_HandleTypeDef* huart);
void CMD_TryParse(void);

#endif /* INC_CMD_PARSER_H_ */
