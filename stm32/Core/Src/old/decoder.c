/**
 ******************************************************************************
 * @file      decoder.c
 * @author    ts-manuel
 * @brief     JPEG decoder
 *
 ******************************************************************************
 */

#include "decoder.h"


static const uint8_t zigZagMap[] = {
	0,   1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11, 4,   5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};

#if _GAMMA_CORRECT
static const uint8_t gamma_lut[256] = {
	     0,   0,   0,   0,  21,  24,  27,  29,  32,  34,  37,  39,  41,  43,  45,  47,
	    48,  50,  52,  54,  55,  57,  59,  60,  62,  63,  65,  66,  68,  69,  71,  72,
	    73,  75,  76,  77,  79,  80,  81,  83,  84,  85,  86,  88,  89,  90,  91,  92,
	    94,  95,  96,  97,  98,  99, 100, 102, 103, 104, 105, 106, 107, 108, 109, 110,
	   111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126,
	   127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 137, 138, 139, 140, 141,
	   142, 143, 144, 145, 145, 146, 147, 148, 149, 150, 151, 151, 152, 153, 154, 155,
	   156, 156, 157, 158, 159, 160, 161, 161, 162, 163, 164, 165, 165, 166, 167, 168,
	   169, 169, 170, 171, 172, 173, 173, 174, 175, 176, 176, 177, 178, 179, 179, 180,
	   181, 182, 182, 183, 184, 185, 185, 186, 187, 188, 188, 189, 190, 191, 191, 192,
	   193, 194, 194, 195, 196, 196, 197, 198, 199, 199, 200, 201, 201, 202, 203, 203,
	   204, 205, 206, 206, 207, 208, 208, 209, 210, 210, 211, 212, 212, 213, 214, 214,
	   215, 216, 216, 217, 218, 218, 219, 220, 220, 221, 222, 222, 223, 224, 224, 225,
	   226, 226, 227, 228, 228, 229, 230, 230, 231, 231, 232, 233, 233, 234, 235, 235,
	   236, 237, 237, 238, 238, 239, 240, 240, 241, 242, 242, 243, 243, 244, 245, 245,
	   246, 247, 247, 248, 248, 249, 250, 250, 251, 251, 252, 253, 253, 254, 254, 255,
};
#endif

// IDCT scaling factors
const float m0 = 2.f * cosf(1.f / 16.f * 2.f * M_PI);
const float m1 = 2.f * cosf(2.f / 16.f * 2.f * M_PI);
const float m3 = 2.f * cosf(2.f / 16.f * 2.f * M_PI);
const float m5 = 2.f * cosf(3.f / 16.f * 2.f * M_PI);
const float m2 = 2.f * cosf(1.f / 16.f * 2.f * M_PI) - 2.f * cosf(3.f / 16.f * 2.f * M_PI);
const float m4 = 2.f * cosf(1.f / 16.f * 2.f * M_PI) + 2.f * cosf(3.f / 16.f * 2.f * M_PI);
const float s0 = cosf(0.f / 16.f * M_PI) / sqrtf(8.f);
const float s1 = cosf(1.f / 16.f * M_PI) / 2.f;
const float s2 = cosf(2.f / 16.f * M_PI) / 2.f;
const float s3 = cosf(3.f / 16.f * M_PI) / 2.f;
const float s4 = cosf(4.f / 16.f * M_PI) / 2.f;
const float s5 = cosf(5.f / 16.f * M_PI) / 2.f;
const float s6 = cosf(6.f / 16.f * M_PI) / 2.f;
const float s7 = cosf(7.f / 16.f * M_PI) / 2.f;

static uint8_t read_byte(FIL* fp);
static uint16_t read_uint(FIL* fp);
static void init_jpg(JPG_t* jpg);
static void ReadAPPn(FIL* fp, JPG_t* jpg);
static void ReadDQT(FIL* fp, JPG_t* jpg);
static void ReadSOF0(FIL* fp, JPG_t* jpg);
static void ReadDRI(FIL* fp, JPG_t* jpg);
static void ReadDHT(FIL* fp, JPG_t* jpg);
static void ReadSOS(FIL* fp, JPG_t* jpg);
static void ReadComment(FIL* fp, JPG_t* jpg);
static bool decode_huffman(JPG_t* jpg, BitBuffer_t* buffer, bool terminate);
static void dequantize(JPG_t* jpg);
static void inverseDCT(JPG_t* jpg);
static void inverseDCT_component(int* component);
static void YCbCr_to_RGB(JPG_t* jpg);
#if (_DEBUG_PRINT > 0)
static void PrintHeader(JPG_t* jpg);
#endif

