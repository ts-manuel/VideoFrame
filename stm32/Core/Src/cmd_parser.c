/**
 ******************************************************************************
 * @file      cmd_parser.c
 * @author    ts-manuel
 * @brief     Handles commands received by the serial port
 *
 ******************************************************************************
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include "cmd_parser.h"

//Extern Variables
extern volatile bool update;


//Function prototypes
static void CMD_ParseString(const char* str);
static void CMD_ParseInvalid(const char* str);
static void CMD_ParseUpdate(const char* str);
static int CMD_StrSpaces(const char* str);


// Serial RX buffer
static USART_TypeDef* uart_instance;
static volatile char new_char;
static volatile char buffer[_MAX_CMD_LENGTH];
static volatile int rx_read_ptr;
static volatile int rx_write_ptr;
#define RX_AVAILABLE_DATA() (((unsigned int)(rx_write_ptr - rx_read_ptr)) % _MAX_CMD_LENGTH)
#define RX_DATA(x) buffer[(rx_read_ptr + (x)) % _MAX_CMD_LENGTH]
#define RX_REMOVE_CHARS(x) {rx_read_ptr = (rx_read_ptr + (x)) % _MAX_CMD_LENGTH;}


/*
 * This callback is called by the HAL_UART_IRQHandler
 * when the given number of bytes are received
 * */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == uart_instance)
	{
		// Insert byte into circular buffer
		buffer[rx_write_ptr] = new_char;
		rx_write_ptr = (rx_write_ptr + 1) % _MAX_CMD_LENGTH;

		// Echo character back
		HAL_UART_Transmit(huart, (uint8_t*)&new_char, 1, 100);

		// Receive next character
		HAL_UART_Receive_IT(huart, (uint8_t*)&new_char, 1);
	}
}


/*
 * Reads (len) characters from the rx buffer and appends a '\0'
 * */
void ReadStringFromRXBuffer(char* dest, int len)
{
	while(len > 0)
	{
		*dest = buffer[rx_read_ptr];
		len--;
		dest++;
		rx_read_ptr = (rx_read_ptr + 1) % _MAX_CMD_LENGTH;
	}
	*dest = '\0';
}


/*
 * Initialize rx buffer and start reception
 * */
void CMD_Init(UART_HandleTypeDef* huart)
{
	rx_read_ptr = 0;
	rx_write_ptr = 0;
	uart_instance = huart->Instance;
	HAL_UART_Receive_IT(huart, (uint8_t*)&new_char, 1);
}


/*
 * Try to parse command,
 * check for '\n' terminated string in the receive buffer
 * if a valid command is found execute it
 * */
void CMD_TryParse(void)
{
	char cmd_str[_MAX_CMD_LENGTH+1];
	int i = 0;
	bool found = false;

	//Search for '\n' in the receive buffer
	while(!found && i < RX_AVAILABLE_DATA())
	{
		if(RX_DATA(i) == '\n' || RX_DATA(i) == '\r')
		{
			found = true;
		}

		i++;
	}

	//If '\n' is found parse string
	if(found)
	{
		//Read command string into cmd_str
		ReadStringFromRXBuffer(cmd_str, i - 1);

		//Remove '\n' '\r' characters
		while(RX_AVAILABLE_DATA() > 0 && (RX_DATA(0) == '\n' || RX_DATA(0) == '\r'))
		{
			RX_REMOVE_CHARS(1);
		}

		//Parse string
		CMD_ParseString(cmd_str);
	}
}


/*
 * Parse string
 * */
static void CMD_ParseString(const char* str)
{
	char command[16];
	const char* args;

	//Extract command from arguments
	int i = 0;
	int spaces = CMD_StrSpaces(str);
	while(str[i + spaces] != ' ' && str[i + spaces] != '\0' && i < 15)
	{
		command[i] = str[i + spaces];
		i++;
	}
	command[i] = '\0';

	args = str + spaces + i;
	args += CMD_StrSpaces(args);

	//Try to match a command
	if(strcmp(command, "update") == 0)
	{
		CMD_ParseUpdate(args);
	}
	else if(strlen(command) > 0)
	{
		CMD_ParseInvalid(str);
	}
}


/*
 * Parse invalid command,
 * echo command with error message
 * */
static void CMD_ParseInvalid(const char* str)
{
	printf("Invalid command: -%s-\n", str);
}


/*
 * Parse update command
 * */
static void CMD_ParseUpdate(const char* str)
{
	update = true;
}



/*
 * Returns the number of spaces at the beginning of the string
 * */
static int CMD_StrSpaces(const char* str)
{
	int i = 0;

	while(str[i] != 0 && str[i] == ' '){
		i++;
	}

	return i;
}
