/** @file sys_main.c 
 *   @brief Application main file
 *   @date 08-Feb-2017
 *   @version 04.06.01
 *
 *   This file contains an empty main function,
 *   which can be used for the application.
 */

/* 
 * Copyright (C) 2009-2016 Texas Instruments Incorporated - www.ti.com
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


/* USER CODE BEGIN (0) */
/* USER CODE END */

/* Include Files */

#include "sys_common.h"
#include "system.h"

/* USER CODE BEGIN (1) */
#include <sys_core.h>
#include <communication.h>
#include <sd.h>
#include <programmer_operations.h>
/* Include FreeRTOS scheduler files */
#include "FreeRTOS.h"
#include "os_task.h"
#include "os_semphr.h"

/* USER CODE END */

/** @fn void main(void)
 *   @brief Application main function
 *   @note This function is empty by default.
 *
 *   This function is called after startup.
 *   The user can use this function to implement the application.
 */

SemaphoreHandle_t xSemaphore =NULL;


packet_t packet;
status_t status;
uint32_t first_block_address;   /*  Variable for ERASE command  */
uint32_t sd_size;               /*  Variable for ERASE command  */
uint32_t start_address;
uint16_t sd_data[DEFAULT_BLOCKLEN];
uint16_t* sd_read_data;
int i, j;       /* Loop variable    */
bool sd_status;

/*  Programming variables   */
int jtag_status;
uint32_t sections_number;
section_field sections[MAX_SECTIONS_NUMBER];
uint32_t external_flash_address;
uint32_t chunk_size;
uint8_t jtag_data[DEFAULT_BLOCKLEN];

/* USER CODE BEGIN (2) */
void vTask1(void *pvParameters){
    commInit();
    if (!SDInit())
        exit(0);
    while(1)
    {
        packet = receivePacket();

        switch(packet.command){
        case ERASE_FLASH:
            i = 0;
            first_block_address =
                    0x0 | ( packet.payload_content.erase_command_content.start_address[0] << 24 )
                    | ( packet.payload_content.erase_command_content.start_address[1] << 16 )
                    | ( packet.payload_content.erase_command_content.start_address[2] << 8 )
                    | ( packet.payload_content.erase_command_content.start_address[3] );
            sd_size = 0x0 | ( packet.payload_content.erase_command_content.size[0] << 16 )
                                  | ( packet.payload_content.erase_command_content.size[1] << 8 )
                                  | ( packet.payload_content.erase_command_content.size[2] );
            do
            {
                sd_status = eraseBlocks(first_block_address, sd_size * DEFAULT_BLOCKLEN);
                ++i;
            }while( (!sd_status) || (i<10) );
            status = (sd_status ? SUCCESS : ERROR);
            break;
        case LOAD_PROGRAM:
            start_address = 0x0UL | (packet.payload_content.load_command_content.seq_number[0] << 8)
            | (packet.payload_content.load_command_content.seq_number[1]);

            for(i=0; i<DATA_LENGTH; i++)
            {
                sd_data[i] = (0xFF & packet.payload_content.load_command_content.data[i]);
            }

            i = 0;
            do
            {
                sd_status = writeSingleDataBlock(start_address*DATA_LENGTH, sd_data);
                ++i;
            }while( (!sd_status) || (i<10) );
            status = (sd_status ? SUCCESS : ERROR);
            break;
        case PROGRAM_FLASH:
            sections_number = 0x0UL | (packet.payload_content.program_command_content.number_sections[0] << 8)
            | (packet.payload_content.program_command_content.number_sections[1]);

            for(i = 0; i < sections_number; i++)
            {
                sections[i].length =
                        0x0 | (packet.payload_content.program_command_content.sections[i].section_length[0] << 16)
                        | (packet.payload_content.program_command_content.sections[i].section_length[1] << 8)
                        | (packet.payload_content.program_command_content.sections[i].section_length[2]);
                sections[i].start_address =
                        0x0 | (packet.payload_content.program_command_content.sections[i].section_start_address[0] << 16)
                        | (packet.payload_content.program_command_content.sections[i].section_start_address[1] << 8)
                        | (packet.payload_content.program_command_content.sections[i].section_start_address[2]);
            }

            i = 0;
            do
            {
                jtag_status = initProgramming();
                ++i;
            }while( (!jtag_status) && (i<10) );

            if(!jtag_status)
            {
                status = ERROR;
                break;
            }
            else
            {
                external_flash_address = 0x0UL;
                for(i = 0; i < sections_number; i++)
                {
                    while(sections[i].length)
                    {
                        chunk_size = (sections[i].length > DEFAULT_BLOCKLEN ? DEFAULT_BLOCKLEN : sections[i].length);
                        sd_read_data = readSingleDataBlock(external_flash_address);

                        for(j=0; j<DEFAULT_BLOCKLEN; j++)
                        {
                            jtag_data[j] = (0xFF & sd_read_data[j]);
                        }

                        jtag_status = flashProgramming((uint32_t*) jtag_data, chunk_size, sections[i].start_address);

                        external_flash_address += DEFAULT_BLOCKLEN;
                        sections[i].length -= chunk_size;
                        sections[i].start_address += chunk_size;
                        if(!jtag_status)
                        {
                            status = ERROR;
                            break;
                        }
                    }
                }
                resetController();
            }
            status = SUCCESS;
            break;
        default:
            status = ERROR;
            break;
        }

        /* Report status */
        sendMessage(status);
        //vTaskDelay(10);
    }

}
/* USER CODE END */



