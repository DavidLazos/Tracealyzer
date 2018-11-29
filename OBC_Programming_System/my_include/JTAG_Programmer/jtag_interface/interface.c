/*
 * interface.c
 *
 *  Created on: 1/6/2016
 *      Author: Fabio
 */

#include "interface.h"

#include <stdlib.h>

static tap_state_t state_follower = TAP_RESET;

void tap_set_state_impl(tap_state_t new_state)
{
	/* this is the state we think the TAPs are in now, was cur_state */
	state_follower = new_state;
}

tap_state_t tap_get_state()
{
	return state_follower;
}

static tap_state_t end_state_follower = TAP_RESET;

void tap_set_end_state(tap_state_t new_end_state)
{
	/* this is the state we think the TAPs will be in at completion of the
	 * current TAP operation, was end_state
	*/
	end_state_follower = new_end_state;
}

tap_state_t tap_get_end_state()
{
	return end_state_follower;
}

int tap_move_ndx(tap_state_t astate)
{
	/* given a stable state, return the index into the tms_seqs[]
	 * array within tap_get_tms_path()
	 */

	int ndx;

	switch (astate) {
		case TAP_RESET:
			ndx = 0;
			break;
		case TAP_IDLE:
			ndx = 1;
			break;
		case TAP_DRSHIFT:
			ndx = 2;
			break;
		case TAP_DRPAUSE:
			ndx = 3;
			break;
		case TAP_IRSHIFT:
			ndx = 4;
			break;
		case TAP_IRPAUSE:
			ndx = 5;
			break;
		default:
			exit(1);
	}

	return ndx;
}

/* tap_move[i][j]: tap movement command to go from state i to state j
 * encodings of i and j are what tap_move_ndx() reports.
 *
 * DRSHIFT->DRSHIFT and IRSHIFT->IRSHIFT have to be caught in interface specific code
 */
struct tms_sequences {
	uint8_t bits;
	uint8_t bit_count;
};

/*
 * These macros allow us to specify TMS state transitions by bits rather than hex bytes.
 * Read the bits from LSBit first to MSBit last (right-to-left).
 */
#define HEX__(n) 0x##n##LU

#define B8__(x)	\
	((((x) & 0x0000000FLU) ? (1 << 0) : 0) \
	+(((x) & 0x000000F0LU) ? (1 << 1) : 0) \
	+(((x) & 0x00000F00LU) ? (1 << 2) : 0) \
	+(((x) & 0x0000F000LU) ? (1 << 3) : 0) \
	+(((x) & 0x000F0000LU) ? (1 << 4) : 0) \
	+(((x) & 0x00F00000LU) ? (1 << 5) : 0) \
	+(((x) & 0x0F000000LU) ? (1 << 6) : 0) \
	+(((x) & 0xF0000000LU) ? (1 << 7) : 0))

#define B8(bits, count) {((uint8_t)B8__(HEX__(bits))), (count)}

static const struct tms_sequences short_tms_seqs[6][6] = { /* [from_state_ndx][to_state_ndx] */
	/*
	state specific comments:
	------------------------
	*->RESET		tried the 5 bit reset and it gave me problems, 7 bits seems to
					work better on ARM9 with ft2232 driver.  (Dick)

	RESET->DRSHIFT add 1 extra clock cycles in the RESET state before advancing.
					needed on ARM9 with ft2232 driver.  (Dick)
					(For a total of *THREE* extra clocks in RESET; NOP.)

	RESET->IRSHIFT add 1 extra clock cycles in the RESET state before advancing.
					needed on ARM9 with ft2232 driver.  (Dick)
					(For a total of *TWO* extra clocks in RESET; NOP.)

	RESET->*		always adds one or more clocks in the target state,
					which should be NOPS; except shift states which (as
					noted above) add those clocks in RESET.

	The X-to-X transitions always add clocks; from *SHIFT, they go
	via IDLE and thus *DO HAVE SIDE EFFECTS* (capture and update).
*/

/* to state: */
/*	RESET		IDLE			DRSHIFT			DRPAUSE			IRSHIFT			IRPAUSE */ /* from state: */
{B8(1111111, 7), B8(0000000, 7), B8(0010111, 7), B8(0001010, 7), B8(0011011, 7), B8(0010110, 7)}, /* RESET */
{B8(1111111, 7), B8(0000000, 7), B8(001, 3),	 B8(0101, 4),	 B8(0011, 4),	 B8(01011, 5)}, /* IDLE */
{B8(1111111, 7), B8(011, 3),	 B8(00111, 5),	 B8(01, 2),		 B8(001111, 6),	 B8(0101111, 7)}, /* DRSHIFT */
{B8(1111111, 7), B8(011, 3),	 B8(01, 2),		 B8(0, 1),		 B8(001111, 6),	 B8(0101111, 7)}, /* DRPAUSE */
{B8(1111111, 7), B8(011, 3),	 B8(00111, 5),	 B8(010111, 6),	 B8(001111, 6),	 B8(01, 2)}, /* IRSHIFT */
{B8(1111111, 7), B8(011, 3),	 B8(00111, 5),	 B8(010111, 6),	 B8(01, 2),		 B8(0, 1)} /* IRPAUSE */
};

