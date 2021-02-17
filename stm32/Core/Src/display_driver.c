/**
 ******************************************************************************
 * @file      display_driver.c
 * @author    ts-manuel
 * @brief     Driver for the E-Paper display
 *
 ******************************************************************************
 */

#include "display_driver.h"

#define _STRIPE_HEIGHT 16
#define STRIPE_PIXEL_R(x, y) stripe[((y)*EPD_5IN65F_WIDTH + (x))*3    ]
#define STRIPE_PIXEL_G(x, y) stripe[((y)*EPD_5IN65F_WIDTH + (x))*3 + 1]
#define STRIPE_PIXEL_B(x, y) stripe[((y)*EPD_5IN65F_WIDTH + (x))*3 + 2]
static int16_t stripe[EPD_5IN65F_WIDTH * (_STRIPE_HEIGHT+1) * 3];
static int pixelCount;
static int stripeCounter;
static int stripeHeight = _STRIPE_HEIGHT;
static int stripeSize = EPD_5IN65F_WIDTH * _STRIPE_HEIGHT;
static bool initilized = false;
static bool prev_state;

typedef struct{
	int16_t r;
	int16_t g;
	int16_t b;
} RGB16_t;

#define _NUM_COLORS 7
static const RGB16_t colors[_NUM_COLORS] = {
		{0x00, 0x00, 0x00},	//EPD_5IN65F_BLACK
		{0xff, 0xff, 0xff},	//EPD_5IN65F_WHITE
		{0x00, 0xff, 0x00},	//EPD_5IN65F_GREEN
		{0x00, 0x00, 0xff},	//EPD_5IN65F_BLUE
		{0xff, 0x00, 0x00},	//EPD_5IN65F_RED
		{0xff, 0xff, 0x00},	//EPD_5IN65F_YELLOW
		{0xff, 0x80, 0x00}	//EPD_5IN65F_ORANGE
		//{0xaa, 0x6e, 0x96}	//EPD_5IN65F_CLEAN
};


static void SendStripe(void);
static uint8_t FindClosestColor(RGB16_t color);




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

	pixelCount = 0;
}


/*
 * Terminate the update cycle
 * */
