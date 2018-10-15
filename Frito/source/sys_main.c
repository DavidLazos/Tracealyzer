/* Include Files */
#include "sys_common.h"
#include "system.h"
/* USER CODE BEGIN (1) */
/* Include FreeRTOS scheduler files */
#include "FreeRTOS.h"
#include "os_task.h"
/* Include HET header file - types, definitions and function declarations for system driver */
#include "het.h"
#include "esm.h"
#include "gio.h"
#include "sci.h"
#include "os_semphr.h"

#define  TSIZE1 7
uint8  TEXT1[TSIZE1]= {'T','O','M','A','D','O',' '};
#define  TSIZE2 7
uint8  TEXT2[TSIZE2]= {'N','O',' ','P','U','D','O'};

void sciDisplayText(sciBASE_t *sci, uint8 *text, uint32 length);
void wait(uint32 time);

#define UART scilinREG

//xSemaphoreTake( SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait );

SemaphoreHandle_t xSemaphore =NULL;

/* Stack Buffer for Task 1 */
//#pragma DATA_ALIGN(stackbuffer, configMINIMAL_STACK_SIZE*sizeof(portSTACK_TYPE))
//static portSTACK_TYPE stackbuffer[configMINIMAL_STACK_SIZE] __attribute__ ((aligned (configMINIMAL_STACK_SIZE * sizeof(portSTACK_TYPE))));
//static portSTACK_TYPE stackbuffer2[configMINIMAL_STACK_SIZE] __attribute__ ((aligned (configMINIMAL_STACK_SIZE * sizeof(portSTACK_TYPE))));


/* Define Task Handles */

//xTaskHandle xTask1Handle;
//xTaskHandle xTask2Handle;
/* Task function */



void vTask1(void *pvParameters){
    /* Set high end timer GIO port hetPort pin direction to all output */

    gioSetDirection(hetPORT1, 0xFFFFFFFF);
        static int cuenta=0;
    for(;;)
    {

        if( xSemaphore != NULL )
        {
            /* See if we can obtain the semaphore.  If the semaphore is not
            available wait 10 ticks to see if it becomes free. */
            if( xSemaphoreTake( xSemaphore, ( TickType_t ) 1 ) == pdTRUE )
            {
                /* Toggle HET[1] with timer tick */
                gioSetBit(hetPORT1, 17, gioGetBit(hetPORT1, 17) ^ 1);
                                cuenta=cuenta+1;
                                if(cuenta==100)
                                    vTraceStop();
                //           wait(20000);
                xSemaphoreGive( xSemaphore );
                vTaskDelay(50);

            }
            else{
                gioSetBit(hetPORT1, 27, gioGetBit(hetPORT1, 27) ^ 1);
                //gioSetBit(hetPORT1, 27, 1);
                vTaskDelay(10);
            }
        }
    }
}
/*Task 1 Parameters*/
//static const xTaskParameters xTaskParamters1={
//                                              vTask1,               /* Function that implements the task */
//                                              "Blinky",/* Just a text name for the task to assist debugging */
//                                              configMINIMAL_STACK_SIZE,     /* Stack size */
//                                              NULL,                         /* Parametrs to be passed to the task function */
//                                              1,                            /* Task Priority. set the portPRIVILEGE_BIT (1|portPRIVILEGE_BIT) if the task should run in a privileged state*/
//                                              stackbuffer,                  /* Buffer to be used as the task stack */
//                                              /* xRegions - Allocate up to three separate memory regions for access by the task, with appropriate access permissions.*/
//                                              /* No region is set in this example. */
//                                              NULL
//};
/* USER CODE END */
/* USER CODE BEGIN (2) */

void vTask2(void *pvParameters){
    /* Set high end timer GIO port hetPort pin direction to all output */
    gioSetDirection(hetPORT1, 0xFFFFFFFF);
    //   xTaskCreate( vTask1, "Task 1", 256, NULL, 5, NULL );

    for(;;)
    {
        if( xSemaphore != NULL ){

            if( xSemaphoreTake( xSemaphore, ( TickType_t ) 1 ) == pdTRUE )
            {
                /* Toggle HET[1] with timer tick */
                gioSetBit(hetPORT1, 0, gioGetBit(hetPORT1, 0) ^ 1);
                //               wait(200000);
                xSemaphoreGive(xSemaphore);
                vTaskDelay(150);

            }
            else{
                gioSetBit(hetPORT1, 18, gioGetBit(hetPORT1, 18) ^ 1);
                //gioSetBit(hetPORT1, 18, 1);
                vTaskDelay(10);
            }
            // vTraceStop();
        }
    }
}

void vTask3(void *pvParameters){
    /* Set high end timer GIO port hetPort pin direction to all output */
    gioSetDirection(hetPORT1, 0xFFFFFFFF);

    for(;;)
    {
        if( xSemaphore != NULL ){

            if( xSemaphoreTake( xSemaphore, ( TickType_t ) 1 ) == pdTRUE )
            {
                //sciDisplayText(UART,&TEXT1[0],TSIZE1);        /* send text code 1 */
                //wait(200);
                /* Toggle HET[1] with timer tick */
                gioSetBit(hetPORT1, 25, gioGetBit(hetPORT1, 25) ^ 1);
                //vTaskDelay(1000);
                sciDisplayText(UART,&TEXT1[0],TSIZE1);
                wait(100);
                //vTraceStop();
                xSemaphoreGive( xSemaphore );
                vTaskDelay(1000);
            }
            else{
                sciDisplayText(UART,&TEXT2[0],TSIZE2);
                vTaskDelay(100);
            }
        }
    }
}
/* USER CODE END */

void main(void){
    vTraceEnable(TRC_INIT);
    sciInit();


    /* USER CODE BEGIN (3) */
    /* Create Task 1 */
#if(configUSE_TRACE_FACILITY==1)
    vTraceEnable(TRC_START);
#endif
    //    if (xTaskCreateRestricted(&xTaskParamters1, &xTask1Handle) != pdTRUE)
    //    {
    //        /* Task could not be created */
    //        while(1);
    //    }

    //uiTraceStart();

    xSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive( xSemaphore );

    //uiTraceStart();

    xTaskCreate( vTask1, "Task 1", 256, NULL, 5, NULL );
    xTaskCreate( vTask2, "Task 2", 256, NULL, 3, NULL );
    xTaskCreate( vTask3, "Task 3", 256, NULL, 1, NULL );

    /* Start Scheduler */
    vTaskStartScheduler();    /* Run forever */
    while(1);
    /* USER CODE END */
}

void sciDisplayText(sciBASE_t *sci, uint8 *text,uint32 length){
    while(length--)
    {
        //while ((UART->FLR & 0x4) == 4);
        if(sciIsTxReady(UART)==4);
        /* wait until busy */
        //wait(100);
        sciSendByte(UART,*text++);
        /* send out text   */
    }
}
void wait(uint32 time){
    while(time!=0)
        time--;
}/* USER CODE END */
