/*
 * jtag.c
 *
 *  Created on: 3/6/2016
 *      Author: Fabio
 */

#include "jtag.h"
#include "bitbang.h"

#define CLOCK_IDLE()	0

void jtag_interface_init(){

	jtag_interface->init();
	jtag_interface->speed( 100 );

	move_to_state(TAP_RESET);
	idle_clock_cycles( 100 );
}

void jtag_interface_reset(){
	bitbang_interface->reset(0,1);
	idle_time(100);
	bitbang_interface->reset(0,0);
}

void idle_clock_cycles(uint32_t idle_cycles){
	int i;
	int tms = (tap_get_state() == TAP_RESET ? 1 : 0);

	for(i=0; i<idle_cycles; i++){
		bitbang_interface->write(1,tms,0);
		bitbang_interface->write(0,tms,0);
	}
}

void move_to_state(tap_state_t to){

	int i;
	/*	NOTE: These variables should be 8-bits sized	*/
	int tms = 0;
	int bits = tap_get_tms_path(tap_get_state(),to);
	int tms_count = tap_get_tms_path_len(tap_get_state(),to);

	for (i = 0; i < tms_count; i++) {
		tms = (bits >> i) & 1;
		bitbang_interface->write(0, tms, 0);
		bitbang_interface->write(1, tms, 0);
	}

	bitbang_interface->write(CLOCK_IDLE(), tms, 0);

	/*	TODO: This assignment should be done using tap_state_transition() after each TCK pulse	*/
	tap_set_state(to);
}

uint64_t shift_bits_in(uint64_t bits_sequence, uint8_t bits_count){

	int tdi;
	static int tms;
	tap_state_t cur_state = tap_get_state();
	int i;
	uint64_t rcvd_data = 0;

	/* Shitf bits in, and read shifted-out data */
	for (i = 0; i < bits_count; i++) {
		tdi = (bits_sequence >> i) & 1;
		tms = (i == (bits_count-1) ? 1 : 0);

		bitbang_interface->write(0, tms, tdi);
		if (bitbang_interface->read())
			rcvd_data |= (1LL << i);

		bitbang_interface->write(1, tms, tdi);
	}

	/* This sequence should takes FSM to IRPAUSE or DRPAUSE. We have to actualize it to the real state*/
	bitbang_interface->write(0, 0, tdi);
	bitbang_interface->write(1, 0, tdi);

	/*	TODO: This assignment should be done using tap_state_transition() after each TCK pulse.
	 * This would avoid using IF statement, and gives better control	*/
	tap_state_t new_state = (cur_state == TAP_IRSHIFT ? TAP_IRPAUSE : TAP_DRPAUSE);
	tap_set_state_impl(new_state);

	bitbang_interface->write(CLOCK_IDLE(), 0, tdi);

	return rcvd_data;
}

void idle_time(int time){

	int i;

	for( i=0; i<time; i++)
		asm volatile ("");
}

