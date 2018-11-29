/*
 * interface.h
 *
 *  Created on: 1/6/2016
 *      Author: Fabio
 */

#ifndef MY_INCLUDE_JTAG_INTERFACE_H_
#define MY_INCLUDE_JTAG_INTERFACE_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum tap_state {
	TAP_INVALID = -1,

	/* Proper ARM recommended numbers */
	TAP_DREXIT2 = 0x0,
	TAP_DREXIT1 = 0x1,
	TAP_DRSHIFT = 0x2,
	TAP_DRPAUSE = 0x3,
	TAP_IRSELECT = 0x4,
	TAP_DRUPDATE = 0x5,
	TAP_DRCAPTURE = 0x6,
	TAP_DRSELECT = 0x7,
	TAP_IREXIT2 = 0x8,
	TAP_IREXIT1 = 0x9,
	TAP_IRSHIFT = 0xa,
	TAP_IRPAUSE = 0xb,
	TAP_IDLE = 0xc,
	TAP_IRUPDATE = 0xd,
	TAP_IRCAPTURE = 0xe,
	TAP_RESET = 0x0f,

} tap_state_t;


/** implementation of wrapper function tap_set_state() */
void tap_set_state_impl(tap_state_t new_state);

static inline void tap_set_state(tap_state_t new_state)
{
	tap_set_state_impl(new_state);
}

/**
 * This function gets the state of the "state follower" which tracks the
 * state of the TAPs connected to the cable. @see tap_set_state @return
 * tap_state_t The state the TAPs are in now.
 */
tap_state_t tap_get_state(void);

/**
 * This function sets the state of an "end state follower" which tracks
 * the state that any cable driver thinks will be the end (resultant)
 * state of the current TAP SIR or SDR operation.
 *
 * At completion of that TAP operation this value is copied into the
 * state follower via tap_set_state().
 *
 * @param new_end_state The state the TAPs should enter at completion of
 * a pending TAP operation.
 */
void tap_set_end_state(tap_state_t new_end_state);

/**
 * For more information, @see tap_set_end_state
 * @return tap_state_t - The state the TAPs should be in at completion of the current TAP operation.
 */
tap_state_t tap_get_end_state(void);

/**
 * This function provides a "bit sequence" indicating what has to be
 * done with TMS during a sequence of seven TAP clock cycles in order to
 * get from state \a "from" to state \a "to".
 *
 * The length of the sequence must be determined with a parallel call to
 * tap_get_tms_path_len().
 *
 * @param from The starting state.
 * @param to The desired final state.
 * @return int The required TMS bit sequence, with the first bit in the
 * sequence at bit 0.
 */
int tap_get_tms_path(tap_state_t from, tap_state_t to);

/**
 * Function int tap_get_tms_path_len
 * returns the total number of bits that represents a TMS path
 * transition as given by the function tap_get_tms_path().
 *
 * For at least one interface (JLink) it's not OK to simply "pad" TMS
 * sequences to fit a whole byte.  (I suspect this is a general TAP
 * problem within OOCD.) Padding TMS causes all manner of instability
 * that's not easily discovered.  Using this routine we can apply
 * EXACTLY the state transitions required to make something work - no
 * more - no less.
 *
 * @param from is the starting state
 * @param to is the resultant or final state
 * @return int - the total number of bits in a transition.
 */
int tap_get_tms_path_len(tap_state_t from, tap_state_t to);


/**
 * Function tap_move_ndx
 * when given a stable state, returns an index from 0-5.  The index corresponds to a
 * sequence of stable states which are given in this order: <p>
 * { TAP_RESET, TAP_IDLE, TAP_DRSHIFT, TAP_DRPAUSE, TAP_IRSHIFT, TAP_IRPAUSE }
 * <p>
 * This sequence corresponds to look up tables which are used in some of the
 * cable drivers.
 * @param astate is the stable state to find in the sequence.  If a non stable
 *  state is passed, this may cause the program to output an error message
 *  and terminate.
 * @return int - the array (or sequence) index as described above
 */
int tap_move_ndx(tap_state_t astate);

/**
 * Function tap_is_state_stable
 * returns true if the \a astate is stable.
 */
bool tap_is_state_stable(tap_state_t astate);

/**
 * Function tap_state_transition
 * takes a current TAP state and returns the next state according to the tms value.
 * @param current_state is the state of a TAP currently.
 * @param tms is either zero or non-zero, just like a real TMS line in a jtag interface.
 * @return tap_state_t - the next state a TAP would enter.
 */
tap_state_t tap_state_transition(tap_state_t current_state, bool tms);


/**
 * Represents a driver for a debugging interface.
 *
 * @TODO Rename; perhaps "debug_driver".  This isn't an interface,
 * it's a driver!  Also, not all drivers support JTAG.
 *
 * @TODO We need a per-instance structure too, and changes to pass
 * that structure to the driver.  Instances can for example be in
 * either SWD or JTAG modes.  This will help remove globals, and
 * eventually to cope with systems which have more than one such
 * debugging interface.
 */
struct jtag_interface {

	/**
	 * Set the interface speed.
	 * @param speed The new interface speed setting.
	 * @returns ERROR_OK on success, or an error code on failure.
	 */
	int (*speed)(int speed);

	/**
	 * Interface driver must initialize any resources and connect to a
	 * JTAG device.
	 *
	 * quit() is invoked if and only if init() succeeds. quit() is always
	 * invoked if init() succeeds. Same as malloc() + free(). Always
	 * invoke free() if malloc() succeeds and do not invoke free()
	 * otherwise.
	 *
	 * @returns ERROR_OK on success, or an error code on failure.
	 */
	int (*init)(void);

	/**
	 * Interface driver must tear down all resources and disconnect from
	 * the JTAG device.
	 *
	 * @returns ERROR_OK on success, or an error code on failure.
	 */
	int (*quit)(void);

	/**
	 * Read and clear the srst asserted detection flag.
	 *
	 * Like power_dropout this does *not* read the current
	 * state.  SRST assertion is transitionary and may be much
	 * less than 1ms, so the interface driver must watch for these
	 * events until this routine is called.
	 *
	 * @param srst_asserted On return, indicates whether SRST has
	 * been asserted.
	 * @returns ERROR_OK on success, or an error code on failure.
	 */
	//int (*srst_asserted)(int *srst_asserted);
};

extern struct jtag_interface *jtag_interface;

#endif /* MY_INCLUDE_JTAG_INTERFACE_H_ */