typedef const struct tms_sequences tms_table[6][6];

static tms_table *tms_seqs = &short_tms_seqs;

int tap_get_tms_path(tap_state_t from, tap_state_t to)
{
	return (*tms_seqs)[tap_move_ndx(from)][tap_move_ndx(to)].bits;
}

int tap_get_tms_path_len(tap_state_t from, tap_state_t to)
{
	return (*tms_seqs)[tap_move_ndx(from)][tap_move_ndx(to)].bit_count;
}

bool tap_is_state_stable(tap_state_t astate)
{
	bool is_stable;

	/*	A switch () is used because it is symbol dependent
	 * (not value dependent like an array), and can also check bounds.
	*/
	switch (astate) {
		case TAP_RESET:
		case TAP_IDLE:
		case TAP_DRSHIFT:
		case TAP_DRPAUSE:
		case TAP_IRSHIFT:
		case TAP_IRPAUSE:
			is_stable = true;
			break;
		default:
			is_stable = false;
	}

	return is_stable;
}

tap_state_t tap_state_transition(tap_state_t cur_state, bool tms)
{
	tap_state_t new_state;

	/*	A switch is used because it is symbol dependent and not value dependent
	 * like an array.  Also it can check for out of range conditions.
	*/

	if (tms) {
		switch (cur_state) {
			case TAP_RESET:
				new_state = cur_state;
				break;
			case TAP_IDLE:
			case TAP_DRUPDATE:
			case TAP_IRUPDATE:
				new_state = TAP_DRSELECT;
				break;
			case TAP_DRSELECT:
				new_state = TAP_IRSELECT;
				break;
			case TAP_DRCAPTURE:
			case TAP_DRSHIFT:
				new_state = TAP_DREXIT1;
				break;
			case TAP_DREXIT1:
			case TAP_DREXIT2:
				new_state = TAP_DRUPDATE;
				break;
			case TAP_DRPAUSE:
				new_state = TAP_DREXIT2;
				break;
			case TAP_IRSELECT:
				new_state = TAP_RESET;
				break;
			case TAP_IRCAPTURE:
			case TAP_IRSHIFT:
				new_state = TAP_IREXIT1;
				break;
			case TAP_IREXIT1:
			case TAP_IREXIT2:
				new_state = TAP_IRUPDATE;
				break;
			case TAP_IRPAUSE:
				new_state = TAP_IREXIT2;
				break;
			default:
				exit(1);
				break;
		}
	} else {
		switch (cur_state) {
			case TAP_RESET:
			case TAP_IDLE:
			case TAP_DRUPDATE:
			case TAP_IRUPDATE:
				new_state = TAP_IDLE;
				break;
			case TAP_DRSELECT:
				new_state = TAP_DRCAPTURE;
				break;
			case TAP_DRCAPTURE:
			case TAP_DRSHIFT:
			case TAP_DREXIT2:
				new_state = TAP_DRSHIFT;
				break;
			case TAP_DREXIT1:
			case TAP_DRPAUSE:
				new_state = TAP_DRPAUSE;
				break;
			case TAP_IRSELECT:
				new_state = TAP_IRCAPTURE;
				break;
			case TAP_IRCAPTURE:
			case TAP_IRSHIFT:
			case TAP_IREXIT2:
				new_state = TAP_IRSHIFT;
				break;
			case TAP_IREXIT1:
			case TAP_IRPAUSE:
				new_state = TAP_IRPAUSE;
				break;
			default:
				exit(1);
				break;
		}
	}

	return new_state;
}

extern struct jtag_interface tms570gpio_interface;

struct jtag_interface *jtag_interface = &tms570gpio_interface;
