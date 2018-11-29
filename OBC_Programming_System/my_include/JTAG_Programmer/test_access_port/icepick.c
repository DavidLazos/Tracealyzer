/*
 * icepick_tap.c
 *
 *  Created on: 13 de oct. de 2016
 *      Author: Fabio
 */

#include "icepick.h"

#include <stdint.h>

uint32_t icepick_IDcode(){

	move_to_state(TAP_IRSHIFT);
	shift_bits_in(ICE_IDCODE, ICEPICK_IR_LENGTH);

	move_to_state(TAP_DRSHIFT);
	return (uint32_t) shift_bits_in(0x0, ICEPICK_DR_LENGTH);

}

uint32_t icepick_icepickcode(){

	move_to_state(TAP_IRSHIFT);
	shift_bits_in(ICE_ICEPICKCODE, ICEPICK_IR_LENGTH);

	move_to_state(TAP_DRSHIFT);
	return (uint32_t) shift_bits_in(0x0, ICEPICK_DR_LENGTH);

}

/*
	TODO: This function should be separated into two or three new functions.
	Suggestion: connect_icepick();
				access_mapped_register(...);
				or...
				read_mapped_regiter(register);
				write_mapped_register(register, data);
	Additionally, this code should have an 'static' variable in order to save current ICEPick instruction.
	It shouldn't repeat ROUTER instruction between mapped register accesses.
*/
void select_tap(icepick_mapped_reg tap){

	uint32_t data = 0;

	/*	Writing the Debug Connect Register. CONNECTKEY field must be b'1001'
	 *	to put the ICEPick module in the connected state.	*/
	/*
		TODO: Debug CONNECT register write sequence should be in a separated function.
		This register should be written once. Activating secondary TAPs are independent from this sequence
	*/
	move_to_state(TAP_IRSHIFT);
	shift_bits_in(ICE_CONNECT, ICEPICK_IR_LENGTH);
	move_to_state(TAP_DRSHIFT);
	shift_bits_in(0x89, ICEPICK_DCON_LENGTH);

	/*
		TODO: ROUTER instruction sequence should be in a separated function.
		Data can be written or read using this instruction.
	*/
	/*	Accessing ICEPick mapped register - It must be through ROUTER instruction */
	move_to_state(TAP_IRSHIFT);
	shift_bits_in(ICE_ROUTER, ICEPICK_IR_LENGTH);
	move_to_state(TAP_DRSHIFT);
	data = (0x0 | (1 << 31) | (SYS_CNTL << 24) | (0x021000));	/*	Configuring SYS_CNTL mapped register. FIXME: Check 'data' value  */
	shift_bits_in(data, ICEPICK_DR_LENGTH);
	move_to_state(TAP_IDLE);
	move_to_state(TAP_DRSHIFT);
	data = 0;
	/*	This configuration is specifically for DAP TAP. 24-LSB may be different for another TAP.
	 *		bit[13]: Enable the Debug logic associated with DAP
	 *		bit[8]:	Select DAP to be on the JTAG scan chain
	 *		bit[3]: Force active power and clock to the logic associated with DAP.	*/
	data = (0x0 | (1 << 31) | (tap << 24) | (0x002108));		/* Configuring SDTAPx mapped register */
	shift_bits_in(data, ICEPICK_DR_LENGTH);
	move_to_state(TAP_IDLE);
	move_to_state(TAP_IRSHIFT);
	shift_bits_in(ICE_BYPASS, ICEPICK_IR_LENGTH);
	move_to_state(TAP_IDLE);

	/*	Idle time for DAP activation. */
	idle_time(1000);
	idle_clock_cycles(10);
	idle_time(1000);
}