/*
 * Decode JPG file and send pixels to display
 * */
bool JPG_decode(FIL* fp, JPG_t* jpg)
{
	uint8_t byte0;
	uint8_t byte1;

	init_jpg(jpg);

	//Check SOI Marker
	byte0 = read_byte(fp);
	byte1 = read_byte(fp);
	if(byte0 != 0xff || byte1 != 0xd8)
	{
		jpg->valid = false;
		return false;
	}
	jpg->valid = true;

	//Read markers
	do
	{
		//Read next tow bytes
		byte0 = read_byte(fp);
		byte1 = read_byte(fp);

		//Exit with error if the end of file is reached before the End Of Image Marker
		if (f_eof(fp)) {
			printf("ERROR: File ended prematurely\n");
			jpg->valid = false;
			break;
		}

		//Exit with error if there is no valid marker (I expect to find a marker at every loop iteration)
		if (byte0 != 0xff)
		{
			printf("ERROR: Unable to find valid Marker\n");
			jpg->valid = false;
			break;
		}

		//Read Markers
		if (byte1 >= _APP0 && byte1 <= _APP15)
		{
			ReadAPPn(fp, jpg);
		}
		else if (byte1 == _DQT)
		{
			ReadDQT(fp, jpg);
		}
		else if (byte1 == _SOF0)
		{
			ReadSOF0(fp, jpg);
		}
		else if (byte1 == _DRI)
		{
			ReadDRI(fp, jpg);
		}
		else if (byte1 == _DHT)
		{
			ReadDHT(fp, jpg);
		}
		else if (byte1 == _SOS)
		{
			ReadSOS(fp, jpg);
			// break from while loop after SOS
			break;
		}
		else if (byte1 == _COM)
		{
			ReadComment(fp, jpg);
		}
		//Unused markers that can be skipped
		else if ((byte1 >= _JPG0 && byte1 <= _JPG13) || byte1 == _DNL || byte1 == _DHP || byte1 == _EXP)
		{
			ReadComment(fp, jpg);
		}
		else if (byte1 == _TEM)
		{
			// TEM has no size
		}
		//Any number of 0xff in a row is allowed and should be ignored
		else if (byte1 == 0xff)
		{
			byte1 = read_byte(fp);
			continue;
		}
		else if (byte1 == _SOI)
		{
			printf("ERROR: Embedded JPGs not supported\n");
			jpg->valid = false;
		}
		else if (byte1 == _EOI)
		{
			printf("ERROR: EOI detected before SOS\n");
			jpg->valid = false;
		}
		else if (byte1 == _DAC)
		{
			printf("ERROR: Arithmetic coding not supported\n");
			jpg->valid = false;
		}
		else if (byte1 >= _SOF0 && byte1 <= _SOF15)
		{
			printf("ERROR: SOS marker  not supported: 0x%02hhX\n", byte1);
			jpg->valid = false;
		}
		else if (byte1 >= _RST0 && byte1 <= _RST7)
		{
			printf("ERROR: RSTn detected before SOS\n");
			jpg->valid = false;
		}
		else
		{
			printf("ERROR: Unknown marker: 0x%02hhX\n", byte1);
			jpg->valid = false;
		}

	} while(jpg->valid);

	//Read Huffman data if jpg is still valid
	if(jpg->valid)
	{
		BitBuffer_t buffer;
		byte1 = read_byte(fp);

		//Initialize huffman data buffer
		BB_Init(&buffer);

		DISP_SetStripeHeight(jpg->verticalSamplingFactor * 8);

		//Read compressed image data
		while (jpg->valid)
		{
			if (f_eof(fp))
			{
				printf("ERROR: File ended prematurely");
				jpg->valid = false;
				break;
			}

			byte0 = byte1;
			byte1 = read_byte(fp);

			//If a marker is found
			if (byte0 == 0xff)
			{
				//0xff00 means put a literal 0xff in image data and ignore 0x00
				if (byte1 == 0x00)
				{
					BB_PushByte(&buffer, byte0);
					byte1 = read_byte(fp);
				}
				//Restart marker
				else if (byte1 >= _RST0 && byte1 <= _RST7)
				{
					byte1 = read_byte(fp);
				}
				//Ignore multiple0xff's in a row
				else if (byte1 == 0xff)
				{
					continue;
				}
				//End Of Image
				else if (byte1 == _EOI)
				{
					break;
				}
				else
				{
					printf("ERROR: Invalid marker during compressed data scan: 0x%02X\n", (int)byte1);
					jpg->valid = false;
					break;
				}
			}
			else
			{
				BB_PushByte(&buffer, byte0);
			}

			//Decode Huffman data from the buffer
			if(BB_Size(&buffer) > (_BIT_BUFF_DEPTH - 1) * 8)
				if(!decode_huffman(jpg, &buffer, false))
					jpg->valid = false;
		}
		if(!decode_huffman(jpg, &buffer, true))
			jpg->valid = false;
	}

#if (_DEBUG_PRINT > 0)
	//Print file header
	PrintHeader(jpg);
#endif

	return !jpg->valid;
}

