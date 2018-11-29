/*
 * dap_tap.c
 *
 *  Created on: 15 de oct. de 2016
 *      Author: Fabio
 */

#include <debug_port.h>

static uint8_t current_instruction = 0x0;

uint64_t access_access_port_register(uint32_t data, ap_register ap_reg, access_t access_type){

	uint64_t retval = 0;

	if (current_instruction != DAP_APACC){
		move_to_state(TAP_IRSHIFT);
		shift_bits_in(0x3fb, 10);		// FIXME: corregir preamble
		current_instruction = DAP_APACC;
	}

	move_to_state(TAP_DRSHIFT);
	retval = shift_bits_in( (0xfffffffff & (((uint64_t)data << 3)|(ap_reg << 1)|(access_type << 0))), 36 );

	move_to_state(TAP_IDLE);

	idle_time(1000);		// FIXME: VER BIEN ESTE VALOR

	return (retval>>3); /* NOTE: This value does not return ACK value shifted-data[2:0]	*/

}

uint64_t access_debug_port_register(uint32_t data, dp_register dp_reg, access_t access_type){

	uint64_t retval = 0;

	if (current_instruction != DAP_DPACC){
		move_to_state(TAP_IRSHIFT);
		shift_bits_in(0x3fa, 10);		// FIXME: corregir preamble
		current_instruction = DAP_DPACC;
	}

	move_to_state(TAP_DRSHIFT);
	retval = shift_bits_in( (0xfffffffff & (((uint64_t)data << 3)|(dp_reg << 1)|(access_type << 0))) , 36 );

	move_to_state(TAP_IDLE);

	idle_time(1000);		//	FIXME: VER BIEN ESTE VALOR

	return (retval>>3); /* NOTE: This value does not return ACK value shifted-data[2:0]	*/
}
