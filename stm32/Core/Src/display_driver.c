/**
 ******************************************************************************
 * @file      display_driver.c
 * @author    ts-manuel
 * @brief     Driver for the E-Paper display
 *
 ******************************************************************************
 */

#include "display_driver.h"

static bool initilized = false;
static bool prev_state;


/*
 * Initialize display
 * */
void DISP_Init(void)
{
	EPD_5IN65F_Init();
	initilized = true;
}

/*
 * Puts the display into low power mode
 * */
void DISP_Sleep(void)
{
	EPD_5IN65F_Sleep();
	initilized = false;
}

/*
 * Begin the update cycle
 * */
void DISP_BeginUpdate(void)
{
	prev_state = initilized;

	//Initialize the display if it is not already
	if(!initilized)
		EPD_5IN65F_Init();

	//Send commands (set resolution, data start)
	EPD_5IN65F_SendCommand(0x61);
	EPD_5IN65F_SendData(0x02);
	EPD_5IN65F_SendData(0x58);
	EPD_5IN65F_SendData(0x01);
	EPD_5IN65F_SendData(0xC0);
	EPD_5IN65F_SendCommand(0x10);
}


/*
 * Terminate the update cycle
 * */
void DISP_EndUpdate(void)
{
	//Send commands (power on, refresh, power of)
	EPD_5IN65F_SendCommand(0x04);
	EPD_5IN65F_BusyHigh();
	EPD_5IN65F_SendCommand(0x12);
	EPD_5IN65F_BusyHigh();
	EPD_5IN65F_SendCommand(0x02);
	EPD_5IN65F_BusyLow();

	//Leave the display in the same state it was found
	if(!prev_state)
		DISP_Sleep();
}

/*
 * Clear the display with a solid color
 * */
void DISP_Clear(uint8_t color)
{
	bool state = initilized;

	//Initialize the display if it is not already
	if(!initilized)
		EPD_5IN65F_Init();

	//Send data
	EPD_5IN65F_Clear(color);

	//Leave the display in the same state it was found
	if(!state)
		DISP_Sleep();
}

/*
 * Show 7 color blocks test pattern
 * */
void DISP_ShowBlocks(void)
{
	prev_state = initilized;

	//Initialize the display if it is not already
	if(!initilized)
		EPD_5IN65F_Init();

	//Send data
	EPD_5IN65F_Show7Block();

	//Leave the display in the same state it was found
	if(!prev_state)
		DISP_Sleep();
}

/*
 * Show colored stripes test pattern
 * (going from full white to this pattern is used
 *  to determine the typical power consumption)
 * */
void DISP_ShowStripes(void)
{
	prev_state = initilized;
	uint8_t colors[7] = {
			EPD_5IN65F_BLUE,
			EPD_5IN65F_ORANGE,
			EPD_5IN65F_YELLOW,
			EPD_5IN65F_GREEN,
			EPD_5IN65F_RED,
			EPD_5IN65F_BLACK,
			EPD_5IN65F_WHITE
	};

	//Initialize the display if it is not already
	if(!initilized)
		EPD_5IN65F_Init();

	DISP_BeginUpdate();

	//Send data
	for(int y = 0; y < EPD_5IN65F_HEIGHT; y++)
	{
		for(int x = 0; x < EPD_5IN65F_WIDTH/2; x++)
		{
			uint8_t c = colors[y / (EPD_5IN65F_HEIGHT / 7)];
			EPD_5IN65F_SendData((c << 4) | c);
		}
	}

	DISP_EndUpdate();

	//Leave the display in the same state it was found
	if(!prev_state)
		DISP_Sleep();
}

/*
 * Show alternate black and white lines
 * (going from full white to this pattern is used
 *  to determine the maximum power consumption)
 * */
void DISP_ShowLines(void)
{
	prev_state = initilized;

	//Initialize the display if it is not already
	if(!initilized)
		EPD_5IN65F_Init();

	DISP_BeginUpdate();

	//Send data
	for(int y = 0; y < EPD_5IN65F_HEIGHT; y++)
	{
		for(int x = 0; x < EPD_5IN65F_WIDTH/2; x++)
		{
			uint8_t c = y % 2 == 0 ? EPD_5IN65F_WHITE : EPD_5IN65F_BLACK;
			EPD_5IN65F_SendData((c << 4) | c);
		}
	}

	DISP_EndUpdate();

	//Leave the display in the same state it was found
	if(!prev_state)
		DISP_Sleep();
}
