/*
 * communication.h
 *
 *  Created on: 4/10/2016
 *      Author: SalaLimpia
 */

#ifndef MY_INCLUDE_COMMUNICATION_H_
#define MY_INCLUDE_COMMUNICATION_H_

#include <stdint.h>

#define SEQ_FIELD_SIZE	2	/*< Sequence field number in bytes	*/
#define DATA_LENGTH		512	/*< Data payload in bytes (i.e. 4096 bits)	*/
#define CRC_LENGTH		2	/*<	CRC length in bytes	*/

#define ERASE_PACKET_PAYLOAD	7
#define LOAD_PACKET_PAYLOAD		(DATA_LENGTH + SEQ_FIELD_SIZE)
#define	PROGRAM_PACKET_PAYLOAD	122

#define EXT_MEM_INIT_ADDRESS	4
#define EXT_MEM_SIZE			3
#define SECTION_FIELD			2
#define SECTIONS_FIELD_LENGTH	3	/*< Field width (in bytes) for sections fields in program command	*/
#define MAX_SECTIONS_NUMBER		20

#define MAX_PAYLOAD_LENGTH		(DATA_LENGTH + SEQ_FIELD_SIZE)

typedef enum command_type{
		UNKNOWN = 0x00,
	ERASE_FLASH = 0x45,			/*< Erase external flash. ASCII representation of 'E' character	*/
	LOAD_PROGRAM = 0x4c,		/*< Load program received onto external flash. ASCII representation of 'L' character	*/
	PROGRAM_FLASH = 0x50		/*<	Writing program from external flash onto the other OBC. ASCII representation of 'P' character	*/
}command_t;

typedef struct base_packet{
	command_t command;			/*<	Packet type. It will define the payload	*/
	union payload{
		uint8_t payload[MAX_PAYLOAD_LENGTH];
		struct erase_command{
			uint8_t start_address[EXT_MEM_INIT_ADDRESS];
			uint8_t size[EXT_MEM_SIZE]; /*< This field stores block size to be erased expressed in bytes */
		}erase_command_content;
		struct load_command {
			uint8_t seq_number[SEQ_FIELD_SIZE];		/*<	Sequence number for program received packets	*/
			uint8_t data[DATA_LENGTH];		/*< Packet data */
		}load_command_content;
		struct program_command{
			uint8_t number_sections[SECTION_FIELD];
			struct sections{
				uint8_t section_length[SECTIONS_FIELD_LENGTH];
				uint8_t section_start_address[SECTIONS_FIELD_LENGTH];
			} sections[MAX_SECTIONS_NUMBER];
		}program_command_content;
	}payload_content;
	uint8_t crc[CRC_LENGTH];				/*< 16-bit CRC for the received packet	*/
}packet_t;


typedef struct section_fields{
	uint32_t length;
	uint32_t start_address;
}section_field;


typedef enum status_code{
		DEFAULT = 0x00,
	SUCCESS = 0x53,
	ERROR	= 0x45
}status_t;

/*!
 * This function initializes the communication channel interface
 */
void commInit();

/*!
 * This function reads incoming data from UART interface.
 * \return A packet if its label was recognized as correct.
 */
packet_t receivePacket();

/*!
 * This function sends a code that indicates the status of an operation.
 * \param status_t A code that indicates 'ERROR' or 'SUCCESS'
 */
void sendMessage(status_t code);

#endif /* MY_INCLUDE_COMMUNICATION_H_ */
