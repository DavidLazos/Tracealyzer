/*
 * icepick_tap.h
 *
 *  Created on: 13 de oct. de 2016
 *      Author: Fabio
 */

#ifndef MY_INCLUDE_JTAG_PROGRAMMER_TEST_ACCESS_PORT_ICEPICK_H_
#define MY_INCLUDE_JTAG_PROGRAMMER_TEST_ACCESS_PORT_ICEPICK_H_

#include <stdint.h>
#include "jtag.h"
#include "tap.h"

typedef enum icepick_mapped_register{
	SYS_CNTL	= 0x01,
	DAP			= 0x20,
	DMMRTP		= 0x21,	/*	Not implemented in this board	*/
	AJSM		= 0x22
}icepick_mapped_reg;

/*!
 * This function returns the content of the device identification register.
 */
uint32_t icepick_IDcode();

/*!
 *  This function returns the content of the ICEPICK code register.
 */
uint32_t icepick_icepickcode();

/*!
 *	This function enables the TAP to be acceded. It has to be performed
 *	before sending any command to the Debugging Unit.
 */
void select_tap(icepick_mapped_reg tap);

#endif /* MY_INCLUDE_JTAG_PROGRAMMER_TEST_ACCESS_PORT_ICEPICK_H_ */
