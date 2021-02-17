/**
 ******************************************************************************
 * @file      bit_buffer.c
 * @author    ts-manuel
 * @brief     Circular buffer used by the JPEG decoder
 *
 ******************************************************************************
 */

#include "bit_buffer.h"

/*
 * Initialize memory
 * */
void BB_Init(BitBuffer_t* buff)
{
	buff->write_byte_ptr = 0;
	buff->read_byte_ptr = 0;
	buff->read_bit_ptr = 0;
	buff->size = 0;
}

/*
 * Insert byte into the buffer
 * */
void BB_PushByte(BitBuffer_t* buff, uint8_t byte)
{
	buff->data[buff->write_byte_ptr] = byte;
	buff->write_byte_ptr = (buff->write_byte_ptr + 1) % _BIT_BUFF_DEPTH;
	buff->size += 8;
}

/*
 * Return number of bits in the buffer
 * */
int BB_Size(BitBuffer_t* buff)
{
	return buff->size;
}


/*
 * Read multiple bits from buffer
 * */
uint16_t BB_ReadBits(BitBuffer_t* buff, uint8_t len)
{
	uint16_t res = 0;

	res = (uint16_t)buff->data[buff->read_byte_ptr] << (buff->read_bit_ptr + 8);
	res |= (uint16_t)buff->data[(buff->read_byte_ptr+1)%_BIT_BUFF_DEPTH] << buff->read_bit_ptr;
	res |= (uint16_t)buff->data[(buff->read_byte_ptr+2)%_BIT_BUFF_DEPTH] >> (8 - buff->read_bit_ptr);

	res = BB_Peek16(buff) >> (16 - len);
	BB_DiscardBits(buff, len);

	return res;
}

/*
 * Read multiple bits from buffer without removing them from the buffer
 * */
uint16_t BB_Peek16(BitBuffer_t* buff)
{
	uint16_t res;

	res = (uint16_t)buff->data[buff->read_byte_ptr] << (buff->read_bit_ptr + 8);
	res |= (uint16_t)buff->data[(buff->read_byte_ptr+1)%_BIT_BUFF_DEPTH] << buff->read_bit_ptr;
	res |= (uint16_t)buff->data[(buff->read_byte_ptr+2)%_BIT_BUFF_DEPTH] >> (8 - buff->read_bit_ptr);

	return res;
}

/*
 * Removes multiple bits from the buffer
 * */
void BB_DiscardBits(BitBuffer_t* buff, uint8_t len)
{
	buff->read_byte_ptr = (buff->read_byte_ptr + (buff->read_bit_ptr + len) / 8) % _BIT_BUFF_DEPTH;
	buff->read_bit_ptr = (buff->read_bit_ptr + len) % 8;
	buff->size -= len;
}

/*
 * If there are bits remaining advance to the first bit of the next byte
 * */
void BB_Align(BitBuffer_t* buff)
{
	if (buff->read_bit_ptr != 0) {
		buff->size -= (int)(8 - buff->read_bit_ptr);
		buff->read_bit_ptr = 0;
		buff->read_byte_ptr = (buff->read_byte_ptr + 1) % _BIT_BUFF_DEPTH;
	}
}
