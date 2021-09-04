/**
 ******************************************************************************
 * @file      file_manager.c
 * @author    ts-manuel
 * @brief     File Manager
 *
 ******************************************************************************
 */

#include "file_manager.h"

static void extract_folder_file(char* folder, char* file, const char* old_path);
static bool check_file(const char* folder, const char* file);
static bool check_folder(const char* folder);
static void increment_file_name(char* file);
static void find_next_folder(char* folder);
static void find_first_file(const char* folder, char* file);


/*
 * Find next file to display
 * */
bool FMAN_FindNext(char* new_path, const char* old_path)
{
	char folder[_MAX_LFN+1];
	char file[_MAX_LFN+1];
	bool change_folder = true;

	extract_folder_file(folder, file, old_path);


	//Check if the current folder still exists
	if(check_folder(folder))
	{
		//Increment file name
		increment_file_name(file);

		//Check if the next file name exists
		change_folder = !check_file(folder, file);
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

	sprintf(new_path, "%s/%s", folder, file);

	return check_file(folder, file);
}


/*
 *
 * */
static void extract_folder_file(char* folder, char* file, const char* old_path)
{
	int i = 0;

	//Extract folder
	while(old_path[i] != '\0' && old_path[i] != '/' && old_path[i] != '\\')
	{
		folder[i] = old_path[i];
		i++;
	}
	folder[i] = '\0';

	int j = i+1;
	i = 0;
	//Extract file
	while(old_path[i] != '\0')
	{
		file[i] = old_path[j];
		i++;
		j++;
	}
	file[i] = '\0';
}


/*
 * Check if the file exists
 * */
static bool check_file(const char* folder, const char* file)
{
	FRESULT fres;
	FILINFO fno;
	char path[(_MAX_LFN+1)*2 + 2];

	sprintf(path, "%s/%s", folder, file);

	fres = f_stat(path, &fno);

	return fres == FR_OK && !(fno.fattrib & AM_DIR);
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

	strcpy(file, "");

	//Open directory
	if(f_opendir(&dp, folder) != FR_OK)
	{
		return;
	}

	printf("Opening DIR: %s\n", folder);

	//Go through all the entries in the directory
	while(f_readdir(&dp, &fno) == FR_OK)
	{
		//When all directory entries have been read f_read returns a null string
		if(fno.fname[0] == '\0')
			break;

		//Check if new file name comes first
		if(strlen(file) ==  0 || strcmp(fno.fname, file) < 0)
		{
			strcpy(file, fno.fname);
		}
	}
}
