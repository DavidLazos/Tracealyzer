/*
 * communication.c
 *
 *  Created on: 28 de jul. de 2017
 *      Author: Fabio
 */
#include "communication.h"

#include <stdio.h>
#include <string.h>
#include <sci.h>

#include "crc16.h"

#define sci	sciREG

static const uint16_t poly = 0xa2eb;
static uint16_t* crctable;

void commInit()
{
	sciInit();

	/* Initialize CRC table */
	crctable = CalculateTable_CRC16(poly);
}

packet_t receivePacket()
{
	static packet_t packet;
	uint16_t crcvalue;
	uint16_t crcreceived;
	uint32_t packetsize;
	status_t status;

	do{
		status = ERROR;
		packetsize = 0;
		memset(&packet.command, 0, sizeof(command_t));
		sciReceive(sci, 1, (uint8_t *) &packet.command);

		switch(packet.command){
		case ERASE_FLASH:
			packetsize = ERASE_PACKET_PAYLOAD;
			break;
		case LOAD_PROGRAM:
			packetsize = LOAD_PACKET_PAYLOAD;
			break;
		case PROGRAM_FLASH:
			packetsize = PROGRAM_PACKET_PAYLOAD;
			break;
		default:
			packetsize = 0;
			break;
		}

		if(packetsize)
		{
			sciReceive( sci, packetsize, packet.payload_content.payload );
			sciReceive( sci, CRC_LENGTH, packet.crc );

			crcvalue = Compute_CRC16( packet.payload_content.payload, packetsize, crctable );
			crcreceived = 0x0 | ( packet.crc[0] << 8 )
							  | ( packet.crc[1] );
			status = ( crcvalue == crcreceived ? SUCCESS : ERROR );
		}
	}while(status == ERROR);

	return packet;
}

void sendMessage(status_t code)
{
	sciSend(sci, 1, (uint8_t *)&code);
}
