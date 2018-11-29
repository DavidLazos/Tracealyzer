/*
 * programmer_operations.h
 *
 *  Created on: 21 de set. de 2017
 *      Author: Fabio
 */

#ifndef MY_INCLUDE_JTAG_PROGRAMMER_TEST_ACCESS_PORT_PROGRAMMER_OPERATIONS_H_
#define MY_INCLUDE_JTAG_PROGRAMMER_TEST_ACCESS_PORT_PROGRAMMER_OPERATIONS_H_

#include <stdint.h>
#include <debugger.h>
#include <flash_loader.h>

/*!
 * JTAG interface initialization sequence.
 *
 *	\return An integer value that indicates success(1) or error(0).
 * */
int initProgramming();

/*!
 *	Writes some data onto flash memory through JTAG interface.
 *
 *	\param data. A pointer to 32-bit data to be written.
 *	\param chunk_size. Amount of data to be written expressed in words (32-bit).
 *	\param flash_data_address. Address of first byte to be written in flash memory
 *	\return An integer value that indicates success or error.
 * */
int flashProgramming(uint32_t* data, uint32_t chunk_size, uint32_t flash_data_address);

void resetController();

#endif /* MY_INCLUDE_JTAG_PROGRAMMER_TEST_ACCESS_PORT_PROGRAMMER_OPERATIONS_H_ */