/*
 * Read one byte from the file
 * */
static uint8_t read_byte(FIL* fp)
{
	uint8_t buff[1];
	UINT br;

	f_read(fp, (void*)buff, 1, &br);

	return buff[0];
}

/*
 * Read two bytes from the file
 * */
static uint16_t read_uint(FIL* fp)
{
	uint8_t buff[2];
	UINT br;

	f_read(fp, (void*)buff, 2, &br);

	return ((uint16_t)buff[0] << 8) + (uint16_t)buff[1];
}

/*
 * Initialize JPG_t struct
 * */
static void init_jpg(JPG_t* jpg)
{
	jpg->numComp = 0;
	jpg->restartInterval = 0;

	for(int i = 0; i < 4; i++)
	{
		jpg->QTables[i].used = false;
		jpg->HTablesAC[i].used = false;
		jpg->HTablesDC[i].used = false;
	}

	for(int i = 0; i < 3; i++)
	{
		jpg->colorComp[i].used = false;
	}

	jpg->decode.compNum = 0;
	jpg->decode.indx = 0;
	jpg->decode.previousDc[0] = 0;
	jpg->decode.previousDc[1] = 0;
	jpg->decode.previousDc[2] = 0;
	jpg->decode.mcu.x = 0;
	jpg->decode.mcu.y = 0;
	jpg->decode.blockCounter = 0;
}

/*
	Read APPn Markers
*/
static void ReadAPPn(FIL* fp, JPG_t* jpg)
{
	uint16_t length = read_uint(fp);

	//Discard data
	for (int i = 0; i < length - 2; i++)
	{
		read_byte(fp);
	}

#if (_DEBUG_PRINT > 1)
	printf("Reading APPn Marker\n");
#endif
}

/*
	Read Comment
*/
static void ReadComment(FIL* fp, JPG_t* jpg)
{
	uint16_t length = read_uint(fp);

	//Discard data
	for (int i = 0; i < length - 2; i++)
	{
		read_byte(fp);
	}

#if (_DEBUG_PRINT > 1)
	printf("Reading COM Marker\n");
#endif
}

/*
	Read Quantization tables
*/
static void ReadDQT(FIL* fp, JPG_t* jpg)
{
	int length = read_uint(fp) - 2;
#if (_DEBUG_PRINT > 1)
	printf("Reading DQT Marker\n");
#endif

	//Read all the tables
	while (length > 0)
	{
		uint8_t tableInfo =  read_byte(fp);
		uint8_t tableID = tableInfo & 0x0f;
		length -= 1;

		if (tableID > 3)
		{
			printf("ERROR: Invalid quantization table ID: %d", (int)tableID);
			jpg->valid = false;
			return;
		}

		//Read table data
		if (tableInfo >> 4)
		{
			//16 bit data
			for (int i = 0; i < 64; i++)
			{
				jpg->QTables[tableID].table[zigZagMap[i]] = read_uint(fp);
			}
			length -= 128;
		}
		else
		{
			//8 bit data
			for (int i = 0; i < 64; i++)
			{
				jpg->QTables[tableID].table[zigZagMap[i]] = (uint16_t)read_byte(fp);
			}
			length -= 64;
		}

		jpg->QTables[tableID].used = true;
	}

	if (length != 0)
	{
		printf("ERROR: DQT Invalid\n");
		jpg->valid = false;
		return;
	}
}

