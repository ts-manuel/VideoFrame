/**
 ******************************************************************************
 * @file      console_task.c
 * @author    ts-manuel
 * @brief     Handles commands received from the serial port
 *
 ******************************************************************************
 */

#include "tasks/console_task.h"
#include "hardware/sd.h"

//Function prototypes
static void CMD_ParseString(const char* str, ConsoleTaskArgs_t* args, bool* en_lpw);
static void CMD_ParseInvalid(const char* str);
static void CMD_ParseDisplay(const char* str, ConsoleTaskArgs_t* args);
static void CMD_ParseLoad(const char* str_args, ConsoleTaskArgs_t* args);
//static void CMD_ParseUpdate(const char* str);
static void CMD_ParseTaskInfo(const char* str);
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
static void ReadStringFromRXBuffer(char* dest, int len)
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
 *
 * */
void StartConsoleTask(void *_args)
{
	ConsoleTaskArgs_t* args = (ConsoleTaskArgs_t*)_args;
	uint32_t last_cmd_tick = osKernelGetTickCount();
	bool low_power_timeout_enabled = true;

	//Initialize RX buffer and start UART ISR
	rx_read_ptr = 0;
	rx_write_ptr = 0;
	uart_instance = args->huart->Instance;
	HAL_UART_Receive_IT(args->huart, (uint8_t*)&new_char, 1);

	//Parse commands
	while(1)
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
				CMD_ParseString(cmd_str, args, &low_power_timeout_enabled);
			}

			last_cmd_tick = osKernelGetTickCount();
		}

		//Enter low power mode if there are no commands for more than _SLEEP_TIMEOUT seconds
		uint32_t tick = osKernelGetTickCount();
		if((tick - last_cmd_tick) / osKernelGetTickFreq() > _SLEEP_TIMEOUT && low_power_timeout_enabled)
		{
			PWR_WriteBacupData(args->state, false);

			printf("Entering low power mode\n");
			osDelay(1);

			PWR_EnterStandBy();
		}
	}
}


/*
 * Parse string
 * */
static void CMD_ParseString(const char* str, ConsoleTaskArgs_t* args, bool* en_lpw)
{
	const char* str_args;

	//Remove spaces
	str = CMD_TrimSpaces(str);

	//Try to match a command
	if((str_args = CMD_Trim(str, "display")))
	{
		CMD_ParseDisplay(str_args, args);
	}
	else if((str_args = CMD_Trim(str, "load")))
	{
		CMD_ParseLoad(str_args, args);
	}
	else if((str_args = CMD_Trim(str, "update")))
	{
		//CMD_ParseUpdate(str_args, args);
	}
	else if((str_args = CMD_Trim(str, "start")))
	{
		*en_lpw = true;
		printf("Low power timeout enabled\n");
	}
	else if((str_args = CMD_Trim(str, "stop")))
	{
		*en_lpw = false;
		printf("Low power timeout disabled\n");
	}
	else if((str_args = CMD_Trim(str, "task-info")))
	{
		CMD_ParseTaskInfo(str_args);
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
static void CMD_ParseDisplay(const char* str, ConsoleTaskArgs_t* args)
{
	const char* color_name;
	DisplayMessage_t msg;

	printf("Wait...\n");

	if((color_name = CMD_Trim(str, "grad")))
	{
		uint8_t color;

		if(CMD_ReadColor(color_name, &color) != NULL)
		{
			msg.color = color;
			msg.action = e_DisplayGradient;
		}
		else
		{
			printf("Invalid color -%s-\n", color_name);
			return;
		}
	}
	else if(strcmp(str, "stripes") == 0)
		msg.action = e_DisplayStripes;
	else if(strcmp(str, "lines") == 0)
		msg.action = e_DisplayLines;
	else if(strcmp(str, "blocks") == 0)
		msg.action = e_DisplayBlocks;
	else
	{
		uint8_t color;

		if(CMD_ReadColor(str, &color) != NULL)
		{
			msg.color = color;
			msg.action = e_DisplaySolid;
		}
		else
		{
			printf("Invalid argument -%s-\n", str);
			return;
		}
	}

	//Trigger display task
	osMessageQueuePut(args->display_message_queue, &msg, 1, 0);
	osThreadFlagsSet(args->displayTaskId, _FLAG_DISPLAY_UPDATE);
	osThreadYield();

	printf("Done\n");
}

extern SD_HandleTypeDef hsd;
extern FATFS fs;
/*
 * Parse Load command
 * */
static void CMD_ParseLoad(const char* str_args, ConsoleTaskArgs_t* args)
{
	DisplayMessage_t msg;
	FRESULT fres;
	FIL file;

	printf("Wait...\n");

	/*if(SD_Init(&hsd, &fs) != HAL_OK)
		printf("ERROR: Unable to initialize SD card\n");
	else
		printf("SD Initialized OK!\n");*/

	/*if(strlen(str_args) < _MAX_LFN*2+2)
	{
		if(SD_Init(&hsd, &fs) != HAL_OK)
			printf("ERROR: Unable to initialize SD card\n");
		else
			printf("SD Initialized OK!\n");


		if((fres = f_open(&file, str_args, FA_READ | FA_OPEN_EXISTING)) == FR_OK)
		{
			msg.action = e_DisplayJPEG;
			msg.fp = &file;

			//Trigger display task
			osMessageQueuePut(args->display_message_queue, &msg, 1, 0);
			osThreadFlagsSet(args->displayTaskId, _FLAG_DISPLAY_UPDATE);
			osThreadYield();

			//Close file
			f_close(&file);
		}
		else
		{
			printf("ERROR: Unable to open file, f_open returned %d\n", (int)fres);
		}

		printf("Done\n");
	}
	else
	{
		printf("ERROR: Invalid file name (only 8.3 filename supported)\n");
	}*/
}


/*
 * Forces display update cycle
 * */
/*static void CMD_ParseUpdate(const char* str)
{
	update = true;
}*/


/*
 * Print info on all running tasks
 * */
static void CMD_ParseTaskInfo(const char* str)
{
	const char* state_to_str[] = {"Inactive", "Ready", "Running", "Blocked", "Terminated", "Error"};
	osThreadId_t thread_IDs[16];
	uint32_t thread_count;

	//Get ID for all running threads
	thread_count = osThreadEnumerate (thread_IDs, 16);

	for(int i = 0; i < thread_count; i++)
	{
		osThreadId_t id = thread_IDs[i];
		const char* name = osThreadGetName(id);
		osPriority_t priority = osThreadGetPriority(id);
		//uint32_t stack_size = osThreadGetStackSize(id);
		uint32_t stack_space = osThreadGetStackSpace(id);
		const char* state = state_to_str[osThreadGetState(id)];

		printf("%d Thread <%s>, Priority: %d, Stack Space: %ldBytes, State: %s\n",
				i, name, (int)priority, stack_space, state);
	}
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
