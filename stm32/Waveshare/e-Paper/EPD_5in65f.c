/*****************************************************************************
* | File      	:   EPD_5in65f.c
* | Author      :   Waveshare team
* | Function    :   5.65inch e-paper
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2020-07-07
* | Info        :
* -----------------------------------------------------------------------------
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "EPD_5in65f.h"

/******************************************************************************
function :	Software reset
parameter:
******************************************************************************/
static void EPD_5IN65F_Reset(void)
{
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(200);
    DEV_Digital_Write(EPD_RST_PIN, 0);
    DEV_Delay_ms(1);
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(200);
}

/******************************************************************************
function :	send command
parameter:
     Reg : Command register
******************************************************************************/
void EPD_5IN65F_SendCommand(UBYTE Reg)
{
    DEV_Digital_Write(EPD_DC_PIN, 0);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Reg);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
******************************************************************************/
void EPD_5IN65F_SendData(UBYTE Data)
{
    DEV_Digital_Write(EPD_DC_PIN, 1);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Data);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}


void EPD_5IN65F_BusyHigh(void)// If BUSYN=0 then waiting
{
    while(!(DEV_Digital_Read(EPD_BUSY_PIN)));
}

void EPD_5IN65F_BusyLow(void)// If BUSYN=1 then waiting
{
    while(DEV_Digital_Read(EPD_BUSY_PIN));
}

uint8_t vcom_lut[] = {
	 1,
	 0b00011000,
	 0b00000000,
	 5,100,100,0,0,0,0,0,

	 6,
	 0b01001000,
	 0b00000000,
	 25,1,25,0,0,0,0,0,

	 1,
	 0b00111111,
	 0b11111111,
	 50,1,0,0,0,0,0,0
};

/*uint8_t black_lut[] = {
	 4,
	 0b01110111,
	 0b01110111,
	 0b01110111,
	 0b01110111,
	 5,100,100,0,0,0,0,0
};*/

uint8_t black_lut[] = {
	 1,
	 0b00000001,
	 0b00100011,
	 0b00000000,
	 0b00000000,
	 5,100,100,50,50,0,0,0
};

uint8_t white_lut[] = {
	 1,
	 0b00000010,
	 0b00010011,
	 0b00000000,
	 0b00000000,
	 5,100,100,50,50,0,0,0
};

uint8_t xon_lut[10] = {
	0,
	0b11111111,
	0,0,0,0,0,0,0,0
};

void EPD_5IN65F_SetVCOM_LUT()
{
	EPD_5IN65F_SendCommand(0x20);
	for(int i = 0; i < 220; i++){
		if(i < sizeof(vcom_lut))
			EPD_5IN65F_SendData(vcom_lut[i]);
		else
			EPD_5IN65F_SendData(0x00);
	}
	DEV_Delay_ms(100);
}

//Offset 0 - 7
void EPD_5IN65F_SetCOLOR_LUT_black(uint8_t offset)
{
	EPD_5IN65F_SendCommand(0x21 + offset);
	for(int i = 0; i < 260; i++){
		if(i < sizeof(vcom_lut))
			EPD_5IN65F_SendData(black_lut[i]);
		else
			EPD_5IN65F_SendData(0x00);
	}
	DEV_Delay_ms(100);
}

void EPD_5IN65F_SetCOLOR_LUT_white(uint8_t offset)
{
	EPD_5IN65F_SendCommand(0x21 + offset);
	for(int i = 0; i < 260; i++){
		if(i < sizeof(vcom_lut))
			EPD_5IN65F_SendData(white_lut[i]);
		else
			EPD_5IN65F_SendData(0x00);
	}
	DEV_Delay_ms(100);
}

void EPD_5IN65F_SetXON_LUT(uint8_t* lut)
{
	EPD_5IN65F_SendCommand(0x29);
	for(int i = 0; i < 20; i++){
		for(int j = 0; j < 10; j++)
		{
			EPD_5IN65F_SendData(lut[j]);
		}
	}
	DEV_Delay_ms(100);
}


