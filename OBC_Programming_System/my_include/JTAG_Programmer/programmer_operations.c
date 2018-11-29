/*
 * programmer_operations.c
 *
 *  Created on: 21 de set. de 2017
 *      Author: Fabio
 */

#include <programmer_operations.h>

int initProgramming()
{
	uint32_t firmware_text_size;
	uint32_t* firmware_text;
	uint32_t sram_text_address;
	uint32_t firmware_data_size;
	uint32_t* firmware_data;
	uint32_t sram_data_address;

	/*	It access to ICEPick TAP to enabling and configuring DAP TAP	*/
	debugger_init();
	halt_processor();

	/*	APB-AP configuration through DSCR debug register writing	*/
	configure_debug_apb_system(0x3086803);

	/* This function calling configures HW BKPT 0, for address matching at address 0x0	*/
	set_hardware_breakpoint(0, 0x0, 0x1e7);

	/*	Software reset routine	*/
	write_arm_register(0xffffffe0, reg0);		/* Write SYSECR address on reg0 */
	write_arm_register(0x8000, reg1);			/* It writes to reg1 the value to be stored on SYSECR */
	execute_arm_instruction(STR | (reg0 << 16) | (reg1 << 12) );		/* Execute instruction STR r1,[r0] */
	write_arm_register(0xffffffe4, reg0);		/* Write SYSESR address on reg0 */
	do
	{
		execute_arm_instruction(LDR | (reg0 << 16) | (reg1 << 12));		/* Execute LDR r1,[r0] */
	}while(!(read_arm_register(reg1) & 0x10 ));

	/* Clear reset bit on SYSESR */
	write_arm_register(0xffffffe4, reg0);				/* It writes SYSESR address in reg0 */
	write_arm_register(0xe038, reg1);					/* Escribe en reg1 el valor a escribir en SYSECR */
	execute_arm_instruction(STR | (reg0 << 16) | (reg1 << 12) );		/* Escribe instruccion STR r1,[r0] */

	/*	Poll DSCR.MOE, checking whether the HW BKPT hits or not	*/
	poll_debug_register(DBGDSCR, 0x3d, 0x5);

	/*	RAM initialization routine	*/
	write_arm_register(0xffffff5c, reg0);
	write_arm_register(0xa, reg1);
	execute_arm_instruction(STR | (reg0 << 16) | (reg1 <<12));		// Escribe instruccion STR r1,[r0]

	write_arm_register(0xffffff60, reg0);
	write_arm_register(0x1, reg1);
	execute_arm_instruction(STR | (reg0 << 16) | (reg1 <<12));		// Escribe instruccion STR r1,[r0]

	write_arm_register(0xffffff6c, reg0);
	do
	{
		execute_arm_instruction(LDR | (reg0 << 16) | (reg1 << 12));		/* Execute LDR r1,[r0] */
	}while(!(read_arm_register(reg1) & 0x1 ));

	/*	Writing firmware instructions onto SRAM	*/
	firmware_text_size = (tms570_fw_text_size >> 2);	// To words
	firmware_text = (uint32_t*) tms570_firmware;
	sram_text_address = tms570_fw_text_offset;
	while(firmware_text_size)
	{
		uint32_t chunk_size = (firmware_text_size > MAX_CHUNK_SIZE ? MAX_CHUNK_SIZE : firmware_text_size);

		/*	This function calling performs SRAM writing	*/
		write_sram_memory(sram_text_address, firmware_text, chunk_size);

		/*	Updating variables for next loop entry	*/
		firmware_text_size -= chunk_size;
		firmware_text += chunk_size;
		sram_text_address += (chunk_size << 2);
	}

	/*	Write firmware data onto SRAM.
	 *	IMPORTANT: Note that NOT all data values are 32-bit sized	*/
	firmware_data_size = (tms570_fw_data_size >> 2);
	firmware_data = (uint32_t*) tms570_fw_data;
	sram_data_address = tms570_fw_data_offset;
	while(firmware_data_size)
	{
		uint32_t chunk_size = (firmware_data_size > MAX_CHUNK_SIZE ? MAX_CHUNK_SIZE : firmware_data_size);

		if(!write_sram_memory(sram_data_address, firmware_data, chunk_size))
		{
			break;
		}

		/*	Updating variables for next loop entry	*/
		firmware_data_size -= chunk_size;
		firmware_data += chunk_size;
		sram_data_address += (chunk_size << 2);
	}

	write_arm_register(tms570_fw_flash_erasing, program_counter);
	write_arm_register(0x0, link_register);
	write_arm_register(0x08001500, stack_pointer);

	/*	Start executing 'flash_erase' firmware routine. It will enter to Debug state because of
	 * 	a BKPT	*/
	set_hardware_breakpoint(0, 0x0, 0x1e7);

	reset_processor();

	/*	Wait until a BKPT occurs	*/
	poll_debug_register(DBGDSCR, 0x3d, 0x5);

	/* 	It reads return value from firmware routine execution. If this value is not 0, it
	 * 	will return from main function because of an error	*/
	if(read_arm_register(reg0) != 0x0)
	{
		return 0;
	}
	return 1;
}


int flashProgramming(uint32_t* data, uint32_t chunk_size, uint32_t flash_data_address)
{
	/*	This function calling performs SRAM writing	*/
	write_sram_memory(data_buffer_offset, data, chunk_size>>2);

	/*	Updating variables for next loop entry	*/
	write_arm_register(tms570_fw_flash_programming, program_counter);
	write_arm_register(0x0, link_register);

	/*	Parameter passing	*/
	write_arm_register(flash_data_address, reg0);
	write_arm_register(data_buffer_offset, reg1);
	write_arm_register(chunk_size, reg2);

	/*	Reset processor and wait for a HW Breakpoint occurrence */
	reset_processor();
	poll_debug_register(DBGDSCR, 0x3d, 0x5);

	/*	Read returned value from flash_loader */
	if(read_arm_register(reg0) != 0x0)
	{
		return 0;
	}
	return 1;
}

void resetController()
{
	set_hardware_breakpoint(0,0,0);
	debugger_reset();
}
