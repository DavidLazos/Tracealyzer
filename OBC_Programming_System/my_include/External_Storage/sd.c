/*
 * sd.c
 *
 *  Created on: 16 de dic. de 2016
 *      Author: Fabio
 */
#include <sd.h>

#define spi spiREG2

static spiDAT1_t datconfig_t;
static uint16_t tx_data[DATA_TRANSFER_LENGTH];
static uint16_t rx_data[DATA_TRANSFER_LENGTH];


bool SDInit()
{
	int i;

	sd_command_f command;
	response_f response;
	uint16_t init_data[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
							0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	spiInit();
	spiDisableLoopback(spi);

	delay(MAX_DELAY);
	/*	Power up. The host shall supply at least 74 clock cycles	*/
	/*	Keep CS high during at least 74 clock cycles	*/
	spiPORT2->DOUT |= 0x1;
	spiTransmitData(spi, &datconfig_t, 22, init_data);
	spiPORT2->DOUT &= 0xfffffffe;

	/*	Change CS pin mode from GPIO to SPISCS#	*/
	spi->PC0 |= 0x1;

	datconfig_t.CS_HOLD = true;
	datconfig_t.WDEL = false;
	datconfig_t.DFSEL = SPI_FMT_0;
	datconfig_t.CSNR = SPI_CS_0;

	delay(0x3000);

	/*	Send CMD0 command	*/
	command.start_bit = 0x0;
	command.transmission_bit = 0x1;
	command.command = CMD0;
	command.argument = 0x0U;
	command.crc = 0x4a;
	command.end_bit = 0x1;

	i = 0;
	do
	{
		response = SDSendCommand(command, COMMAND_R1_LENGTH, RESPONSE_1);
		++i;
	}while((response.r1.r1_content != 0x01) && (i < 10));

	if(response.r1.r1_content != 0x01)
	{
		return false;
	}

	/*	Send CMD8 command	*/
	command.command = CMD8;
	command.argument = 0x1aa;
	command.crc = 0x43;
	response = SDSendCommand(command, COMMAND_R7_LENGTH, RESPONSE_7);
	if((response.r1.r1_content != 0x01) || (response.addition.r7.r7_fields.check_pattern != 0xaa))
	{
		return false;
	}

	/*	Send CMD55/ACMD41 command	*/
	i = 0;
	do
	{
		command.command = CMD55;
		command.argument = 0x0U;
		command.crc = 0x32;
		response = SDSendCommand(command, COMMAND_R1_LENGTH, RESPONSE_1);
		if(response.r1.r1_content == 0x01)
		{
			command.command = ACMD41;
			command.argument = 0x40000000;
			command.crc = 0x72;
			response = SDSendCommand(command, COMMAND_R1_LENGTH, RESPONSE_1);
			if(response.r1.r1_content == 0x0)
			{
				return true;
			}
		}
		++i;
	}while((response.r1.r1_content != 0x0) && (i < 10));

	return false;
}

bool eraseBlocks(uint32_t first_block_address, uint32_t last_block_address)
{
	sd_command_f command;
	response_f response;

	/*	Send CMD32 command	*/
	command.start_bit = 0x0;
	command.transmission_bit = 0x1;
	command.command = CMD32;
	command.argument = first_block_address;
	command.crc = 0x7f;
	command.end_bit = 0x1;
	response = SDSendCommand(command, COMMAND_R1_LENGTH, RESPONSE_1);
	if (response.r1.r1_content != 0x0)
	{
		return false;
	}

	/*	Send CMD33 command	*/
	command.command = CMD33;
	command.argument = last_block_address;
	response = SDSendCommand(command, COMMAND_R1_LENGTH, RESPONSE_1);
	if (response.r1.r1_content != 0x0)
	{
		return false;
	}

	/*	Send CMD38 command	*/
	command.command = CMD38;
	command.argument = 0x0U;
	response = SDSendCommand(command, COMMAND_R2_LENGTH, RESPONSE_1B);
	if (response.r1.r1_content != 0x0)
	{
		return false;
	}

	while(SDIsBusy())
	{
		delay(MAX_DELAY);
	}

	/*	Check SD status	*/
	response = SDStatus();
	if(response.addition.r2.r2_content != 0x0)
	{
		return false;
	}

	return true;
}

uint16* readSingleDataBlock(uint32_t start_address)
{
	response_f response;
	sd_command_f command;

	/*	Send CMD17 command	*/
	command.start_bit = 0x0;
	command.transmission_bit = 0x1;
	command.command = CMD17;
	command.argument = start_address;
	command.crc = 0x7f;
	command.end_bit = 0x1;

	response = SDSendCommand(command, COMMAND_R1_LENGTH, RESPONSE_1);
	if(response.r1.r1_content != 0x0)
	{
		return NULL;
	}

	delay(MAX_DELAY);

	/*	Read data from SD	*/
	return SDReceiveData(DATA_TRANSFER_LENGTH);
}

bool writeSingleDataBlock(uint32_t start_address, uint16_t* data)
{
	sd_command_f command;
	response_f response;
	uint8_t data_resp_token;

	/*	Send CMD24 command	*/
	command.start_bit = 0x0;
	command.transmission_bit = 0x1;
	command.command = CMD24;
	command.argument = start_address;
	command.crc = 0x7f;
	command.end_bit = 0x1;

	commandToInteger16(command);
	dataForSent(data);

	response = SDSendData(DATA_TRANSFER_LENGTH, &data_resp_token);
	if((response.r1.r1_content != 0x0) || (data_resp_token != 0xe5))
	{
		return false;
	}

	while(SDIsBusy())
	{
		delay(MAX_DELAY);
	}

	/*	Check SD status	*/
	response = SDStatus();
	if(response.addition.r2.r2_content != 0x0)
	{
		return false;
	}

	return true;
}

bool setBlockLength(uint32_t blocklen)
{
	response_f response;
	sd_command_f command;

	/*	Send CMD16 command	*/
	command.start_bit = 0x0;
	command.transmission_bit = 0x1;
	command.command = CMD16;
	command.argument = blocklen;
	command.crc = 0x7f;
	command.end_bit = 0x1;

	response = SDSendCommand(command, COMMAND_R1_LENGTH, RESPONSE_1);

	return(response.r1.r1_content == 0x0 ? true : false);
}

uint32_t readOCR(){
	sd_command_f command;
	response_f response;

	/*	Send CMD16 command	*/
	command.start_bit = 0x0;
	command.transmission_bit = 0x1;
	command.command = CMD58;
	command.argument = 0x0;
	command.crc = 0x7e;
	command.end_bit = 0x1;

	response = SDSendCommand(command, COMMAND_R3_LENGTH, RESPONSE_3);

	return response.addition.r3.ocr;
}


/*******************************************************************************************************
 ********************************			Static functions 			********************************
 ******************************************************************************************************/

static void initTransmitionArray()
{
	memset(tx_data, 0xff, sizeof(tx_data));
}

static void delay(uint32_t value)
{
	uint32_t i;
	for(i=0; i<value; i++);
}

static void commandToInteger16(sd_command_f command)
{
	initTransmitionArray();

	tx_data[0] = (uint16_t) (0xff & ((command.start_bit << 7) | (command.transmission_bit << 6) | (command.command)));
	tx_data[1] = (uint16_t) (0xff & (command.argument >> 24));
	tx_data[2] = (uint16_t) (0xff & (command.argument >> 16));
	tx_data[3] = (uint16_t) (0xff & (command.argument >> 8));
	tx_data[4] = (uint16_t) (0xff & (command.argument));
	tx_data[5] = (uint16_t) (0xff & ((command.crc << 1) | (command.end_bit)));
}

static void dataForSent(uint16_t* data)
{
	/*	Data token for single block writing operations	*/
	tx_data [MAX_NCR + COMMAND_LENGTH + R1_LENGTH + MAX_NWR] = (uint16_t) 0xfe;

	/*	Copying data into tx_array	*/
	memcpy(tx_data + MAX_NCR + COMMAND_LENGTH + R1_LENGTH + MAX_NWR + DATA_TOKEN_LENGTH, data, DEFAULT_BLOCKLEN * 2);

	/*	These 2 bytes correspond to CRC fields for data	(CRC should be disabled)*/
	tx_data [MAX_NCR + COMMAND_LENGTH + R1_LENGTH + MAX_NWR + DATA_TOKEN_LENGTH + DEFAULT_BLOCKLEN] = (uint16_t) 0xaa;
	tx_data [MAX_NCR + COMMAND_LENGTH + R1_LENGTH + MAX_NWR + DATA_TOKEN_LENGTH + DEFAULT_BLOCKLEN + 1] = (uint16_t) 0xaa;
}

static bool SDIsBusy()
{
	uint16_t dummy_data;
	uint16_t busy_flag;

	dummy_data = 0xffff;
	spiTransmitAndReceiveData(spi, &datconfig_t, 1, &dummy_data, &busy_flag);

	return !busy_flag;
}

static response_f SDStatus()
{
	sd_command_f command;

	/*	Send CMD13 command	*/
	command.start_bit = 0x0;
	command.transmission_bit = 0x1;
	command.command = CMD13;
	command.argument = 0x0U;
	command.crc = 0x7f;
	command.end_bit = 0x1;

	return SDSendCommand(command, COMMAND_R2_LENGTH, RESPONSE_2);
}

static response_f SDSendCommand(sd_command_f command, uint32_t tx_length, response_type r_type)
{
	int i;
	response_f response;

	commandToInteger16(command);
	spiTransmitAndReceiveData(spi, &datconfig_t, tx_length, tx_data, rx_data);

	/*	Response assembling	*/
	i = 0;
	while((rx_data[i] == 0xff) && (i < tx_length))
	{
		i++;
	}

	if(i == tx_length)
	{
		memset(&response, 0xff, sizeof(response));
		return response;
	}

	response.r1.r1_content = 0xff & rx_data[i];
	switch(r_type)
	{
	case RESPONSE_1:
		break;
	case RESPONSE_1B:
	case RESPONSE_2:
		response.addition.r2.r2_content = 0xff & rx_data[i+1];
		break;
	case RESPONSE_3:
		response.addition.r3.ocr = (uint32_t)(((0xff & rx_data[i+1]) << 24)
										 |((0xff & rx_data[i+2]) << 16)
										 |((0xff & rx_data[i+3]) << 8)
										 |((0xff & rx_data[i+4])));
		break;
	case RESPONSE_7:
		response.addition.r7.r7_content = (uint32_t)(((0xff & rx_data[i+1]) << 24)
											 |((0xff & rx_data[i+2]) << 16)
											 |((0xff & rx_data[i+3]) << 8)
											 |((0xff & rx_data[i+4])));
		break;
	default:
		memset(&response, 0xff, sizeof(response));
		break;
	}

	return response;
}

static response_f SDSendData(uint32_t tx_length, uint8_t* data_response_token)
{
	int i;
	response_f response;

	spiTransmitAndReceiveData(spi, &datconfig_t, tx_length, tx_data, rx_data);

	/*	Looks for R1 response into rx_data array	*/
	i = 0;
	while((rx_data[i] == 0xff) &&  (i < tx_length))
	{
		++i;
	}

	if(i == tx_length)
	{
		memset(&response, 0xff, sizeof(response));
		return response;
	}
	response.r1.r1_content = 0xff & rx_data[i];

	/*	Looks for data response token into rx_data array	*/
	++i;
	while((rx_data[i] == 0xff) && (i < tx_length))
	{
		++i;
	}

	if(i == tx_length)
	{
		data_response_token = NULL;
	}
	else
	{
		*data_response_token = 0xff & rx_data[i];
	}

	return response;
}

static uint16_t* SDReceiveData(uint32_t tx_length)
{
	int i;

	if(tx_length < DATA_TRANSFER_LENGTH)
	{
		return NULL;
	}

	initTransmitionArray();
	spiTransmitAndReceiveData(spi, &datconfig_t, tx_length, tx_data, rx_data);

	/*	Looks for first received data into rx_data array	*/
	i = 0;
	while((rx_data[i] == 0xff) && (i < tx_length))
	{
		++i;
	}

	return (i < tx_length ? &rx_data[i+1] : NULL);
}