/*
	Read Start Of Frame
*/
static void ReadSOF0(FIL* fp, JPG_t* jpg)
{
	uint16_t length = read_uint(fp) - 2;
#if (_DEBUG_PRINT > 1)
	printf("Reading SOF Marker\n");
#endif
	//Precision must be 8
	uint8_t precision = read_byte(fp);
	if (precision != 8)
	{
		printf("ERROR: Invalid precision\n");
		jpg->valid = false;
		return;
	}

	//Read frame info
	jpg->heigth = read_uint(fp);
	jpg->width = read_uint(fp);

	jpg->numComp = read_byte(fp);
	if (jpg->numComp == 4)
	{
		printf("ERROR: CMYK color mode not supported\n");
		jpg->valid = false;
		return;
	}

	//Read components
	for (int i = 0; i < jpg->numComp; i++)
	{
		uint8_t componentID = read_byte(fp);
		uint8_t samplingFactor = read_byte(fp);
		uint8_t QTableID = read_byte(fp);

		//Component IDs are usually 1,2,3 but rarely can be seen as 0, 1, 2
		if (componentID == 4 || componentID == 5)
		{
			printf("ERROR: YIQ color mode not supported\n");
			jpg->valid = false;
			return;
		}
		if (componentID == 0 || componentID > 3)
		{
			printf("ERROR: Invalid component ID\n");
			jpg->valid = false;
			return;
		}

		RGB16_t* colorComp = &jpg->colorComp[componentID - 1];
		if (colorComp->used)
		{
			printf("ERROR: Duplicate color component ID\n");
			jpg->valid = false;
			return;
		}
		colorComp->used = true;

		//Sampling factors must be 1 for each colorComp channel
		uint8_t horizontalSamplingFactor = samplingFactor >> 4;
		uint8_t verticalSamplingFactor = samplingFactor & 0x0f;
		if(componentID == 1)
		{
			jpg->horizontalSamplingFactor = horizontalSamplingFactor;
			jpg->verticalSamplingFactor = verticalSamplingFactor;
		}
		else
		{
			if(horizontalSamplingFactor != 1 || verticalSamplingFactor != 1)
			{
				printf("ERROR: Sampling factors not supported\n");
				jpg->valid = false;
				return;
			}
		}

		if(QTableID > 3)
		{
			printf("ERROR: Invalid quantization table ID in frame components\n");
			jpg->valid = false;
			return;
		}
		colorComp->qTable = &jpg->QTables[QTableID];
	}

	//Compute number of MCUs
	uint16_t hMCUs = (jpg->width-1) / (8*jpg->horizontalSamplingFactor) + 1;
	uint16_t vMCUs = (jpg->heigth-1) / (8*jpg->verticalSamplingFactor) + 1;
	jpg->numMCUs = hMCUs * vMCUs;

	if (length - 6 - (3 * jpg->numComp) != 0)
	{
		printf("ERROR: SOF Invalid\n");
		jpg->valid = false;
		return;
	}
}

/*
	Read Restart Interval marker
*/
static void ReadDRI(FIL* fp, JPG_t* jpg)
{
	uint16_t length = read_uint(fp) - 2;
#if (_DEBUG_PRINT > 1)
	printf("Reading DRI Marker\n");
#endif
	jpg->restartInterval = read_uint(fp);

	if (length != 2)
	{
		printf("ERROR: DRI Invalid\n");
		jpg->valid = false;
		return;
	}
}

/*
	Read Huffman Tables
*/
static void ReadDHT(FIL* fp, JPG_t* jpg)
{
	int length = read_uint(fp) - 2;
#if (_DEBUG_PRINT > 1)
	printf("Reading DHT Marker\n");
#endif
	while (length > 0)
	{
		uint8_t tableInfo = read_byte(fp);
		uint8_t tableID = tableInfo & 0x0f;
		bool ACTable = tableInfo >> 4;

		if (tableID > 3)
		{
			printf("ERROR: Invalid Huffman Table ID: %d\n", (int)tableID);
			jpg->valid = false;
			return;
		}

		HTable_t* hTable = ACTable ? &jpg->HTablesAC[tableID] : &jpg->HTablesDC[tableID];
		hTable->used = true;
		uint8_t symbolCounter = 0;
		uint8_t symbolCount[16];

		//Read code lengths
		for (int i = 0; i < 16; i++)
		{
			symbolCount[i] = read_byte(fp);
			symbolCounter += symbolCount[i];
		}
		if (symbolCounter > 162)
		{
			printf("ERROR: Too many symbols in Huffman table");
			jpg->valid = false;
			return;
		}

		//Read symbols
		uint16_t code = 0;
		for(int i = 0; i < 16; i++)
		{
			//Loop for all symbols of length i+1
			for(int j = 0; j < symbolCount[i]; j++)
			{
				//Read symbol
				uint8_t symbol = read_byte(fp);

				//Fill lookup table
				if ((code << (15-i)) >= 0xfc00)
				{
					int start = (code << (15-i)) & 0x3ff;
					int stop = start | (0x3ff >> (i-5));
					for(int k = start; k <= stop; k++)
					{
						hTable->longLUT[k].symbol = symbol;
						hTable->longLUT[k].length = i+1;
					}
				}
				else
				{
					int start = code << (9-i);
					int stop = start | (0x3ff >> (i+1));

					//Check for codes that cant be stored
					if(start > 0x3ff)
					{
						printf("ERROR: Short code over 10bit\n");
						jpg->valid = false;
						return;
					}

					for(int k = start; k <= stop; k++)
					{
						hTable->shortLUT[k].symbol = symbol;
						hTable->shortLUT[k].length = i+1;
					}
				}

				//Generate new code
				code += 1;
			}
			code <<= 1;	//Append a 0 to the right
		}

		//Update length
		length -= 1 + 16 + symbolCounter;
	}

	if (length != 0)
	{
		printf("ERROR: DHT Invalid");
		jpg->valid = false;
		return;
	}
}

