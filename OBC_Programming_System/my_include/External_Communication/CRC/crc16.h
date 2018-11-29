/*
 * crc.h
 *
 *  Created on: 9 de nov. de 2016
 *      Author: Fabio
 */

#ifndef MY_INCLUDE_CRC_CRC16_H_
#define MY_INCLUDE_CRC_CRC16_H_

#include <stdio.h>
#include <stdint.h>

uint16_t* CalculateTable_CRC16(uint16_t generator);
uint16_t Compute_CRC16(uint8_t* data, uint32_t bytes_count, uint16_t* crctable);

#endif /* MY_INCLUDE_CRC_CRC16_H_ */
