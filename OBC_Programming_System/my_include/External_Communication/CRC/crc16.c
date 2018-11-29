/*
 * crc.c
 *
 *  Created on: 9 de nov. de 2016
 *      Author: Fabio
 */
#include "crc16.h"
static uint16_t crctable[256];

uint16_t* CalculateTable_CRC16(uint16_t generator)
{
    uint16_t divident;
	uint16_t curByte;
	uint8_t bit;

	/* iterate over all possible input byte values 0 - 255 */
    for (divident = 0; divident < 256; divident++) {
        curByte = (divident << 8); /* move divident byte into MSB of 16Bit CRC */

        for (bit = 0; bit < 8; bit++){
            if (curByte & 0x8000){
                curByte <<= 1;
                curByte ^= generator;
            }
            else
                curByte <<= 1;
        }
        crctable[divident] = curByte;
    }
    return (uint16_t *) &crctable[0];
}

uint16_t Compute_CRC16(uint8_t* data, uint32_t bytes_count, uint16_t* crctable){
    uint16_t crc = 0;
    uint8_t pos;

    while (bytes_count){
        /* XOR-in next input byte into MSB of crc, that's our new intermediate divident */
        pos = (uint8_t)( (crc >> 8) ^ (*data));
        /* Shift out the MSB used for division per lookuptable and XOR with the remainder */
        crc = (uint16_t)((crc << 8) ^ (uint16_t)(crctable[pos]));
		--bytes_count;
		++data;
    }
    return crc;
}