/*
	Read Start Of Scan
*/
static void ReadSOS(FIL* fp, JPG_t* jpg)
{
	uint16_t length = read_uint(fp) - 2;
#if (_DEBUG_PRINT > 1)
	printf("Reading SOS Marker\n");
#endif
	//SOF must precede SOS Marker
	if (jpg->numComp == 0)
	{
		printf("ERROR: SOS detected before SOF\n");
		jpg->valid = false;
		return;
	}

	for (int i = 0; i < jpg->numComp; i++)
	{
		jpg->colorComp[i].used = false;
	}

	uint8_t numComp = read_byte(fp);	//Must be the same in the SOF marker
	for (int i = 0; i < numComp; i++)
	{
		uint8_t componentID = read_byte(fp);
		if (componentID > jpg->numComp)
		{
			printf("EROR: Invalid component ID: %d\n", (int)componentID);
			jpg->valid = false;
			return;
		}

		RGB16_t* colorComp = &jpg->colorComp[componentID - 1];
		if (colorComp->used)
		{
			printf("ERROR: Duplicate colorComp component ID: %d\n", (int)componentID);
			jpg->valid = false;
			return;
		}
		colorComp->used = true;

		//Read Huffman Table IDs
		uint8_t hTableIDs = read_byte(fp);
		uint8_t HTableDCID = hTableIDs >> 4;
		uint8_t HTableACID = hTableIDs & 0x0f;
		if (HTableDCID > 3 || HTableACID > 3)
		{
			printf("ERROR: Invalid Huffman table ID");
			jpg->valid = false;
			return;
		}
		colorComp->hTableDC = &jpg->HTablesDC[HTableDCID];
		colorComp->hTableAC = &jpg->HTablesAC[HTableACID];
	}

	//Read block info
	uint8_t startOfSelection = read_byte(fp);
	uint8_t endOfSelection = read_byte(fp);
	uint8_t sucessiveAproximation = read_byte(fp);
	//Baseline JPGs don't use special selection or successive approximation
	if (startOfSelection != 0 || endOfSelection != 63)
	{
		printf("ERROR: Invalid spectral selection");
		jpg->valid = false;
		return;
	}
	if (sucessiveAproximation != 0)
	{
		printf("ERROR: Invalid successive approximation\n");
		jpg->valid = false;
		return;
	}

	length -= 1 + 2 * numComp + 3;
	if (length != 0)
	{
		printf("ERROR: SOS Invalid\n");
		jpg->valid = false;
		return;
	}
}

#if (_DEBUG_PRINT > 0)
/*
	Print the header content
*/
static void PrintHeader(JPG_t* jpg)
{
	if (!jpg)
	{
		return;
	}

#if (_DEBUG_PRINT > 1)
	printf("DQT============\n");
	for (int i = 0; i < 4; i++)
	{
		if (jpg->QTables[i].used)
		{
			printf("Table ID: %d\n", i);
			printf("Table Data:\n");
			for (int j = 0; j < 64; j++)
			{
				if (j % 8 == 0)
					printf("\n");
				printf("%4d ", jpg->QTables[i].table[j]);
			}
			printf("\n");
		}
	}


	printf("DHT============\n");
	printf("DC Tables\n");
	for (int i = 0; i < 4; i++)
	{
		HTable_t* hTable = &jpg->HTablesDC[i];
		if (hTable->used)
		{
			printf("Table ID: %d\n", i);
			printf("Symbols: \n");
			for (int j = 0; j < 16; j++)
			{
				printf("%2d: ", j + 1);
				for (int k = hTable->offsets[j]; k < hTable->offsets[j + 1]; k++)
				{
					printf("%02X ", (int)hTable->symbols[k]);
				}
				printf("\n");
			}
		}
	}
	printf("AC Tables\n");
	for (int i = 0; i < 4; i++)
	{
		HTable_t* hTable = &jpg->HTablesAC[i];
		if (hTable->used)
		{
			printf("Table ID: %d\n", i);
			printf("Symbols: \n");
			for (int j = 0; j < 16; j++)
			{
				printf("%2d: ", j + 1);
				for (int k = hTable->offsets[j]; k < hTable->offsets[j + 1]; k++)
				{
					printf("%02X ", (int)hTable->symbols[k]);
				}
				printf("\n");
			}
		}
	}
#endif

	printf("============\n");
	printf("Height: %d\n", (int)jpg->heigth);
	printf("Width:  %d\n", (int)jpg->width);
	printf("horizontal Sampling Factor: %d\n", (int)jpg->horizontalSamplingFactor);
	printf("vertical Sampling Factor:   %d\n", (int)jpg->verticalSamplingFactor);
	printf("Restart Interval: %d\n", (int)jpg->restartInterval);
}
#endif