int main(void)
{
    /* USER CODE BEGIN (3) */


    /*	Start main function	*/
    _disable_interrupt_();
    vTraceEnable(TRC_INIT);
#if(configUSE_TRACE_FACILITY==1)
    vTraceEnable(TRC_START);
#endif

    xSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive( xSemaphore );


    xTaskCreate( vTask1, "Task 1", 256, NULL, 5 | portPRIVILEGE_BIT, NULL );

    /*	Initialize interfaces	*/
    //    xTaskCreateRestricted( &xTask1, NULL );
    vTaskStartScheduler();

    //	while(1)
    //	{
    //		packet = receivePacket();
    //
    //		switch(packet.command){
    //		case ERASE_FLASH:
    //			i = 0;
    //			first_block_address =
    //					0x0 | ( packet.payload_content.erase_command_content.start_address[0] << 24 )
    //						| ( packet.payload_content.erase_command_content.start_address[1] << 16 )
    //						| ( packet.payload_content.erase_command_content.start_address[2] << 8 )
    //						| ( packet.payload_content.erase_command_content.start_address[3] );
    //			sd_size = 0x0 | ( packet.payload_content.erase_command_content.size[0] << 16 )
    //						  | ( packet.payload_content.erase_command_content.size[1] << 8 )
    //						  | ( packet.payload_content.erase_command_content.size[2] );
    //			do
    //			{
    //				sd_status = eraseBlocks(first_block_address, sd_size * DEFAULT_BLOCKLEN);
    //				++i;
    //			}while( (!sd_status) || (i<10) );
    //			status = (sd_status ? SUCCESS : ERROR);
    //			break;
    //		case LOAD_PROGRAM:
    //			start_address = 0x0UL | (packet.payload_content.load_command_content.seq_number[0] << 8)
    //								  | (packet.payload_content.load_command_content.seq_number[1]);
    //
    //			for(i=0; i<DATA_LENGTH; i++)
    //			{
    //				sd_data[i] = (0xFF & packet.payload_content.load_command_content.data[i]);
    //			}
    //
    //			i = 0;
    //			do
    //			{
    //				sd_status = writeSingleDataBlock(start_address*DATA_LENGTH, sd_data);
    //				++i;
    //			}while( (!sd_status) || (i<10) );
    //			status = (sd_status ? SUCCESS : ERROR);
    //			break;
    //		case PROGRAM_FLASH:
    //			sections_number = 0x0UL | (packet.payload_content.program_command_content.number_sections[0] << 8)
    //									| (packet.payload_content.program_command_content.number_sections[1]);
    //
    //			for(i = 0; i < sections_number; i++)
    //			{
    //				sections[i].length =
    //						0x0 | (packet.payload_content.program_command_content.sections[i].section_length[0] << 16)
    //							| (packet.payload_content.program_command_content.sections[i].section_length[1] << 8)
    //							| (packet.payload_content.program_command_content.sections[i].section_length[2]);
    //				sections[i].start_address =
    //						0x0 | (packet.payload_content.program_command_content.sections[i].section_start_address[0] << 16)
    //							| (packet.payload_content.program_command_content.sections[i].section_start_address[1] << 8)
    //							| (packet.payload_content.program_command_content.sections[i].section_start_address[2]);
    //			}
    //
    //			i = 0;
    //			do
    //			{
    //				jtag_status = initProgramming();
    //				++i;
    //			}while( (!jtag_status) && (i<10) );
    //
    //			if(!jtag_status)
    //			{
    //				status = ERROR;
    //				break;
    //			}
    //			else
    //			{
    //				external_flash_address = 0x0UL;
    //				for(i = 0; i < sections_number; i++)
    //				{
    //					while(sections[i].length)
    //					{
    //						chunk_size = (sections[i].length > DEFAULT_BLOCKLEN ? DEFAULT_BLOCKLEN : sections[i].length);
    //						sd_read_data = readSingleDataBlock(external_flash_address);
    //
    //						for(j=0; j<DEFAULT_BLOCKLEN; j++)
    //						{
    //							jtag_data[j] = (0xFF & sd_read_data[j]);
    //						}
    //
    //						jtag_status = flashProgramming((uint32_t*) jtag_data, chunk_size, sections[i].start_address);
    //
    //						external_flash_address += DEFAULT_BLOCKLEN;
    //						sections[i].length -= chunk_size;
    //						sections[i].start_address += chunk_size;
    //						if(!jtag_status)
    //						{
    //							status = ERROR;
    //							break;
    //						}
    //					}
    //				}
    //				resetController();
    //			}
    //			status = SUCCESS;
    //			break;
    //		default:
    //			status = ERROR;
    //			break;
    //		}
    //
    //		/* Report status */
    //		sendMessage(status);
    //	}

    /* USER CODE END */

}


/* USER CODE BEGIN (4) */
/* USER CODE END */