/******************************************************************************
function :	Initialize the e-Paper register
parameter:
******************************************************************************/
void EPD_5IN65F_Init(uint16_t vcom_mv)
{
	EPD_5IN65F_Reset();
    EPD_5IN65F_BusyHigh();
    EPD_5IN65F_SendCommand(0x00);
    //EPD_5IN65F_SendData(0xCF);	//Use internal LUTs
    //EPD_5IN65F_SendData(0x80);
    EPD_5IN65F_SendData(0xEF);		//Use FLASH LUTs
    EPD_5IN65F_SendData(0x08);
    EPD_5IN65F_SendCommand(0x01);
    EPD_5IN65F_SendData(0x37);
    EPD_5IN65F_SendData(0x00);	//VGH = +20V, VGL = -20V
    EPD_5IN65F_SendData(0x23);	//VSHC_LVL = +10V
    EPD_5IN65F_SendData(0x23);	//VSHC_LVL = -10V
    EPD_5IN65F_SendCommand(0x03);
    EPD_5IN65F_SendData(0x00);	//Power OFF VSH/VSL and VGH/VGL after 1 frame
    EPD_5IN65F_SendCommand(0x06);
    EPD_5IN65F_SendData(0xC7);
    EPD_5IN65F_SendData(0xC7);
    EPD_5IN65F_SendData(0x1D);
    EPD_5IN65F_SendCommand(0x30);	//PLL Control
    EPD_5IN65F_SendData(0x3C);	//50Hz
    EPD_5IN65F_SendCommand(0x40);
    EPD_5IN65F_SendData(0x00);
    EPD_5IN65F_SendCommand(0x50);
    EPD_5IN65F_SendData(0x37);
    EPD_5IN65F_SendCommand(0x60);
    EPD_5IN65F_SendData(0x22);
    EPD_5IN65F_SendCommand(0x61);	//Resolution setting
    EPD_5IN65F_SendData(0x02);		//600x448
    EPD_5IN65F_SendData(0x58);
    EPD_5IN65F_SendData(0x01);
    EPD_5IN65F_SendData(0xC0);
    EPD_5IN65F_SendCommand(0xE3);
    EPD_5IN65F_SendData(0xAA);

	DEV_Delay_ms(100);
    EPD_5IN65F_SendCommand(0x50);
    EPD_5IN65F_SendData(0x37);
    EPD_5IN65F_SendCommand(0x82);		//Set Vcom
    EPD_5IN65F_SendData(vcom_mv / 50);	//data = VCOM(mV) / 50

    //EPD_5IN65F_LoadLUTs();

	//EPD_5IN65F_SendCommand(0x65);	//SPI Flash control
	//EPD_5IN65F_SendData(0x01);		//bypass
}

void EPD_5IN65F_LoadLUTs(void)
{
	EPD_5IN65F_SetVCOM_LUT();
	EPD_5IN65F_SetCOLOR_LUT_black(0);
	EPD_5IN65F_SetCOLOR_LUT_white(1);
	EPD_5IN65F_SetCOLOR_LUT_black(2);
	EPD_5IN65F_SetCOLOR_LUT_black(3);
	EPD_5IN65F_SetCOLOR_LUT_black(4);
	EPD_5IN65F_SetCOLOR_LUT_black(5);
	EPD_5IN65F_SetCOLOR_LUT_black(6);
	EPD_5IN65F_SetCOLOR_LUT_black(7);
	EPD_5IN65F_SetXON_LUT(xon_lut);
}

/******************************************************************************
function :	Clear screen
parameter:
******************************************************************************/
void EPD_5IN65F_Clear(UBYTE color)
{
    EPD_5IN65F_SendCommand(0x61);//Set Resolution setting
    EPD_5IN65F_SendData(0x02);
    EPD_5IN65F_SendData(0x58);
    EPD_5IN65F_SendData(0x01);
    EPD_5IN65F_SendData(0xC0);
    EPD_5IN65F_SendCommand(0x10);
    for(int i=0; i<EPD_5IN65F_WIDTH/2; i++) {
        for(int j=0; j<EPD_5IN65F_HEIGHT; j++)
            EPD_5IN65F_SendData((color<<4)|color);
    }
    EPD_5IN65F_SendCommand(0x04);//0x04
    EPD_5IN65F_BusyHigh();
    EPD_5IN65F_SendCommand(0x12);//0x12
    EPD_5IN65F_BusyHigh();
    EPD_5IN65F_SendCommand(0x02);  //0x02
    EPD_5IN65F_BusyLow();
    DEV_Delay_ms(500);
}

