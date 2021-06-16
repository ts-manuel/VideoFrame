/**
 ******************************************************************************
 * @file      display_task.c
 * @author    ts-manuel
 * @brief     Updates the display
 *
 ******************************************************************************
 */

#include "tasks/display_task.h"


static void display_solid(uint8_t color);
static void display_bloks(void);
static void display_stripes(void);
static void display_lines(void);
static void display_gradient(uint8_t color);
static void display_jpeg(FIL* fp);


/*
 *
 * */
void StartDisplayTask(void *_args)
{
	const DisplayTaskArgs_t* args = (DisplayTaskArgs_t*)_args;

	while(1)
	{
		//Wait for flag
		osThreadFlagsWait(_FLAG_DISPLAY_UPDATE, osFlagsWaitAny, osWaitForever);

		//Read message
		DisplayMessage_t msg;
		if(osMessageQueueGet (args->message_queue, &msg, NULL, 100) == osOK)
		{

			//Initialize display
			DISP_Init();
			DISP_BeginUpdate();

			switch(msg.action)
			{
				case e_DisplaySolid:
					display_solid(msg.color);
					break;
				case e_DisplayBlocks:
					display_bloks();
					break;
				case e_DisplayStripes:
					display_stripes();
					break;
				case e_DisplayLines:
					display_lines();
					break;
				case e_DisplayGradient:
					display_gradient(msg.color);
					break;
				case e_DisplayJPEG:
					display_jpeg(msg.fp);
					break;
			}

			//Update display and enter low power mode
			DISP_EndUpdate();
			DISP_Sleep();
		}
		else
		{
			printf("DisplayTask: Unable to read message\n");
		}
	}
}


/*
 * Display solid color
 * */
static void display_solid(uint8_t color)
{
    for(int i = 0; i < _DISPLAY_WIDTH/2; i++)
    {
        for(int j = 0; j < _DISPLAY_HEIGHT; j++)
        {
        	DISP_SendData((color<<4)|color);
        }
    }
}


/*
 * Show 7 color blocks test pattern
 * */
static void display_bloks(void)
{
	for(int i = 0; i < _DISPLAY_HEIGHT/2; i++)
	{
		for(uint8_t k = 0; k < 4; k++)
		{
			for(int j = 0; j < _DISPLAY_WIDTH/8; j++)
			{
				DISP_SendData((k<<4)|k);
			}
		}
	}
	for(int i = 0; i < _DISPLAY_HEIGHT/2; i++)
	{
		for(uint8_t k = 4; k < 8; k++)
		{
			for(int j = 0; j < _DISPLAY_WIDTH/8; j++)
			{
				DISP_SendData((k<<4)|k);
			}
		}
	}
}


/*
 * Show colored stripes test pattern
 * (going from full white to this pattern is used
 *  to determine the typical power consumption)
 * */
static void display_stripes(void)
{
	for(int y = 0; y < _DISPLAY_HEIGHT; y++)
	{
		for(int x = 0; x < _DISPLAY_WIDTH/2; x++)
		{
			uint8_t c = y / (_DISPLAY_HEIGHT / 7);
			DISP_SendData((c << 4) | c);
		}
	}
}


/*
 * Show alternate black and white lines
 * (going from full white to this pattern is used
 *  to determine the maximum power consumption)
 * */
void display_lines(void)
{
	for(int y = 0; y < _DISPLAY_HEIGHT; y++)
	{
		for(int x = 0; x < _DISPLAY_WIDTH/2; x++)
		{
			uint8_t c = y % 2 == 0 ? 1 : 0;
			DISP_SendData((c << 4) | c);
		}
	}
}


/*
 * Show color gradient from full color to white and from black to color
 * */
static void display_gradient(uint8_t color)
{
	RGB16_t cl = display_colors[color];

	const float r0 = (float)cl.r / 255.f;
	const float g0 = (float)cl.g / 255.f;
	const float b0 = (float)cl.b / 255.f;

	//Send data
	for(int y = 0; y < EPD_5IN65F_HEIGHT; y++)
	{
		if(y < 448/2)
		{
			//Color to White
			for(int x = 0; x < EPD_5IN65F_WIDTH; x++)
			{
				float r1 = 1.f;
				float g1 = 1.f;
				float b1 = 1.f;
				float t = (float)x / (float)(EPD_5IN65F_WIDTH-1);

				uint8_t r = (uint8_t)((r0 * t + r1 * (1-t)) * 255.f);
				uint8_t g = (uint8_t)((g0 * t + g1 * (1-t)) * 255.f);
				uint8_t b = (uint8_t)((b0 * t + b1 * (1-t)) * 255.f);

				DISP_WritePixel(x, y, r, g, b);
			}
		}
		else
		{
			//Black to Color
			for(int x = 0; x < EPD_5IN65F_WIDTH; x++)
			{
				float r1 = 0.f;
				float g1 = 0.f;
				float b1 = 0.f;
				float t = 1.f - (float)x / (float)(EPD_5IN65F_WIDTH-1);

				uint8_t r = (uint8_t)((r0 * t + r1 * (1-t)) * 255.f);
				uint8_t g = (uint8_t)((g0 * t + g1 * (1-t)) * 255.f);
				uint8_t b = (uint8_t)((b0 * t + b1 * (1-t)) * 255.f);

				DISP_WritePixel(x, y, r, g, b);
			}
		}
	}
}


/*
 * Load jpeg image from SD card
 * */
static void display_jpeg(FIL* fp)
{
	JPG_t jpg;

	//Decode image
	if(JPG_decode(fp, &jpg))
	{
		printf("ERROR: JPG decoding failed\n");
	}
}
