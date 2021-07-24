/**
 ******************************************************************************
 * @file      settings.h
 * @author    ts-manuel
 * @brief     Global settings
 *
 ******************************************************************************
 */

#ifndef INC_SETTINGS_H_
#define INC_SETTINGS_H_

/* If the measured voltage is lower than the _MINIMUM_VOLTAGE
 * the current state is saved to FLASH and sleep mode is entered
 * */
#define _MINIMUM_VOLTAGE 4.0f

/*
 * Size of the serial port receive buffer,
 * maximum number of characters in a command
 * */
#define _MAX_CMD_LENGTH 64

/*
 * Number of seconds from the last command after witch
 * the micro enters low power mode
 * */
#define _SLEEP_TIMEOUT 60

/*
 * DO NOT max length of file path buffer
 * */
#define _FILE_PATH_MAX_LEN (8+1+8+1+3+1)

#endif /* INC_SETTINGS_H_ */