/*
 * Decode Huffman data byte by byte as it gets read from the file,
 * decode MCUs as they get received
 * */
static bool decode_huffman(JPG_t* jpg, BitBuffer_t* buffer, bool terminate)
{

	//While there is enough data to read a full length code plus a full length coefficient
	//codes max length = 16 bit
	//DC coefficients max length = 11 bit
	//AC coefficients max length = 10 bit
	while((BB_Size(buffer) >= (16 + 11) || (terminate && BB_Size(buffer) > 0)) && jpg->decode.blockCounter < jpg->numMCUs){
		//Read code
		bool dc = (jpg->decode.indx % 64) == 0;
		HTable_t* hTable = dc ? jpg->colorComp[jpg->decode.compNum].hTableDC : jpg->colorComp[jpg->decode.compNum].hTableAC;
		uint16_t data = BB_Peek16(buffer);

		uint8_t code;
		if (data >= 0xfc00)
		{
			code = hTable->longLUT[data & 0x3ff].symbol;
			BB_DiscardBits(buffer, hTable->longLUT[data & 0x3ff].length);
		}
		else
		{
			code = hTable->shortLUT[(data >> 6) & 0x3ff].symbol;
			BB_DiscardBits(buffer, hTable->shortLUT[(data >> 6) & 0x3ff].length);
		}

		//Read coefficient
		uint8_t num_zeros = code  >> 4;
		uint8_t coeff_len = code & 0x0f;
		int* mcu_comp = &MCU_COMP(jpg->decode.mcu, jpg->decode.compNum)[jpg->decode.indx & ~0x3f];
		int coeff = BB_ReadBits(buffer, coeff_len);

		if(dc)
		{
			if (coeff_len != 0 && coeff < (1 << (coeff_len - 1)))
			{
				coeff -= (1 << coeff_len) - 1;
			}

			//Store DC coefficient
			coeff += jpg->decode.previousDc[jpg->decode.compNum];
			mcu_comp[jpg->decode.indx % 64] = coeff;
			jpg->decode.previousDc[jpg->decode.compNum] = coeff;
			jpg->decode.indx++;
		}
		else
		{
			//Symbol 0x00 means fill remainder of components with 0
			if (code == 0x00) {
				while((jpg->decode.indx % 64) != 0)
				{
					mcu_comp[zigZagMap[jpg->decode.indx % 64]] = 0;
					jpg->decode.indx++;
				}
			}
			else
			{
				//Symbol 0xf0 means skip 16 0's
				if (code == 0xf0)
				{
					num_zeros = 16;
				}

				//Insert zeros
				for (int i = 0; i < num_zeros; i++)
				{
					mcu_comp[zigZagMap[jpg->decode.indx % 64]] = 0;
					jpg->decode.indx++;
				}

				if (coeff_len != 0)
				{
					if (coeff < (1 << (coeff_len - 1)))
					{
						coeff -= (1 << coeff_len) - 1;
					}
					mcu_comp[zigZagMap[jpg->decode.indx % 64]] = coeff;
					jpg->decode.indx++;
				}
			}

			//Check for color component completion
			int max_index = jpg->decode.compNum == 0 ? 64 * (int)jpg->horizontalSamplingFactor * (int)jpg->verticalSamplingFactor : 64;
			if(jpg->decode.indx >= max_index)
			{
				jpg->decode.compNum++;
				jpg->decode.indx = 0;

				//If MCU is completed
				if(jpg->decode.compNum >= jpg->numComp)
				{
					dequantize(jpg);
					inverseDCT(jpg);
					YCbCr_to_RGB(jpg);

					//Handle restart intervals
					jpg->decode.blockCounter ++;
					if(jpg->restartInterval != 0 && (jpg->decode.blockCounter % jpg->restartInterval) == 0)
					{
						jpg->decode.previousDc[0] = 0;
						jpg->decode.previousDc[1] = 0;
						jpg->decode.previousDc[2] = 0;
						BB_Align(buffer);
					}

					//Increment color component and MCU start position
					jpg->decode.compNum = 0;
					jpg->decode.mcu.x += 8 * jpg->horizontalSamplingFactor;
					if(jpg->decode.mcu.x >= jpg->width)
					{
						jpg->decode.mcu.x = 0;
						jpg->decode.mcu.y += 8 * jpg->verticalSamplingFactor;
					}
				}
			}
		}
	}

	return true;
}


