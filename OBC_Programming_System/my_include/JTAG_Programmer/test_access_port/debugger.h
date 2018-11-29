/*
 * adi_v5_1.h
 *
 *  Created on: 18 de oct. de 2016
 *      Author: Fabio
 */

#ifndef MY_INCLUDE_JTAG_PROGRAMMER_TEST_ACCESS_PORT_DEBUGGER_H_
#define MY_INCLUDE_JTAG_PROGRAMMER_TEST_ACCESS_PORT_DEBUGGER_H_

#include <stdint.h>
#include "icepick.h"
#include "instructions.h"
#include "debug_port.h"

#define MAX_CHUNK_SIZE	256

/*!
 *
 */
typedef enum arm_core_register{
	reg0,
	reg1,
	reg2,
	reg3,
	reg4,
	reg5,
	reg6,
	reg7,
	reg8,
	reg9,
	reg10,
	reg11,
	reg12,
	stack_pointer,
	link_register,
	program_counter
} arm_core_r;

/*!
 * 	This function initializes the JTAG interface and performs the sequence of operations to add the
 * 	DAP TAP to the JTAG scan chain.
 * 	IMPORTANT: This function MUST be executed before any other debug operation. Otherwise none of
 * 	those will work.
 */
void debugger_init();

void debugger_reset();

/*!
 *	This function performs a CPU halt. NOTE: It must be issued before any DAP operation
 */
void halt_processor();

/*!
 *	This function performs a CPU reset.
 */
void reset_processor();

/*!
 *	This function set a hardware breakpoint writing the debug registers.
 *
 *		\param uint8_t bkpt_number. This value indicates which  of the 16 hardware breakpoints
 *			has to be set. NOTE: This parameter must be between 0 and 7.
 *		\param uint32_t bkpt_value.
 *		\param uint32_t bkpt_config. NOTE: A normal configuration value may be 0x1e7.
 *		\return bool. A boolean value that indicates if an error has occurred or not.
 */
bool set_hardware_breakpoint(uint8_t bkpt_number, uint32_t bkpt_value, uint32_t bkpt_config);

/*!
 *	This function writes a 32-bit value into an ARM core register.
 *
 *		\param	uint32_t data. Value to be written.
 *		\param	arm_core_r reg. Register to be written.
 *		\return bool. The status of the write operation.
 */
void write_arm_register(uint32_t data, arm_core_r reg);

/*!
 * This function reads the contents of an ARM core register.
 *
 *		\param arm_core_r reg. Register to be read.
 *		\return uint32_t. The value stored into ARM core register.
 */
uint32_t read_arm_register(arm_core_r reg);

/*!
 *	This function performs an ARM instruction through CPU.
 *
 *		 \param uint32_t instruction. The instruction to be performed.
 *		 \return bool. The status of the performed operation.
 */
void execute_arm_instruction(uint32_t instruction);

/*!
 * 	This function writes a chunk of 32-bit data onto the SRAM memory through AHB-AP.
 *
 * 		\param uint32_t address. The address on the first word to be written.
 * 		\param uint32_t* data. A pointer to the chunk of data.
 * 		\param uint16_t chunk_size. The size of the chunk of data to be written expressed in words
 * 			of 32 bits.
 * 			NOTE: chunk_size MUST NOT be greater than 256. This value is limited by auto-address
 * 			increment for AP_TAR register.
 * 		\return A boolean value that indicates if parameters are correct or not.
 */
bool write_sram_memory(uint32_t address, uint32_t* data, uint32_t chunk_size);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 											wrapper functions
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
inline void poll_debug_register(core_debug_r reg, uint32_t mask, uint32_t value);

inline void configure_debug_apb_system(uint32_t configuration);

inline void configure_dap_tap(uint32_t configuration);

inline void unlock_debug_registers();
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 									static functions definitions
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*!
 *
 */
static void write_cpu_debug_register(uint32_t data, core_debug_r debug_reg);

static uint32_t read_cpu_debug_register(core_debug_r debug_reg);

static void resource_access(uint32_t address, uint32_t* data, uint32_t data_size, access_t access_type);

static void configure_debug_port(uint32_t config_value);

static bool select_access_port(ap_interface ap, uint8_t bank);

static void select_and_configure_access_port(ap_interface ap, uint32_t conf_value);

//static uint32_t read_access_port_idr(ap_interface ap);

static uint32_t read_ctrl_stat_register();

#endif /* MY_INCLUDE_JTAG_PROGRAMMER_TEST_ACCESS_PORT_DEBUGGER_H_ */
