/**
 ******************************************************************************
 * @file      cmd_parser.c
 * @author    ts-manuel
 * @brief     Handles commands received by the serial port
 *
 ******************************************************************************
 */

#include "cmd_parser.h"

//Extern Variables
extern bool update;
extern bool running;
extern char file_name[];
extern char folder_name[];

//Function prototypes
static void CMD_ParseString(const char* str);
static void CMD_ParseInvalid(const char* str);
static void CMD_ParseDisplay(const char* str);
static void CMD_ParseLoad(const char* str);
static void CMD_ParseStart(const char* str);
static void CMD_ParseStop(const char* str);
static void CMD_ParseUpdate(const char* str);
static const char* CMD_Trim(const char* str, const char* msg);
static const char* CMD_TrimSpaces(const char* str);
static const char* CMD_ReadColor(const char* str, uint8_t* color);


// Serial RX buffer
static USART_TypeDef* uart_instance;
static volatile char new_char;
static volatile char buffer[_MAX_CMD_LENGTH];
static volatile int rx_read_ptr;
static volatile int rx_write_ptr;
static volatile bool new_char_available = false;
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
		//If character is backspace
		if(new_char == 127)
		{
			//Remove last character from buffer
			if(rx_write_ptr != rx_read_ptr)
			{
				rx_write_ptr = (unsigned int)(rx_write_ptr - 1) % _MAX_CMD_LENGTH;

				//Echo character back
				HAL_UART_Transmit(huart, (uint8_t*)&new_char, 1, 100);
			}
		}
		//Enter character if buffer is not full
		else if(RX_AVAILABLE_DATA() < _MAX_CMD_LENGTH-1)
		{
			//Insert byte into circular buffer
			buffer[rx_write_ptr] = new_char;
			rx_write_ptr = (rx_write_ptr + 1) % _MAX_CMD_LENGTH;

			//Echo character back
			HAL_UART_Transmit(huart, (uint8_t*)&new_char, 1, 100);

			new_char_available = true;
		}
		//Ring the bell when the buffer if full
		else
		{
			char bel = '\a';
			HAL_UART_Transmit(huart, (uint8_t*)&bel, 1, 100);
		}

		//Receive next character
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

	//If there is a new character from the serial port
	if(new_char_available)
	{
		new_char_available = false;

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
}


/*
 * Parse string
 * */
static void CMD_ParseString(const char* str)
{
	const char* args;

	//Remove spaces
	str = CMD_TrimSpaces(str);

	//Try to match a command
	if((args = CMD_Trim(str, "update")))
	{
		CMD_ParseUpdate(args);
	}
	else if((args = CMD_Trim(str, "display")))
	{
		CMD_ParseDisplay(args);
	}
	else if((args = CMD_Trim(str, "load")))
	{
		CMD_ParseLoad(args);
	}
	else if((args = CMD_Trim(str, "start")))
	{
		CMD_ParseStart(args);
	}
	else if((args = CMD_Trim(str, "stop")))
	{
		CMD_ParseStop(args);
	}
	else if(strlen(str) > 0)
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
static void CMD_ParseDisplay(const char* str)
{
	const char* color_name;

	printf("Wait...\n");

	if((color_name = CMD_Trim(str, "grad")))
	{
		uint8_t color;

		if(CMD_ReadColor(color_name, &color) != NULL)
		{
			DISP_ShowGradient(color);
		}
		else
		{
			printf("Invalid color -%s-\n", color_name);

			return;
		}
	}
	else if(strcmp(str, "stripes") == 0)
		DISP_ShowStripes();
	else if(strcmp(str, "lines") == 0)
		DISP_ShowLines();
	else if(strcmp(str, "blocks") == 0)
		DISP_ShowBlocks();
	else
	{
		uint8_t color;

		if(CMD_ReadColor(str, &color) != NULL)
		{
			DISP_Clear(color);
		}
		else
		{
			printf("Invalid argument -%s-\n", str);
			return;
		}
	}

	printf("Done\n");
}


/*
 * Parse Load command
 * */
static void CMD_ParseLoad(const char* str)
{
	if(strlen(str) < (_MAX_LFN+1)*2 + 2)
	{
		bool found = false; // Found '/' or '\'
		int i = 0, j = 0;

		//Copy characters to the folder name until eater '\' or '/'
		// are found and than copy the remaining characters to the  file name
		folder_name[0] = '\0';
		file_name[0] = '\0';
		while(str[i] != '\0' && j < _MAX_LFN+1)
		{
			if(str[i] == '\\' || str[i] == '/')
			{
				if(found)
				{
					printf("ERROR: Invalid file path (nested directories not supported)\n");
				}

				found = true;
				folder_name[j] = '\0';	//Zero terminate string
				j = 0;
			}
			else
			{
				if(!found)
				{
					folder_name[j++] = str[i];
				}
				else
				{
					file_name[j++] = str[i];
				}
			}

			i++;
		}
		file_name[j] = '\0';	//Zero terminate string

		update = true;
	}
	else
	{
		printf("ERROR: Invalid file name (only 8.3 filename supported)\n");
	}
}


/*
 * Starts display update
 * */
static void CMD_ParseStart(const char* str)
{
	running = true;
	printf("Start updating\n");
}


/*
 * Stops display update
 * */
static void CMD_ParseStop(const char* str)
{
	running = false;
	printf("Stop updating\n");
}


/*
 * Forces display update cycle
 * */
static void CMD_ParseUpdate(const char* str)
{
	update = true;
}


/*
 * If msg appears at the beginning of the string, remove msg + following spaces
 * and return pointer to the following part.
 * Else return NULL
 * */
static const char* CMD_Trim(const char* str, const char* msg)
{
	if(str != NULL && strstr(str, msg))
	{
		return CMD_TrimSpaces(str + strlen(msg));
	}
	else
	{
		return NULL;
	}
}


/*
 * Remove spaces from the beginning of the string
 * */
static const char* CMD_TrimSpaces(const char* str)
{
	while(*str == ' ')
		str++;

	return str;
}


/*
 * Read a color name from the string
 * */
static const char* CMD_ReadColor(const char* str, uint8_t* color)
{
	const char* str1;

	//Search for color
	if((str1 = CMD_Trim(str, "black")))
		*color = EPD_5IN65F_BLACK;
	else if((str1 = CMD_Trim(str, "white")))
		*color = EPD_5IN65F_WHITE;
	else if((str1 = CMD_Trim(str, "green")))
		*color = EPD_5IN65F_GREEN;
	else if((str1 = CMD_Trim(str, "blue")))
		*color = EPD_5IN65F_BLUE;
	else if((str1 = CMD_Trim(str, "red")))
		*color = EPD_5IN65F_RED;
	else if((str1 = CMD_Trim(str, "yellow")))
		*color = EPD_5IN65F_YELLOW;
	else if((str1 = CMD_Trim(str, "orange")))
		*color = EPD_5IN65F_ORANGE;
	else if((str1 = CMD_Trim(str, "clean")))
		*color = EPD_5IN65F_CLEAN;
	else
		return NULL;

	return CMD_TrimSpaces(str1);
}
