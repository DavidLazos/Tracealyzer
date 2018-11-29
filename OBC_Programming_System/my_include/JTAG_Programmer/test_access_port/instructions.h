/*
 * instructions.h
 *
 *  Created on: 15 de oct. de 2016
 *      Author: Fabio
 */

#ifndef MY_INCLUDE_JTAG_PROGRAMMER_TEST_ACCESS_PORT_INSTRUCTIONS_H_
#define MY_INCLUDE_JTAG_PROGRAMMER_TEST_ACCESS_PORT_INSTRUCTIONS_H_

/*!
 * 	Load Register (register) calculates an address from a base register value and an offset
 * 	register value, loads a word from memory, and writes it to a register.
 * 	NOTE: It has to be used at form  0xe59<Rn><Rt>000 where Rn is the address base register
 * 		and Rt is the destination register.
 */
#define LDR		0xe5900000

/*!
 * 	Move to Coprocessor from ARM core register passes the value of an ARM core register to a
 * 	coprocessor.
 * 	NOTE: It has to be used at form  0xee00<Rt>e15
 */
#define MCR		0xee000e15

/*!
 *	Move (register) copies a value from a register to the destination register.
 *	NOTE: It has to be used at form  0xe1a0<Rd>00<Rm>
 */
#define MOV		0xe1a00000

/*!
 * 	Move to ARM core register from Coprocessor causes a coprocessor to transfer a value to an ARM
 * 	core register or to the condition flags.
 * 	NOTE: It has to be used at form  0xee10<Rt>e15
 */
#define MRC		0xee100e15

/*!
 * 	Push Multiple Registers stores multiple registers to the stack, storing to consecutive memory
 * 	locations ending just below the address in SP, and updates SP to point to the start of the
 * 	stored data.
 * 	NOTE: It has to be used at form 0xe52d<Rt>004 where Rt is the source register to be pushed to
 * 		the stack
 */
#define PUSH	0xe52d0004

/*!
 *	Store Register (register) calculates an address from a base register value and an offset
 *	register value, stores a word from a register to memory.
 *	NOTE: It has to be used at form 0xe58<Rn><Rt>000 where Rn is the base memory address and
 *		Rt the source register.
 */
#define STR		0xe5800000

#endif /* MY_INCLUDE_JTAG_PROGRAMMER_TEST_ACCESS_PORT_INSTRUCTIONS_H_ */
