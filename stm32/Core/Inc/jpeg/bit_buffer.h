/**
 ******************************************************************************
 * @file      bit_buffer.h
 * @author    ts-manuel
 * @brief     Circular buffer used by the JPEG decoder
 *
 ******************************************************************************
 */

#ifndef INC_JPEG_BIT_BUFFER_H_
#define INC_JPEG_BIT_BUFFER_H_

#include <stdint.h>

#define _BIT_BUFF_DEPTH 32

typedef struct {
	uint8_t data[_BIT_BUFF_DEPTH];
	uint8_t write_byte_ptr;
	uint8_t read_byte_ptr;
	uint8_t read_bit_ptr;
    int size;
} BitBuffer_t;

void BB_Init(BitBuffer_t* buff);
void BB_PushByte(BitBuffer_t* buff, uint8_t byte);
int BB_Size(BitBuffer_t* buff);
uint16_t BB_ReadBits(BitBuffer_t* buff, uint8_t len);
uint16_t BB_Peek16(BitBuffer_t* buff);
void BB_DiscardBits(BitBuffer_t* buff, uint8_t len);
void BB_Align(BitBuffer_t* buff);

#endif /* INC_JPEG_BIT_BUFFER_H_ */
