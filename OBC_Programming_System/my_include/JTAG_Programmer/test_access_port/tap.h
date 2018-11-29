/*
 * tap.h
 *
 *  Created on: 14 de oct. de 2016
 *      Author: Fabio
 */

#ifndef MY_INCLUDE_JTAG_PROGRAMMER_TEST_ACCESS_PORT_TAP_H_
#define MY_INCLUDE_JTAG_PROGRAMMER_TEST_ACCESS_PORT_TAP_H_

/*****************************************************
 **************		ICEPick C macros	**************
 ****************************************************/
/*-------------- ICEPick C registers size ----------*/
#define	ICEPICK_DR_LENGTH	32
#define ICEPICK_DCON_LENGTH	8
#define	ICEPICK_IR_LENGTH	6

/*----------- ICEPick C TAP instructions -----------*/
#define ICE_ROUTER		0x02
#define ICE_IDCODE		0x04
#define ICE_ICEPICKCODE	0x05
#define ICE_CONNECT		0x07
#define ICE_BYPASS		0x3f

/*****************************************************
 ****************		DAP macros		**************
 ****************************************************/
/*--------------- DAP TAP instructions -------------*/
#define DAP_ABORT	0x8
#define DAP_DPACC	0xa
#define DAP_APACC	0xb
#define DAP_IDCODE	0xe
#define DAP_BYPASS	0xf

/* IMPORTANT: This is for DAP accesses only */
typedef enum acces_type{
	WRITE,
	READ
}access_t;

#endif /* MY_INCLUDE_JTAG_PROGRAMMER_TEST_ACCESS_PORT_TAP_H_ */
