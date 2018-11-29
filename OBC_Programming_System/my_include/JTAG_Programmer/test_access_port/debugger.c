/*
 * adi_v5_1.c
 *
 *  Created on: 18 de oct. de 2016
 *      Author: Fabio
 */

#include <debugger.h>

void debugger_init(){

	/* 	Initialize JTAG interface. This function calling prepares GPIO port for bitbanging JTAG
	 * 	signals	*/
	jtag_interface_init();
	/*	ICEPick should enable other TAPs in order to add them to the scan chain	*/
	select_tap(DAP);
	/*	Power-up handshake. Bit[30] and bit[28] are System power-up request and Debug power-up
	 * 	request respectively. Bit[5], bit[4] and bit[1] clear sticky flags.
	 * 	TODO: Next function should include CTRL/STAT value polling, according to the steps in SPNA230.
	 * 	NOTE: For details see SPNA230 - Example 2.3.3	*/
	configure_debug_port(0x50000032);
	while((read_ctrl_stat_register() & 0xf0000000) >> 28 != 0xf);
}

void debugger_reset(){
	jtag_interface_reset();
}

void halt_processor(){

	/*	Write DRCR.Halt bit	*/
	write_cpu_debug_register(0x1, DBGDRCR);
	/*	Poll DSCR.Core_halted bit to determine when the processor has entered debug state	*/
	while( !(read_cpu_debug_register(DBGDSCR) & 0x1 ));
}

void reset_processor(){

	/*	Write DRCR.Halt bit	*/
	write_cpu_debug_register(0x2, DBGDRCR);
	/*	Poll DSCR.Core_halted bit to determine when the processor has entered debug state	*/
	while( !(read_cpu_debug_register(DBGDSCR) & 0x2));
}

bool set_hardware_breakpoint(uint8_t bkpt_number, uint32_t bkpt_value, uint32_t bkpt_config){

	core_debug_r value;
	core_debug_r config;

	/*	Check if the parameter is valid	*/
	if(bkpt_number > 0x7)
		return false;

	/*	Set the pair of registers to be written	*/
	switch(bkpt_number){
	case 0:
		value = DBGBVR_0;
		config = DBGBCR_0;
		break;
	case 1:
		value = DBGBVR_1;
		config = DBGBCR_1;
		break;
	case 2:
		value = DBGBVR_2;
		config = DBGBCR_2;
		break;
	case 3:
		value = DBGBVR_3;
		config = DBGBCR_3;
		break;
	case 4:
		value = DBGBVR_4;
		config = DBGBCR_4;
		break;
	case 5:
		value = DBGBVR_5;
		config = DBGBCR_5;
		break;
	case 6:
		value = DBGBVR_6;
		config = DBGBCR_6;
		break;
	case 7:
		value = DBGBVR_7;
		config = DBGBCR_7;
		break;
	default:
		value = DBGBVR_0;
		config = DBGBCR_0;
		break;
	}

	/*	Write the pair of registers for a hardware breakpoint configuration	*/
	write_cpu_debug_register(0x0, config);
	write_cpu_debug_register(bkpt_value, value);
	write_cpu_debug_register(bkpt_config, config);

	return true;
}

void write_arm_register(uint32_t data, arm_core_r reg){

	/*	NOTE: 'Program counter' register can not be written directly, but must be written through
	 * 	another core register. For this particular case, we use register 0	*/
	if (reg == program_counter){
		/*	Recursive calling to store data in a intermediary register	*/
		write_arm_register(data, reg0);
		/*	After 'reg0' has been written, MOV r15, r0 instruction should be executed	*/
		execute_arm_instruction( MOV | reg << 12 | reg0 << 0);

		return;
	}
	/*	Move data to DTRRX debug register	*/
	write_cpu_debug_register(data, DBGDTRRX);
	/*	Check for DSCR.DTRRXFull flag	*/
	while( !(read_cpu_debug_register(DBGDSCR) & (1 << 30)));
	/*	Move DTRRX content to the specified core register	*/
	execute_arm_instruction( MRC | reg << 12 );

	return;
}

uint32_t read_arm_register(arm_core_r reg){

	/*	TODO: Must implement special case for Program counter */

	/*	It should execute MCR instruction in order to move 'reg' content to the DBGDTRTX debug
	 * 	register*/
	execute_arm_instruction( MCR | reg << 12 );
	/*	Active waiting until DSCR.DTRTXFull == 1	*/
	while( !(read_cpu_debug_register(DBGDSCR) & (1 << 29)) );
	/*	Read and return the content of DTRTX	*/
	return read_cpu_debug_register(DBGDTRTX);
}

void execute_arm_instruction(uint32_t instruction){

	/*	Write into DBGITR debug register the ARM instruction to be performed by the CPU	*/
	write_cpu_debug_register(instruction, DBGITR);
	/*	Poll DSCR.InstrCompl == 1	*/
	while( !(read_cpu_debug_register(DBGDSCR) & (1 << 24)) );
}

