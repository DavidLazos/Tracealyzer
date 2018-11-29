/*
 * It defines basic operations of JTAG.
 *
 * jtag.h
 *
 *  Created on: 3/6/2016
 *      Author: Fabio
 */

#ifndef MY_INCLUDE_JTAG_PROGRAMMER_JTAG_INTERFACE_JTAG_H_
#define MY_INCLUDE_JTAG_PROGRAMMER_JTAG_INTERFACE_JTAG_H_

#include <stdint.h>
#include "interface.h"

/**
 *	This function initializes the JTAG interface for bitbanging.
 */
void jtag_interface_init();

/*!
 *	This function performs a system reset.
 *	NOTE: after calling this function, all Debug Port configurations are deleted.
 */
void jtag_interface_reset();

/*!
 * 	This function performs "cycles" idle clock cycles in a stable state.
 *
 * 		\param idle_cycles. Number of clock cycles in idle state
 */
void idle_clock_cycles(uint32_t idle_cycles);

/*!
 * 	This function allows you move to a stable state from another stable state.
 *
 * 		\param to. This parameter indicates the end state. It has to be a stable state.
 *		FIXME: 	Should implement this: return false if it is not a stable state. True in another
 *				case.
 */
void move_to_state(tap_state_t to);

/*!
 * 	This function performs shift in bits to the TAP. It assumes that the current state is Shift-IR
 * 	or Shift-DR. After the operation that keeps in the same state.
 *
 * 		\param bits_sequence. The string of bytes that has to be shifted in.
 * 		\param bits_count. Number of bits to shift into the TAP.
 */
uint64_t shift_bits_in(uint64_t bits_sequence, uint8_t bits_count);

/*!
 * 	This function maintains inactive the interface a proportional time indicated by 'time'.
 *
 * 		\param time.
 */
void idle_time(int time);

#endif /* MY_INCLUDE_JTAG_PROGRAMMER_JTAG_INTERFACE_JTAG_H_ */
