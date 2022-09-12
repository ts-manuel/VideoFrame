/**
 ******************************************************************************
 * @file      console_task.c
 * @author    ts-manuel
 * @brief     Handles commands received from the serial port
 *
 ******************************************************************************
 */

#include "tasks/console_task.h"


//Function prototypes
void ConsoleReceive(char* buff, int len, bool from_usb);
static void CMD_ParseString(const char* str, ConsoleTaskArgs_t* args, bool* en_lpw);
static void CMD_ParseInvalid(const char* str);
static void CMD_ParseHelp(const char* str);
static void CMD_ParseDisplay(const char* str, ConsoleTaskArgs_t* args);
static void CMD_ParseLoad(const char* str_args, ConsoleTaskArgs_t* args);
static void CMD_ParseUpdate(const char* str, ConsoleTaskArgs_t* args);
static void CMD_ParseTaskInfo(const char* str);
static void CMD_ParseSleep(const char* str, ConsoleTaskArgs_t* args);
static void CMD_ParseFlash(const char* str, ConsoleTaskArgs_t* args);
static const char* CMD_Trim(const char* str, const char* msg);
static const char* CMD_TrimSpaces(const char* str);
static const char* CMD_ReadColor(const char* str, uint8_t* color);


// Serial RX buffer
const char bel = '\a';
static UART_HandleTypeDef* huart_handle;
static volatile char new_char;
static volatile char buffer[_MAX_CMD_LENGTH];
static volatile int rx_read_ptr = 0;
static volatile int rx_write_ptr = 0;
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
	if (huart->Instance == huart_handle->Instance)
	{
		ConsoleReceive((char*)&new_char, 1, false);

		//Receive next character
		HAL_UART_Receive_IT(huart, (uint8_t*)&new_char, 1);
	}
}


/*
 * Adds received characters to the rx buffer and echoes them back
 * */
