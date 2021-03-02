/**
 ******************************************************************************
 * @file      file_manager.c
 * @author    ts-manuel
 * @brief     File Manager
 *
 ******************************************************************************
 */

#include "file_manager.h"


static bool check_folder(const char* folder);
static void increment_file_name(char* file);
static void find_next_folder(char* folder);
static void find_first_file(const char* folder, char* file);


/*
 * Check if the file exists
 * */
bool FMAN_CheckFile(const char* folder, const char* file)
{
	FRESULT fres;
	FILINFO fno;
	char path[(_MAX_LFN+1)*2 + 2];

	sprintf(path, "%s/%s", folder, file);

	fres = f_stat(path, &fno);

	return fres == FR_OK && !(fno.fattrib & AM_DIR);
}


/*
 * Find next file to display
 * */
bool FMAN_FindNext(char* folder, char* file)
{
	bool change_folder = true;

	//Check if the current folder still exists
	if(check_folder(folder))
	{
		//Increment file name
		increment_file_name(file);

		//Check if the next file name exists
		change_folder = !FMAN_CheckFile(folder, file);
	}

	//Change the current folder to the next available in alphabetical order
	if(change_folder)
	{
		int i = 0;
		do{
			//Find next folder in alphabetical order
			find_next_folder(folder);

			//Find first file in the new folder
			find_first_file(folder, file);

		} while(file[0] == '\0' && i++ < 16);
	}

	return FMAN_CheckFile(folder, file);
}


/*
 * Check if the folder exist in the root directory
 * */
static bool check_folder(const char* folder)
{
	FRESULT fres;
	FILINFO fno;

	fres = f_stat(folder, &fno);

	return fres == FR_OK && fno.fattrib & AM_DIR;
}


/*
 * Increment file name by one
 * */
static void increment_file_name(char* file)
{
	int len = strlen(file);
	bool inc = true;

	for(uint8_t i = len - 1; i < len; i--)
	{
		if(isdigit(file[i]) && inc)
		{
			file[i] += 1;
			inc = false;

			if(!isdigit(file[i])){
				file[i] = '0';
				inc = true;
			}
		}
	}
}


/*
 * Search the root directory for the next folder in alphabetical order
 * */
static void find_next_folder(char* folder)
{
	FRESULT fres;
	FILINFO fno;
	DIR dp;
	char first_folder[_MAX_LFN + 1];
	char next_folder[_MAX_LFN + 1];
	bool first_entry = true;
	bool found_next = false;

	//Open root directory
	fres = f_opendir(&dp, "");

	//Read all entries in the root directory
	do
	{
		//Read next entry
		fres = f_readdir(&dp, &fno);

		//Find the first folder name
		if((fno.fattrib & AM_DIR) && (*fno.fname != '\0') && (strcmp(fno.fname, first_folder) < 0 || first_entry))
		{
			first_entry = false;
			strcpy(first_folder, fno.fname);
		}

		//Find the next folder name
		if((fno.fattrib & AM_DIR) && (*fno.fname != '\0') && strcmp(fno.fname, folder) > 0)
		{
			if(strcmp(fno.fname, next_folder) < 0 || !found_next){
				strcpy(next_folder, fno.fname);
			}
			found_next = true;
		}


	} while(fres == FR_OK && *fno.fname != '\0');

	//Restart from first name if no other folder is found
	if(!found_next)
	{
		strcpy(folder, first_folder);
	}
	else
	{
		strcpy(folder, next_folder);
	}

	//Close directory
	fres = f_closedir(&dp);
}


/*
 * Find first file in the folder
 * */
static void find_first_file(const char* folder, char* file)
{
	FILINFO fno;
	DIR dp;
	bool error = false;

	//Open directory
	if(f_opendir(&dp, folder) != FR_OK)
	{
		error |= true;
	}

	//Read first entry
	if(f_readdir(&dp, &fno) != FR_OK)
	{
		error |= true;
	}

	//Copy file name
	strcpy(file, fno.fname);

	//Set all digits to 0
	int len = strlen(file);
	bool ext = true;
	int digit_count = 0;
	for(uint8_t i = len - 1; i < len; i--)
	{
		if(isdigit(file[i]))
		{
			file[i] = '0';
			digit_count ++;
		}
		else
		{
			if(ext && file[i] == '.')
			{
				ext = false;
			}
			else if(!ext)
			{
				error |= true;
			}
		}
	}

	if(digit_count == 0)
	{
		error |= true;
	}

	//Close directory
	f_closedir(&dp);

	//Return empty string if no file is found
	if(error)
	{
		strcpy(file, "");
	}
}