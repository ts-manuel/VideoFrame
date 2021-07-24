/**
 ******************************************************************************
 * @file      flash.c
 * @author    ts-manuel
 * @brief     Flash
 *
 ******************************************************************************
 */

#include "hardware/flash.h"

#define _FLASH_STRT_ADDR	0x080e0000
#define _FLASH_STOP_ADDR	0x08100000

#define FLASH_READ(x) (*(uint8_t*)((x) + _FLASH_STRT_ADDR))

typedef struct
{
	char file_path[_FILE_PATH_MAX_LEN];	//File path string
	uint8_t padding[8];					//Padding to make the struct 32 bytes
	uint8_t magic;						//Magic number (0x5A = entry contains data)
	uint8_t checksum;					//Checksum for the entry
} FlashEntry_t;


static FlashEntry_t* FLASH_FindLastValidEntry(void);
static FlashEntry_t* FLASH_FindFirstEmptyEntry(void);
static uint8_t FLASH_ComputeChecksum(FlashEntry_t* entry);


/*
 * Read file path from flash
 * return false if no valid data is found
 * */
bool FLASH_LoadFilePath(char* file_path)
{
	FlashEntry_t* pt;
	file_path[0] = '\0';

	//Search for most recent valid entry
	pt = FLASH_FindLastValidEntry();
	if(pt == NULL)
		return false;

	//Copy file path
	strcpy(file_path, pt->file_path);

	//Return
	return true;
}


/*
 * Write file path to flash
 * */
void FLASH_StoreFilePath(const char* file_path)
{
	FlashEntry_t new_entity;
	FlashEntry_t* pt;

	//Check if there is space
	pt = FLASH_FindFirstEmptyEntry();

	if(pt == NULL)
	{
		//Erase sector
		FLASH_Erase();
		pt = (FlashEntry_t*)_FLASH_STRT_ADDR;
	}

	//Prepare entry
	strcpy(new_entity.file_path, file_path);
	new_entity.padding[0] = 0xff;
	new_entity.padding[1] = 0xff;
	new_entity.padding[2] = 0xff;
	new_entity.padding[3] = 0xff;
	new_entity.padding[4] = 0xff;
	new_entity.padding[5] = 0xff;
	new_entity.padding[6] = 0xff;
	new_entity.padding[7] = 0xff;
	new_entity.magic = 0x5a;
	new_entity.checksum = 0;
	new_entity.checksum = FLASH_ComputeChecksum(&new_entity);

	//Unlock flash memory
	HAL_FLASH_Unlock();

	//Write data to flash
	for(int i = 0; i < sizeof(FlashEntry_t); i++)
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, (uint32_t)pt + i, ((uint8_t*)(&new_entity))[i]);

	for(int i = 0; i < sizeof(FlashEntry_t); i++)
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, (uint32_t)pt + i, ((uint8_t*)(&new_entity))[i]);

	//Lock flash memory
	HAL_FLASH_Lock();
}


/*
 * Erase all entries from flash
 * */
void FLASH_Erase(void)
{
	HAL_FLASH_Unlock();
	FLASH_Erase_Sector(FLASH_SECTOR_11, FLASH_VOLTAGE_RANGE_1);
	HAL_FLASH_Lock();
}


/*
 * Finds the last entry in the flash
 * returns null if no valid entry is found
 * */
static FlashEntry_t* FLASH_FindLastValidEntry(void)
{
	FlashEntry_t* pt = (FlashEntry_t*)(_FLASH_STOP_ADDR - sizeof(FlashEntry_t));

	while(pt >= (FlashEntry_t*)_FLASH_STRT_ADDR)
	{
		//Check if the entry is valid (checksum is not computed if magic is invalid)
		if(pt->magic == 0x5A && FLASH_ComputeChecksum(pt) == 0)
			return pt;

		//Point to the previous entry
		pt --;
	}

	return NULL;
}


/*
 * Finds the address of the first empty entry
 * returns null if no valid entry is found
 * */
static FlashEntry_t* FLASH_FindFirstEmptyEntry(void)
{
	FlashEntry_t* pt = (FlashEntry_t*)_FLASH_STRT_ADDR;

	while(pt <= (FlashEntry_t*)(_FLASH_STOP_ADDR - sizeof(FlashEntry_t)))
	{
		//Check if the entry is empty (check magic first)
		if(pt->magic == 0xff)
		{
			uint8_t t = 0xff;

			for(int i = 0; i < sizeof(FlashEntry_t); i++)
				t &= ((uint8_t*)pt)[i];

			if(t == 0xff)
				return pt;
		}

		//Point to the next entry
		pt ++;
	}

	return NULL;
}



/*
 * Compute checksum for the entry
 * */
static uint8_t FLASH_ComputeChecksum(FlashEntry_t* entry)
{
	uint8_t sum = 0;

	for(int i = 0; i < sizeof(FlashEntry_t); i++)
		sum += ((uint8_t*)entry)[i];

	return ~sum+1;
}
