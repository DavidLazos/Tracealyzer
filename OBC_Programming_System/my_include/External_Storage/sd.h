/*
 * sd.h
 *
 *  Created on: 16 de dic. de 2016
 *      Author: Fabio
 */

#ifndef MY_INCLUDE_SD_H_
#define MY_INCLUDE_SD_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <gio.h>
#include <spi.h>
#include <sci.h>

#define DEFAULT_BLOCKLEN	512
#define DATA_CRC_LENGTH		2
#define DATA_TOKEN_LENGTH	1
#define COMMAND_LENGTH		6
#define DEFAULT_DATA_LENGTH			(DEFAULT_BLOCKLEN + DATA_CRC_LENGTH + DATA_TOKEN_LENGTH)

/*	Response lengths measured in 8-clock cycles	*/
#define R1_LENGTH			1
#define R1b_LENGTH			2
#define R2_LENGTH			2
#define R3_LENGTH			5
#define R7_LENGTH			5
#define BUSY_CYCLES			1

/*	Delays measured in 8-clock cycles	*/
#define MAX_NCR 		8
#define MAX_NRC			1
#define MAX_NAC			8000		/*	FIXME: See this value	*/
#define MAX_NWR			1
#define MAX_NBR			8
#define MAX_NCX			1
#define MAX_DELAY		0x7ffff		/*	FIXME: 40ms aprox.	*/

/*	Transfer lengths	*/
#define COMMAND_R1_LENGTH		(COMMAND_LENGTH + MAX_NCR + R1_LENGTH)
#define COMMAND_R1b_LENGTH		(COMMAND_LENGTH + MAX_NCR + R1b_LENGTH)
#define COMMAND_R2_LENGTH		(COMMAND_LENGTH + MAX_NCR + R2_LENGTH)
#define COMMAND_R3_LENGTH		(COMMAND_LENGTH + MAX_NCR + R3_LENGTH)
#define COMMAND_R7_LENGTH		(COMMAND_LENGTH + MAX_NCR + R7_LENGTH)
#define DATA_TRANSFER_LENGTH	(COMMAND_LENGTH + MAX_NCR + R1_LENGTH + MAX_NWR + DEFAULT_DATA_LENGTH + BUSY_CYCLES)

typedef enum sd_command_type{
	CMD0 = 0x00,		/*!< GO_IDLE_STATE	*/
	CMD8 = 0x08,		/*!< SEND_IF_COND	*/
	CMD9 = 0x09,		/*!< SEND_CSD	*/
	CMD13 = 0x0d,		/*!< SEND_STATUS	*/
	CMD16 = 0x10,		/*!< SET_BLOCKLEN	*/
	CMD17 = 0x11,		/*!< READ_SINGLE_BLOCK	*/
	CMD24 = 0x18,		/*!< WRITE_BLOCK	*/
	CMD32 = 0x20,		/*!< ERASE_WR_BL_START_ADDR	*/
	CMD33 = 0x21,		/*!< ERASE_WR_BL_END_ADDR	*/
	CMD38 = 0x26,		/*!< ERASE	*/
		ACMD41 = 0x29,	/*!< SD_SEND_OP_COND	*/
		ACMD51 = 0x33,	/*!< SEND_SCR	*/
	CMD55 = 0x37,		/*!< APP_CMD		*/
	CMD58 = 0x3a		/*!< READ_OCR	*/
}sd_command_t;

typedef enum response_type{
	RESPONSE_1,
	RESPONSE_1B,
	RESPONSE_2,
	RESPONSE_3,
	RESPONSE_7
} response_type;

typedef struct __attribute__ ((packed)) sd_command_format{
	uint8_t start_bit:1;			/*!< Start of transmission. It must be 0b	*/
	uint8_t transmission_bit:1;		/*!< It must be 1b	*/
	sd_command_t command:6;
	uint32_t argument;
	uint8_t crc:7;
	uint8_t end_bit:1;		/*!< End of transmission. It must be 1b	*/
} sd_command_f;