/*
 * Dequantize coefficients for all color channels
 * */
static void dequantize(JPG_t* jpg)
{
	//Dequantize luma
	for(int i = 0; i < jpg->horizontalSamplingFactor * jpg->verticalSamplingFactor; i++)
	{
		for (int j = 0; j < 64; j++)
		{
			jpg->decode.mcu.Y[i*64+j] *= jpg->colorComp[0].qTable->table[j];
		}
	}

	//Dequantize chroma
	if(jpg->numComp == 3)
	{
		for (int i = 0; i < 64; i++)
		{
			jpg->decode.mcu.Cb[i] *= jpg->colorComp[1].qTable->table[i];
			jpg->decode.mcu.Cr[i] *= jpg->colorComp[2].qTable->table[i];
		}
	}
}

/*
 * Compute the inverse cosine transform for all components
 * */
static void inverseDCT(JPG_t* jpg)
{
	//IDCT luma
	for(int i = 0; i < jpg->horizontalSamplingFactor * jpg->verticalSamplingFactor; i++)
	{
		inverseDCT_component(&MCU_COMP(jpg->decode.mcu, 0)[i*64]);
	}

	//IDCT chroma
	if(jpg->numComp == 3)
	{
		inverseDCT_component(MCU_COMP(jpg->decode.mcu, 1));
		inverseDCT_component(MCU_COMP(jpg->decode.mcu, 2));
	}
}

/*
 * Compute the inverse cosine transform for one component
 * using the AAN algorithm
 * */