/******************************************************************************
function :	show 7 kind of color block
parameter:
******************************************************************************/
void EPD_5IN65F_Show7Block(void)
{
    unsigned long i,j,k;
    unsigned char const Color_seven[8] =
	{EPD_5IN65F_BLACK,EPD_5IN65F_BLUE,EPD_5IN65F_GREEN,EPD_5IN65F_ORANGE,
	EPD_5IN65F_RED,EPD_5IN65F_YELLOW,EPD_5IN65F_WHITE,EPD_5IN65F_WHITE};
    EPD_5IN65F_SendCommand(0x61);//Set Resolution setting
    EPD_5IN65F_SendData(0x02);
    EPD_5IN65F_SendData(0x58);
    EPD_5IN65F_SendData(0x01);
    EPD_5IN65F_SendData(0xC0);
    EPD_5IN65F_SendCommand(0x10);

    for(i=0; i<224; i++) {
        for(k = 0 ; k < 4; k ++) {
            for(j = 0 ; j < 75; j ++) {
                EPD_5IN65F_SendData((Color_seven[k]<<4) |Color_seven[k]);
            }
        }
    }
    for(i=0; i<224; i++) {
        for(k = 4 ; k < 8; k ++) {
            for(j = 0 ; j < 75; j ++) {
                EPD_5IN65F_SendData((Color_seven[k]<<4) |Color_seven[k]);
            }
        }
    }
    EPD_5IN65F_SendCommand(0x04);//0x04
    EPD_5IN65F_BusyHigh();
    EPD_5IN65F_SendCommand(0x12);//0x12
    EPD_5IN65F_BusyHigh();
    EPD_5IN65F_SendCommand(0x02);  //0x02
    EPD_5IN65F_BusyLow();
	DEV_Delay_ms(200);
}

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void EPD_5IN65F_Display(const UBYTE *image)
{
    unsigned long i,j;
    EPD_5IN65F_SendCommand(0x61);//Set Resolution setting
    EPD_5IN65F_SendData(0x02);
    EPD_5IN65F_SendData(0x58);
    EPD_5IN65F_SendData(0x01);
    EPD_5IN65F_SendData(0xC0);
    EPD_5IN65F_SendCommand(0x10);
    for(i=0; i<EPD_5IN65F_HEIGHT; i++) {
        for(j=0; j<EPD_5IN65F_WIDTH/2; j++)
            EPD_5IN65F_SendData(image[j+((EPD_5IN65F_WIDTH/2)*i)]);
    }
    EPD_5IN65F_SendCommand(0x04);//0x04
    EPD_5IN65F_BusyHigh();
    EPD_5IN65F_SendCommand(0x12);//0x12
    EPD_5IN65F_BusyHigh();
    EPD_5IN65F_SendCommand(0x02);  //0x02
    EPD_5IN65F_BusyLow();
	DEV_Delay_ms(200);
	
}

/******************************************************************************
function :	Sends the part image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void EPD_5IN65F_Display_part(const UBYTE *image, UWORD xstart, UWORD ystart, 
																	UWORD image_width, UWORD image_heigh)
{
    unsigned long i,j;
    EPD_5IN65F_SendCommand(0x61);//Set Resolution setting
    EPD_5IN65F_SendData(0x02);
    EPD_5IN65F_SendData(0x58);
    EPD_5IN65F_SendData(0x01);
    EPD_5IN65F_SendData(0xC0);
    EPD_5IN65F_SendCommand(0x10);
    for(i=0; i<EPD_5IN65F_HEIGHT; i++) {
        for(j=0; j< EPD_5IN65F_WIDTH/2; j++) {
						if(i<image_heigh+ystart && i>=ystart && j<(image_width+xstart)/2 && j>=xstart/2) {
							EPD_5IN65F_SendData(image[(j-xstart/2) + (image_width/2*(i-ystart))]);
						}
						else {
							EPD_5IN65F_SendData(0x11);
						}
				}
    }
    EPD_5IN65F_SendCommand(0x04);//0x04
    EPD_5IN65F_BusyHigh();
    EPD_5IN65F_SendCommand(0x12);//0x12
    EPD_5IN65F_BusyHigh();
    EPD_5IN65F_SendCommand(0x02);  //0x02
    EPD_5IN65F_BusyLow();
	DEV_Delay_ms(200);
	
}

/******************************************************************************
function :	Enter sleep mode
parameter:
******************************************************************************/
void EPD_5IN65F_Sleep(void)
{
    DEV_Delay_ms(100);
    EPD_5IN65F_SendCommand(0x07);
    EPD_5IN65F_SendData(0xA5);
    DEV_Delay_ms(100);
	DEV_Digital_Write(EPD_RST_PIN, 0); // Reset
}