typedef struct response_token{

	union r1_token{
		uint8_t r1_content;
		struct __attribute__ ((packed)) r1_flags{
			uint8_t zero:1;
			uint8_t parameter_error:1;
			uint8_t address_error:1;
			uint8_t erase_seq_error:1;
			uint8_t com_crc_error:1;
			uint8_t illegal_command:1;
			uint8_t erase_reset:1;
			uint8_t in_idle_state:1;
		}r1_flags;
	}r1;

	union addition{
		union r2_addition{
			uint8_t r2_content;		/*	This is not proper name for this field. It only stores second information byte.	*/
			struct __attribute__ ((packed)) r2_flags{
				uint8_t out_of_range_csd_overwrite:1;
				uint8_t erase_param:1;
				uint8_t wp_violation:1;
				uint8_t card_ecc_failed:1;
				uint8_t cc_error:1;
				uint8_t error:1;
				uint8_t wp_erase_skip_lock_unlk_cmd_failed:1;
				uint8_t card_is_locked:1;
			}r2_flags;
		}r2;
		struct r3_addition{
			uint32_t ocr;
		}r3;
		union r7_addition{
			uint32_t r7_content;
			struct __attribute__ ((packed)) r7_fields{
				uint32_t command_version:4;
				uint32_t rsvd:16;
				uint32_t voltage_accepted:4;
				uint32_t check_pattern:8;
			}r7_fields;
		}r7;
	}addition;

}response_f;


/*!
 *	This function initializes the SD card into SPI mode and leaves it ready to start data transactions.
 *	\return. A boolean value that indicates if the initialization operation was successfully or not.
 */
bool SDInit();

/*!
 * This function performs block erasing of all blocks between provided addresses, including last one.
 * \return A boolean value that indicates if the operation was successful.
 */
bool eraseBlocks(uint32_t first_block_address, uint32_t last_block_address);

/*!
 * This function reads a variable number of 512-bytes blocks of data.
 * \param address. Address of the first byte to be read. IMPORTANT: This must be aligned to 512.
 * \param blocks. Number of blocks to be read.
 * \return. A pointer to the array that will store read data. A NULL pointer will be returned
 *  	if the operation cannot be executed because of an error.
 */
uint16* readSingleDataBlock(uint32_t address);

/*!
 * This function tries to write blocks of data into SD card.
 * \param start_address The address of the first byte to be written.
 * \param data A pointer to the chunk of data to be written.
 * \param bytes_count The number of bytes in data array.
 * \return A value that represents the status of the writing operation. It must be a command
 * 		response or an operation token.
 * 		IMPORTANT: It is the responsibility of the receiver to interpret the returned value correctly.
 */
bool writeSingleDataBlock(uint32_t start_address, uint16_t* data);

/*!
 *	This function sets block length for all write and read operations.
 *		IMPORTANT: This function works for 512-byte blocks only. tx_data and rx_data
 *		array sizes must be changed for different block lengths.
 *	\param blocklen Block length in bytes.
 *	\return A boolean value that indicates if the operation was successful.
 */
bool setBlockLength(uint32_t blocklen);

/*!
 *	This function is used to read the internal OCR register of the SD card.
 *	\return The value of OCR register.
 */
uint32_t readOCR();


/*******************************************************************************************************
 ********************************			Static functions 			********************************
 ******************************************************************************************************/
/*!
 * This function initializes the array that contains the data to be sent to 0xff in all their positions.
 */
static void initTransmitionArray();

/*!
 * This function performs a nonspecific delay.
 * \param value that indicates a proportional time to be delayed.
 */
static void delay(uint32_t value);

/*!
 * This function converts fields from command struct to short integer into tx_data array.
 * \param command The command to be converted.
 */
static void commandToInteger16(sd_command_f command);

static void dataForSent(uint16_t* data);

/*!
 * This function checks the internal SD card status.
 * \return A boolean value that indicates if the SD is still busy (true) or ready for another transaction (false).
 */
static bool SDIsBusy();

/*!
 *	This function reports SD status that could be changed by an operation.
 *	\return The value of SD status as R2 response format.
 */
static response_f SDStatus();

/*!
 * This is a basic function that puts data onto MOSI line and reads data from MISO line.
 * \param tx_length The maximum length expected for the transmission, measured in 8-clock cycles.
 * \param r_type The expected response type for sent data.
 * \return A pointer to the read values from MISO line along all the transmission.
 */
static response_f SDSendCommand(sd_command_f command, uint32_t tx_length, response_type r_type);

/*!
 *	This function is used for single block write operations
 *	\param tx_length The expected length for current transmission measured in 8-clock cycles.
 *	\param data_response_token A pointer that will store data response token.
 *	\return The response for the sent command before data.
 */
static response_f SDSendData(uint32_t tx_length, uint8_t* data_response_token);

/*!
 * This is a basic function that reads data from MISO line, sending a constant HIGH level
 * signal on MOSI line during all transmission.
 * \param tx_length The maximum length expected for the transmission, measured in 8-clock cycles.
 * \return A pointer to the block of data read.
 */
static uint16_t* SDReceiveData(uint32_t tx_length);

#endif /* MY_INCLUDE_SD_H_ */