static void inverseDCT_component(int* component)
{
	for(int i = 0; i < 8; i++)
	{
		const float g0 = component[0 * 8 + i] * s0;
		const float g1 = component[4 * 8 + i] * s4;
		const float g2 = component[2 * 8 + i] * s2;
		const float g3 = component[6 * 8 + i] * s6;
		const float g4 = component[5 * 8 + i] * s5;
		const float g5 = component[1 * 8 + i] * s1;
		const float g6 = component[7 * 8 + i] * s7;
		const float g7 = component[3 * 8 + i] * s3;

		const float f0 = g0;
		const float f1 = g1;
		const float f2 = g2;
		const float f3 = g3;
		const float f4 = g4 - g7;
		const float f5 = g5 + g6;
		const float f6 = g5 - g6;
		const float f7 = g4 + g7;

		const float e0 = f0;
		const float e1 = f1;
		const float e2 = f2 - f3;
		const float e3 = f2 + f3;
		const float e4 = f4;
		const float e5 = f5 - f7;
		const float e6 = f6;
		const float e7 = f5 + f7;
		const float e8 = f4 + f6;

		const float d0 = e0;
		const float d1 = e1;
		const float d2 = e2 * m1;
		const float d3 = e3;
		const float d4 = e4 * m2;
		const float d5 = e5 * m3;
		const float d6 = e6 * m4;
		const float d7 = e7;
		const float d8 = e8 * m5;

		const float c0 = d0 + d1;
		const float c1 = d0 - d1;
		const float c2 = d2 - d3;
		const float c3 = d3;
		const float c4 = d4 + d8;
		const float c5 = d5 + d7;
		const float c6 = d6 - d8;
		const float c7 = d7;
		const float c8 = c5 - c6;

		const float b0 = c0 + c3;
		const float b1 = c1 + c2;
		const float b2 = c1 - c2;
		const float b3 = c0 - c3;
		const float b4 = c4 - c8;
		const float b5 = c8;
		const float b6 = c6 - c7;
		const float b7 = c7;

		component[0 * 8 + i] = b0 + b7;
		component[1 * 8 + i] = b1 + b6;
		component[2 * 8 + i] = b2 + b5;
		component[3 * 8 + i] = b3 + b4;
		component[4 * 8 + i] = b3 - b4;
		component[5 * 8 + i] = b2 - b5;
		component[6 * 8 + i] = b1 - b6;
		component[7 * 8 + i] = b0 - b7;
	}
	for(int i = 0; i < 8; i++)
	{
		const float g0 = component[i * 8 + 0] * s0;
		const float g1 = component[i * 8 + 4] * s4;
		const float g2 = component[i * 8 + 2] * s2;
		const float g3 = component[i * 8 + 6] * s6;
		const float g4 = component[i * 8 + 5] * s5;
		const float g5 = component[i * 8 + 1] * s1;
		const float g6 = component[i * 8 + 7] * s7;
		const float g7 = component[i * 8 + 3] * s3;

		const float f0 = g0;
		const float f1 = g1;
		const float f2 = g2;
		const float f3 = g3;
		const float f4 = g4 - g7;
		const float f5 = g5 + g6;
		const float f6 = g5 - g6;
		const float f7 = g4 + g7;

		const float e0 = f0;
		const float e1 = f1;
		const float e2 = f2 - f3;
		const float e3 = f2 + f3;
		const float e4 = f4;
		const float e5 = f5 - f7;
		const float e6 = f6;
		const float e7 = f5 + f7;
		const float e8 = f4 + f6;

		const float d0 = e0;
		const float d1 = e1;
		const float d2 = e2 * m1;
		const float d3 = e3;
		const float d4 = e4 * m2;
		const float d5 = e5 * m3;
		const float d6 = e6 * m4;
		const float d7 = e7;
		const float d8 = e8 * m5;

		const float c0 = d0 + d1;
		const float c1 = d0 - d1;
		const float c2 = d2 - d3;
		const float c3 = d3;
		const float c4 = d4 + d8;
		const float c5 = d5 + d7;
		const float c6 = d6 - d8;
		const float c7 = d7;
		const float c8 = c5 - c6;

		const float b0 = c0 + c3;
		const float b1 = c1 + c2;
		const float b2 = c1 - c2;
		const float b3 = c0 - c3;
		const float b4 = c4 - c8;
		const float b5 = c8;
		const float b6 = c6 - c7;
		const float b7 = c7;

		component[i * 8 + 0] = b0 + b7;
		component[i * 8 + 1] = b1 + b6;
		component[i * 8 + 2] = b2 + b5;
		component[i * 8 + 3] = b3 + b4;
		component[i * 8 + 4] = b3 - b4;
		component[i * 8 + 5] = b2 - b5;
		component[i * 8 + 6] = b1 - b6;
		component[i * 8 + 7] = b0 - b7;
	}
}

/*
 * Write MCU pixels to display
 * */
static void YCbCr_to_RGB(JPG_t* jpg)
{
	for(int i = 0; i < jpg->verticalSamplingFactor; i++)
	{
		for(int j = 0; j < jpg->horizontalSamplingFactor; j++)
		{
			for(int y = 0; y < 8; y++)
			{
				for(int x = 0; x < 8; x++)
				{
					int img_x = jpg->decode.mcu.x + x + j * 8;
					int img_y = jpg->decode.mcu.y + y + i * 8;
					int r, g, b;

					if(img_x < jpg->width && img_y < jpg->heigth)
					{
						int mcu_Yindex = y*8+x + (i*jpg->horizontalSamplingFactor + j)*64;
						int Y = MCU_COMP(jpg->decode.mcu, 0)[mcu_Yindex];

						if(jpg->numComp == 3)
						{
							int cbcrPixelRow = y / jpg->verticalSamplingFactor + 4 * i;
							int cbcrPixelColumn = x / jpg->horizontalSamplingFactor + 4 * j;
							int cbcrPixel = cbcrPixelRow * 8 + cbcrPixelColumn;
							int Cb = MCU_COMP(jpg->decode.mcu, 1)[cbcrPixel];
							int Cr = MCU_COMP(jpg->decode.mcu, 2)[cbcrPixel];

							r = Y + 1.402f * Cr + 128;
							g = Y - 0.344f * Cb - 0.714f * Cr + 128;
							b = Y + 1.772f * Cb + 128;
						}
						else
						{
							r = Y + 128;
							g = Y + 128;
							b = Y + 128;
						}

						if(r < 0)	r = 0;
						if(r > 255)	r = 255;
						if(g < 0)	g = 0;
						if(g > 255)	g = 255;
						if(b < 0)	b = 0;
						if(b > 255)	b = 255;

#if _GAMMA_CORRECT
						r = gamma_lut[r];
						g = gamma_lut[g];
						b = gamma_lut[b];
#endif

						DISP_WritePixel(img_x, img_y, r, g, b);
					}
				}
			}
		}
	}
}