bool write_sram_memory(uint32_t address, uint32_t* data, uint32_t chunk_size){

	/*	Checks maximum chunk size allowed for AHB-AP transactions and SRAM limits	*/
	if((chunk_size > MAX_CHUNK_SIZE) || (address < 0x8000000) ||
			((address + chunk_size*4) > 0x0803FFFF))
		return false;

	/*	If checking pass, then write 'chunk_size' of data in SRAM memory, starting at specified
	 * 	'address'*/
	resource_access(address, data, chunk_size, WRITE);
	return true;
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 										Wrapper functions
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
inline void poll_debug_register(core_debug_r reg, uint32_t mask, uint32_t value){
	uint32_t rcvd_value = 0;
	do{
		rcvd_value = read_cpu_debug_register(reg);
	}while((rcvd_value & mask) != value);
}

inline void configure_debug_apb_system(uint32_t configuration){
	write_cpu_debug_register(configuration, DBGDSCR);
}

inline void configure_dap_tap(uint32_t configuration){
	configure_debug_port(configuration);
}


/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 						static functions that implements DAP basic operations
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static core_debug_r current_debug_register = DBG_default;

static void write_cpu_debug_register(uint32_t data, core_debug_r debug_reg){

	/*	These operations must be executed through APB-AP interface	*/
	select_access_port(APB_AP, 0x0);

	access_access_port_register((debug_base_address + debug_reg), AP_TAR, WRITE);
	current_debug_register = debug_reg;
	access_access_port_register(data, AP_DRW, WRITE);
	/*	Read DP registers	*/
	read_ctrl_stat_register();

}

static uint32_t read_cpu_debug_register(core_debug_r debug_reg){

	uint64_t rcvd_value = 0;

	/*	These operations must be executed through APB-AP interface	*/
	select_access_port(APB_AP, 0x0);

	if(current_debug_register != debug_reg){
		access_access_port_register((debug_base_address | debug_reg), AP_TAR, WRITE);
		current_debug_register = debug_reg;
	}
	access_access_port_register(0x0, AP_DRW, READ);

	/*	Read DP registers. It has not to be replaced for 'read_ctrl_stat_register' because that does not
	 * 	return AP read value	*/
	rcvd_value = access_debug_port_register(0x0, DAP_CTRL_STAT, READ);
	access_debug_port_register(0x0, DAP_RDBUFF, READ);

	return (uint32_t) (0xffffffff & rcvd_value);

}

static void resource_access(uint32_t address, uint32_t* data, uint32_t data_size, access_t access_type){

	uint64_t rcvd_value = 0;
	uint32_t i;

	/*	Check if the current Access Port is AHB-AP and bank is 0. Otherwise, they should be
	 * 	selected and configured. This configuration value enables 32-bit transaction size,
	 * 	address increment mode...	*/
	select_and_configure_access_port(AHB_AP, 0x43000012);

	/*	Starting access to the selected memory address
	 * 	IMPORTANT: Check if 10 LSB of auto-incremented address in TAR will not rebase 1Kb */
	access_access_port_register(address, AP_TAR, WRITE);
	read_ctrl_stat_register();
	switch(access_type){
	case WRITE:
		for(i=0; i<data_size; i++){
			/*	Write data at specified address	*/
			access_access_port_register(data[i], AP_DRW, WRITE);
		}
		/*	Read DP registers	*/
		read_ctrl_stat_register();
		break;
	case READ:
		for(i=0; i<data_size; i++){
			/*	Read data from specified address	*/
			access_access_port_register(0x0, AP_DRW, READ);
			/*	Read DP registers	*/
			rcvd_value = access_debug_port_register(0x0, DAP_CTRL_STAT, READ);
			access_debug_port_register(0x0, DAP_RDBUFF, READ);
			data[i] = (uint32_t)(0xffffffff & rcvd_value);
		}
		break;
	default:
		break;
	}

}

static void configure_debug_port(uint32_t config_value){

	/*	Write CTRL/STAT	register	*/
	access_debug_port_register(config_value, DAP_CTRL_STAT, WRITE);
	/*	Read DP registers	*/
	read_ctrl_stat_register();

}

static bool select_access_port(ap_interface ap, uint8_t bank){

	/* 	These variables store the current AP state
	 * 	IMPORTANT: They MUST be reset to default values after a system reset. Otherwise they won't
	 * 	reflect the real state	of the Debug Access Port	*/
	static ap_interface current_ap = AP_default;
	static uint8_t current_bank = 0xff;

	if ((bank != 0x0) && (bank != 0xf))
		return false;

	if((current_ap != ap) || (current_bank != bank)){
		/*	Configure JTAG-DP SELECT register to enable an AP and an specific bank for that AP	*/
		access_debug_port_register((0x0 | ap << 24 | bank << 4), DAP_SELECT, WRITE);
		/*	Read DP registers	*/
		read_ctrl_stat_register();
		/*	It should change the global variable 'current_ap' and 'current_bank' in order to keep
		 * 	updated the current access port and bank enabled	*/
		current_ap = ap;
		current_bank = bank;
	}
	return true;

}

static void select_and_configure_access_port(ap_interface ap, uint32_t conf_value){

	/*	This function call selects an AP and bank 0. After that, this AP will be configured
	 * 	writing CSW register	*/
	select_access_port(ap, 0x0);
	access_access_port_register(conf_value, AP_CSW, WRITE);
	/*	Read DP registers	*/
	read_ctrl_stat_register();

}

static uint32_t read_ctrl_stat_register(){

	uint64_t ctrl_stat = 0;

	access_debug_port_register(0x0, DAP_CTRL_STAT, READ);
	ctrl_stat = access_debug_port_register(0x0, DAP_RDBUFF, READ);

	return (uint32_t)(0xffffffff & ctrl_stat);

}

//static uint32_t read_access_port_idr(ap_interface ap){
//
//	uint64_t idr_register = 0;
//
//	select_access_port(ap, 0xf);
//	access_access_port_register(0x0, AP_IDR, READ);
//
//	/*	Read DP registers. It has not to be replaced for 'read_ctrl_stat_register' because that does not
//	 * 	return AP read value	*/
//	idr_register = access_debug_port_register(0x0, DAP_CTRL_STAT, READ);
//	access_debug_port_register(0x0, DAP_RDBUFF, READ);
//
//	return (uint32_t) (0xffffffff & idr_register);
//
//}