void ConsoleReceive(char* buff, int len, bool from_usb)
{
	for(int i = 0; i < len; i++)
	{
		//If character is backspace
		if(buff[i] == 127)
		{
			//Remove last character from buffer
			if(rx_write_ptr != rx_read_ptr)
			{
				rx_write_ptr = (unsigned int)(rx_write_ptr - 1) % _MAX_CMD_LENGTH;

				//Echo character back
				if(from_usb)
					CDC_Transmit_FS((uint8_t*)&buff[i], 1);
				else
					HAL_UART_Transmit(huart_handle, (uint8_t*)&buff[i], 1, 100);
			}
		}
		//Enter character if buffer is not full
		else if(RX_AVAILABLE_DATA() < _MAX_CMD_LENGTH-1)
		{
			//Insert byte into circular buffer
			buffer[rx_write_ptr] = buff[i];
			rx_write_ptr = (rx_write_ptr + 1) % _MAX_CMD_LENGTH;

			//Echo character back
			if(from_usb)
				CDC_Transmit_FS((uint8_t*)&buff[i], 1);
			else
				HAL_UART_Transmit(huart_handle, (uint8_t*)&buff[i], 1, 100);

			new_char_available = true;
		}
		//Ring the bell when the buffer if full
		else
		{
			if(from_usb)
				CDC_Transmit_FS((uint8_t*)&bel, 1);
			else
				HAL_UART_Transmit(huart_handle, (uint8_t*)&bel, 1, 100);
		}
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
 * Load RX buffer with commands
 * */
bool InitCMDBuffer(const char* str)
{
	if(strlen(str) >= _MAX_CMD_LENGTH)
		return false;

	strcpy((char*)buffer, str);
	rx_write_ptr += strlen(str);
	new_char_available = true;

	return true;
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
	huart_handle = args->huart;
	HAL_UART_Receive_IT(args->huart, (uint8_t*)&new_char, 1);

	//Initialize USB
	MX_USB_DEVICE_Init();

	//Initialize SD
	if(SD_Init() != HAL_OK)
		printf("ERROR: Unable to initialize SD card\n");


	//Parse commands
	while(1)
	{
		char cmd_str[_MAX_CMD_LENGTH+1];
		int i = 0;
		bool found = false;

		//If there is a new character from the serial port
		if(new_char_available)
		{
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

			//Clear flag after handling the character and if there are no more commands pending
			if(RX_AVAILABLE_DATA() == 0)
				new_char_available = false;

			last_cmd_tick = osKernelGetTickCount();
		}

		//Enter low power mode if there are no commands for more than _SLEEP_TIMEOUT seconds
		uint32_t tick = osKernelGetTickCount();
		if((tick - last_cmd_tick) / osKernelGetTickFreq() > _SLEEP_TIMEOUT && low_power_timeout_enabled)
		{
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
	if((str_args = CMD_Trim(str, "help")))
	{
		CMD_ParseHelp(str_args);
	}
	else if((str_args = CMD_Trim(str, "display")))
	{
		CMD_ParseDisplay(str_args, args);
	}
	else if((str_args = CMD_Trim(str, "load")))
	{
		CMD_ParseLoad(str_args, args);
	}
	else if((str_args = CMD_Trim(str, "update")))
	{
		CMD_ParseUpdate(str_args, args);
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
	else if((str_args = CMD_Trim(str, "sleep")))
	{
		CMD_ParseSleep(str_args, args);
	}
	else if((str_args = CMD_Trim(str, "flash")))
	{
		CMD_ParseFlash(str_args, args);
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
 * Parse help command
 * print a list of all commands
 * */
static void CMD_ParseHelp(const char* str)
{

	if(CMD_Trim(str, "start"))
	{
		printf(
			"\n"
			"usage: start \n"
			"Start low power timer, after %d seconds the system enters low power mode, the serial communication is terminated. \n",
			_SLEEP_TIMEOUT
		);
	}
	else if(CMD_Trim(str, "stop"))
	{
		printf(
			"\n"
			"usage: stop \n"
			"Stop low power timer. \n"
			"The system remains operational until the low power timer is started with the start command or the sleep command is executed. \n"
		);
	}
	else if(CMD_Trim(str, "sleep"))
	{
		printf(
			"\n"
			"usage: sleep \n"
			"Enter low power mode immediately if not disabled. \n"
			"The sleep command can be disabled by pressing the BOOT while the system is running. \n"
			"Running the command enables itself, if executed 2 times, the second time it will work. \n"
		);
	}
	else if(CMD_Trim(str, "display"))
	{
		printf(
			"\n"
			"usage: display [color] \n"
			"usage: display [pattern] \n"
			"usage: display [pattern] [color]\n"
			"If only the color is specified, the screen is filled solid with that color. \n"
			"\n"
			"  Available colors: \n"
			"    black \n"
			"    white \n"
			"    green \n"
			"    blue \n"
			"    red \n"
			"    yellow \n"
			"    orange \n"
			"    clean \n"
			"\n"
			"  Test patterns:\n"
			"    grad:      [color] Draws a gradient with a specific color. \n"
			"    stripes:           Draws horizontal stripes whit all the colors. \n"
			"    lines:             Draws horizontal alternating black and white lines. \n"
			"    blocks:            Draws 8 rectanguar blocks, one for each color. \n"
		);
	}
	else if(CMD_Trim(str, "load"))
	{
		printf(
			"\n"
			"usage: load [path] \n"
			"Load image from SD card. \n"
		);
	}
	else if(CMD_Trim(str, "update"))
	{
		printf(
			"\n"
			"usage: update \n"
			"Load next image from SD card. \n"
		);
	}
	else if(CMD_Trim(str, "task-info"))
	{
		printf(
			"\n"
			"usage: task-info \n"
			"Print running tasks. \n"
		);
	}
	else if(CMD_Trim(str, "flash"))
	{
		printf(
				"\n"
			"usage: falsh erase \n"
			"usage: falsh dump [start] [stop] \n"
			"Commands: \n"
			"  erase: Erases the entire flash memory. \n"
			"  dump:  Prints the flash content. start and stop are decimal addresses. \n"
		);
	}
	else
	{
		printf(
			"usage: help [command]  \n"
			"To get help about a specific command. \n"
			"\n"
			"LIST OF COMMANDS \n"
			"  start:               Start low power timer, %d seconds timeout. \n"
			"  stop:                Stop low power timer, no timeout. \n"
			"  sleep:               Enter low power mode immediately if not disabled. \n"
			"  display: [pattern]   Display test pattern. \n"
			"  load:    [path]      Load image from SD card. \n"
			"  update:              Load next image from SD card. \n"
			"  task-info:           Print running tasks. \n"
			"  flash:   [action]    Read / Write internal flash. \n",
			_SLEEP_TIMEOUT
		);
	}
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


/*
 * Parse Load command
 * */
static void CMD_ParseLoad(const char* str_args, ConsoleTaskArgs_t* args)
{
	DisplayMessage_t msg;
	FRESULT fres;
	FIL file;

	printf("Loading <%s>\n", str_args);

	if(strlen(str_args) < _MAX_LFN*2+2)
	{
		//Open file
		if((fres = f_open(&file, str_args, FA_READ | FA_OPEN_EXISTING)) == FR_OK)
		{
			msg.action = e_DisplayJPEG;
			msg.fp = &file;

			//Trigger display task
			osMessageQueuePut(args->display_message_queue, &msg, 1, 0);
			osThreadFlagsSet(args->displayTaskId, _FLAG_DISPLAY_UPDATE);
			osThreadYield();

			//Close file
			int cnt = 0;
			do{
				osDelay(1000);
				fres = f_close(&file);
			} while(fres != FR_OK && cnt < 10);

			if(cnt >= 10)
				printf("ERROR: f_close returned %d\n", (int)fres);

			//Store file path to flash
			FLASH_StoreFilePath(str_args);
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
	}
}


/*
 * Updates the displayed image
 * */
static void CMD_ParseUpdate(const char* str, ConsoleTaskArgs_t* args)
{
	char curr_file_path[_FILE_PATH_MAX_LEN];
	char next_file_path[_FILE_PATH_MAX_LEN];
	bool display_splash_screen = false;

	if(!disk_status(0))
	{
		printf("Updating...\n");

		//Load current file path from flash
		FLASH_LoadFilePath(curr_file_path);

		//Find next file
		if(FMAN_FindNext(next_file_path, curr_file_path))
		{
			//Display new file
			CMD_ParseLoad(next_file_path, args);
		}
		else
		{
			printf("ERROR: Unable to find any jpeg file\n");
			display_splash_screen = true;
		}
	}
	else
	{
		printf("ERROR: Drive not mounted\n");
		display_splash_screen = true;
	}


	//Display bitmap
	if(display_splash_screen)
	{
		DisplayMessage_t msg;
		msg.action = e_DisplayBMP;
		msg.bmp = (uint8_t*)splash_screen;

		//Trigger display task
		osMessageQueuePut(args->display_message_queue, &msg, 1, 0);
		osThreadFlagsSet(args->displayTaskId, _FLAG_DISPLAY_UPDATE);
		osThreadYield();
	}
}


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

		printf("%d Thread <%20s>, Priority: %2d, Stack Space: %4ld Words, State: %s\n",
				i, name, (int)priority, stack_space, state);
	}
}


/*
 * Enter low power mode
 * */
static void CMD_ParseSleep(const char* str, ConsoleTaskArgs_t* args)
{
	if(*args->sleep_cmd_disabled == false)
	{
		printf("Entering low power mode\n");
		osDelay(1);
		PWR_EnterStandBy();
	}
	else
	{
		*args->sleep_cmd_disabled = false;
	}
}


/*
 * FLASH commands (erase and print content)
 * */
static void CMD_ParseFlash(const char* str, ConsoleTaskArgs_t* args)
{
	const char* ss_str;
	const int flash_add_min = 0;
	const int flash_add_max = 131072;
	char path[_FILE_PATH_MAX_LEN];

	if(strcmp(str, "erase") == 0)
	{
		printf("Erasing flash...\n");
		FLASH_Erase();
		printf("Done\n");
	}
	else if((ss_str = CMD_Trim(str, "dump")))
	{
		int strt_add = flash_add_min;
		int stop_add = flash_add_max;

		//Read start and stop addresses
		sscanf(ss_str, "%d %d", &strt_add, &stop_add);

		//Print flash content
		char* pt = (char*)0x080e0000;
		for(int i = strt_add; i < stop_add; i += 16)
		{
			printf("%05X: ", i);

			for(int j = 0; j < 16; j++)
				printf("%02X ", pt[j]);

			printf("  ");

			for(int j = 0; j < 16; j++)
			{
				if(isgraph((int)pt[j]))
					printf("%c", pt[j]);
				else
					printf(".");
			}

			printf("\n");

			pt += 16;
		}
	}
	else
	{
		//Print last valid entry
		bool valid = FLASH_LoadFilePath(path);
		printf("FLASH_Load() returned: %d, path: <%s>\n", valid, path);
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