void DISP_EndUpdate(void)
{
	//Send remaining pixels
	while(pixelCount < EPD_5IN65F_HEIGHT * EPD_5IN65F_WIDTH)
	{
		EPD_5IN65F_SendData(EPD_5IN65F_BLACK << 4 | EPD_5IN65F_BLACK);
		pixelCount += 2;
	}

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


/*
 * Set the height of the stripe buffer
 * 	8 for jpeg files that doesn't use chroma subsampling
 * 	16 for jpeg files that use chroma subsampling
 * */
void DISP_SetStripeHeight(int h)
{
	if(h == 8 || h == 16)
	{
		stripeHeight = h;
		stripeSize = EPD_5IN65F_WIDTH * h;
	}
}


/*
 * Write pixel to the display
 * */
void DISP_WritePixel(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
	//Clear last row of pixel at the beginning of the scan
	if(x == 0 && y == 0)
	{
		for(int i = 0; i < EPD_5IN65F_WIDTH; i++)
		{
			STRIPE_PIXEL_R(i, stripeHeight) = 0;
			STRIPE_PIXEL_G(i, stripeHeight) = 0;
			STRIPE_PIXEL_B(i, stripeHeight) = 0;
		}
	}

	//Write pixel into buffer
	STRIPE_PIXEL_R(x, y % stripeHeight) = (int16_t)r;
	STRIPE_PIXEL_G(x, y % stripeHeight) = (int16_t)g;
	STRIPE_PIXEL_B(x, y % stripeHeight) = (int16_t)b;
	stripeCounter++;

	//Send pixels to the display when the stripe is completed
	if(stripeCounter >= stripeSize)
	{
		SendStripe();
		stripeCounter = 0;
	}
}


/*
 * Send pixels to the display
 * */
static void SendStripe(void)
{
	uint8_t last_code = 0;

	for(int y = 0; y < stripeHeight; y++)
	{
		for(int x = 0; x < EPD_5IN65F_WIDTH; x++)
		{
#if _DITHER == 1	//Floyd–Steinberg dithering
			uint8_t new_code;
			RGB16_t old_color, new_color, quant_err;

			if(y == 0)
			{
				old_color.r = STRIPE_PIXEL_R(x, y) + STRIPE_PIXEL_R(x, stripeHeight);
				old_color.g = STRIPE_PIXEL_G(x, y) + STRIPE_PIXEL_G(x, stripeHeight);
				old_color.b = STRIPE_PIXEL_B(x, y) + STRIPE_PIXEL_B(x, stripeHeight);
				STRIPE_PIXEL_R(x, stripeHeight) = 0;
				STRIPE_PIXEL_G(x, stripeHeight) = 0;
				STRIPE_PIXEL_B(x, stripeHeight) = 0;
			}
			else
			{
				old_color.r = STRIPE_PIXEL_R(x, y);
				old_color.g = STRIPE_PIXEL_G(x, y);
				old_color.b = STRIPE_PIXEL_B(x, y);
			}

			//Find closest color and quantization error
			new_code = FindClosestColor(old_color);
			new_color = colors[new_code];
			quant_err.r = old_color.r - new_color.r;
			quant_err.g = old_color.g - new_color.g;
			quant_err.b = old_color.b - new_color.b;

			//Write color to the display
			if(x % 2 == 0)
			{
				last_code = new_code;
			}
			else
			{
				EPD_5IN65F_SendData((last_code << 4) | new_code);
				pixelCount += 2;
			}

			//Propagate quantization error
			if(x < EPD_5IN65F_WIDTH-1)
			{
				STRIPE_PIXEL_R(x+1, y) = (STRIPE_PIXEL_R(x+1, y)*16 + 7*quant_err.r) / 16;
				STRIPE_PIXEL_G(x+1, y) = (STRIPE_PIXEL_G(x+1, y)*16 + 7*quant_err.g) / 16;
				STRIPE_PIXEL_B(x+1, y) = (STRIPE_PIXEL_B(x+1, y)*16 + 7*quant_err.b) / 16;

				STRIPE_PIXEL_R(x+1, y+1) = (STRIPE_PIXEL_R(x+1, y+1)*16 + 1*quant_err.r) / 16;
				STRIPE_PIXEL_G(x+1, y+1) = (STRIPE_PIXEL_G(x+1, y+1)*16 + 1*quant_err.g) / 16;
				STRIPE_PIXEL_B(x+1, y+1) = (STRIPE_PIXEL_B(x+1, y+1)*16 + 1*quant_err.b) / 16;
			}

			if(x > 0)
			{
				STRIPE_PIXEL_R(x-1, y+1) = (STRIPE_PIXEL_R(x-1, y+1)*16 + 5*quant_err.r) / 16;
				STRIPE_PIXEL_G(x-1, y+1) = (STRIPE_PIXEL_G(x-1, y+1)*16 + 5*quant_err.g) / 16;
				STRIPE_PIXEL_B(x-1, y+1) = (STRIPE_PIXEL_B(x-1, y+1)*16 + 5*quant_err.b) / 16;
			}

			STRIPE_PIXEL_R(x, y+1) = (STRIPE_PIXEL_R(x, y+1)*16 + 3*quant_err.r) / 16;
			STRIPE_PIXEL_G(x, y+1) = (STRIPE_PIXEL_G(x, y+1)*16 + 3*quant_err.g) / 16;
			STRIPE_PIXEL_B(x, y+1) = (STRIPE_PIXEL_B(x, y+1)*16 + 3*quant_err.b) / 16;
#else	//No dithering
			RGB16_t color;
			uint8_t new_code;

			color.r = STRIPE_PIXEL_R(x, y);
			color.g = STRIPE_PIXEL_G(x, y);
			color.b = STRIPE_PIXEL_B(x, y);
			new_code = FindClosestColor(color);

			//Write color to the display
			if(x % 2 == 0)
			{
				last_code = new_code;
			}
			else
			{
				EPD_5IN65F_SendData((last_code << 4) | new_code);
				pixelCount += 2;
			}
#endif
		}
	}
}


/*
 * Returns the closest color from the 7 color-palatte
 * */
static uint8_t FindClosestColor(RGB16_t color)
{
	uint8_t index = 0;
	int closest = 0x7fffffff;

	for(int i = 0; i < _NUM_COLORS; i++)
	{
		int r0 = (int)colors[i].r;
		int g0 = (int)colors[i].g;
		int b0 = (int)colors[i].b;
		int r1 = (int)color.r;
		int g1 = (int)color.g;
		int b1 = (int)color.b;

		int dist = (r0-r1)*(r0-r1) + (g0-g1)*(g0-g1) + (b0-b1)*(b0-b1);

		if(dist < closest)
		{
			closest = dist;
			index = i;
		}
	}

	return index;
}
