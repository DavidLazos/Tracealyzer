/*
 * dap_tap.h
 *
 *  Created on: 15 de oct. de 2016
 *      Author: Fabio
 */

#ifndef MY_INCLUDE_JTAG_PROGRAMMER_TEST_ACCESS_PORT_DEBUG_PORT_H_
#define MY_INCLUDE_JTAG_PROGRAMMER_TEST_ACCESS_PORT_DEBUG_PORT_H_

#include <stdbool.h>
#include <stdint.h>
#include "interface.h"
#include "jtag.h"
#include "tap.h"

static const uint32_t debug_base_address = 0x80001000;

typedef enum core_debug_register{
		DBG_default = -1,
	DBGDIDR 	= 0x000,	/*<	CP14 c0, Debug ID Register	*/
	DBGWFAR		= 0x018,	/*< Watchpoint Fault Address Register	*/
	DBGVCR		= 0x01c,	/*< Vector Catch Register	*/
	DBGDSSCR	= 0x028,	/*< Debug State Cache Control Register	*/
	DBGDTRRX	= 0x080,	/*< Data Transfer Register	*/
	DBGITR		= 0x084,	/*< Instruction Transfer Register	*/
	DBGDSCR		= 0x088,	/*< CP14 c1, Debug Status and Control Register	*/
	DBGDTRTX	= 0x08c,	/*< Data Transfer Register	*/
	DBGDRCR		= 0x090,	/*< Debug Run Control Register	*/
	DBGBVR_0	= 0x100,	/*< Breakpoint Value Registers 0-7	*/
	DBGBVR_1	= 0x104,
	DBGBVR_2	= 0x108,
	DBGBVR_3	= 0x10c,
	DBGBVR_4	= 0x110,
	DBGBVR_5	= 0x114,
	DBGBVR_6	= 0x118,
	DBGBVR_7	= 0x11c,
	DBGBCR_0	= 0x140,	/*< Breakpoint Control Registers 0-7	*/
	DBGBCR_1	= 0x144,
	DBGBCR_2	= 0x148,
	DBGBCR_3	= 0x14c,
	DBGBCR_4	= 0x150,
	DBGBCR_5	= 0x154,
	DBGBCR_6	= 0x158,
	DBGBCR_7	= 0x15c,
	DBGOSLSR	= 0x304,	/*< Operating System Lock Status Register	*/
	DBGPRCR		= 0x310,	/*< Device Power-down and Reset Control Register	*/
	DBGPRSR		= 0x314,		/*< Device Power-down and Reset Status Register	*/

	/* 	NOTE: There are more registers than these. For more details see
	 * 	ARM DDI 0363G - Section 12.3.4	*/
	DBGLAR 		= 0xfb0

} core_debug_r;


typedef enum jtag_dp_register{
		DAP_rsvd,
	DAP_CTRL_STAT,
	DAP_SELECT,
	DAP_RDBUFF
} dp_register;

typedef enum ap_interface{
		AP_default = -1,
	AHB_AP	= 0x0,
	APB_AP	= 0x1
}ap_interface;

typedef enum ap_register{

	/*--- Bank 0 ---*/
	AP_CSW	=	0x0,
	AP_TAR	=	0x1,
		AP_rsvd	=	0x2,
	AP_DRW	=	0x3,

	/*--- Bank 15 ---*/
		AP_rsvd2	=	0x0,
	AP_CFG 	= 	0x1,
	AP_BASE	=	0x2,
	AP_IDR	=	0x3
} ap_register;

uint64_t access_debug_port_register(uint32_t data, dp_register dp_reg, access_t access_type);

uint64_t access_access_port_register(uint32_t data, ap_register ap_reg, access_t access_type);

#endif /* MY_INCLUDE_JTAG_PROGRAMMER_TEST_ACCESS_PORT_DEBUG_PORT_H_ */
