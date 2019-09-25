/*
 * main.c
 *
 *  Created on: 17 џэт. 2019 у.
 *      Author: DA.Tsekh
 */


#include <stdint.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <queue.h>

#include "config.h"
#include "MDR32F9Qx_config.h"
#include "MDR32F9Qx_port.h"
#include "MDR32Fx.h"

#include "uart.h"


/* Typedef's -----------------------------------------------------------------*/
typedef struct TaskParam_t {
  MDR_PORT_TypeDef	*port;
  uint32_t      	led_num ;
  uint32_t      	period  ;
} TaskParam;


void vApplicationIdleHook( void );
void vApplicationTickHook( void );
void vApplicationMallocFailedHook( void );
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName  );

QueueHandle_t xUART2RXQueue;

/* Functions -----------------------------------------------------------------*/

void configLED( void ){
	PORT_InitTypeDef PORT_InitStructure;

	/* Enables the RTCHSE clock on PORTA, PORTB and PORTC */
	RST_CLK_PCLKcmd( RST_CLK_PCLK_PORTC, ENABLE);

	/* Configure PORTD pins 2..6 for input to handle joystick events */
	PORT_InitStructure.PORT_Pin = PORT_Pin_2;
	PORT_InitStructure.PORT_OE = PORT_OE_OUT;
	PORT_InitStructure.PORT_FUNC = PORT_FUNC_PORT;
	PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;
	PORT_InitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;

	PORT_Init(MDR_PORTC, &PORT_InitStructure);

}

/*--------------------------------------------------------------*/
/* Task Function - UART echo									*/
/*--------------------------------------------------------------*/
void vTaskFunctionUART2echo( void *pvParameters )
{
  volatile TaskParam *pxTaskParam;
  portTickType xLastWakeTime;
  portBASE_TYPE xStatus;

  pxTaskParam = (TaskParam *) pvParameters;
  xLastWakeTime = xTaskGetTickCount();

  uint16_t pxuartmsg;

  UART2_SendStr("\n\rMDR2_UART2 echo (RX - PF0, TX - PF1):\n\r");

  if( xUART2RXQueue == 0 ){
	  while(1);
  }

  for( ;; )
  {
	  xStatus = xQueueReceive(xUART2RXQueue, &(pxuartmsg), portMAX_DELAY);
	  if( xStatus == pdPASS ){
		  // Send MSG to MDR_UART2
		  UART_SendData(MDR_UART2, (uint8_t)pxuartmsg );
	  }
  }
  vTaskDelete( NULL );
}


/*--------------------------------------------------------------*/
/* Task Function - Blink LED and Delay                          */
/*--------------------------------------------------------------*/
void vTaskFunctionBlinkLED( void *pvParameters )
{
  volatile TaskParam *pxTaskParam;
  portTickType xLastWakeTime;

  pxTaskParam = (TaskParam *) pvParameters;
  xLastWakeTime = xTaskGetTickCount();

  configLED();

  for( ;; )
  {
	PORT_SetBits( MDR_PORTC, PORT_Pin_2 );
	vTaskDelayUntil( &xLastWakeTime, 500);
	PORT_ResetBits( MDR_PORTC, PORT_Pin_2 );
	vTaskDelayUntil( &xLastWakeTime, 500);
  }
  vTaskDelete( NULL );
}


/* Functions -----------------------------------------------------------------*/

void printLogo( void ){
	  UART2_SendStr("\x1b[2J\x1b[1;1H"); /* Clear screen */
	  UART2_SendStr("\e[107m                   \e[49m\e[0m\n\r");
	  UART2_SendStr("\e[107m                   \e[49m\e[0m\n\r");
	  UART2_SendStr("\e[44m                   \e[49m\e[0m\n\r");
	  UART2_SendStr("\e[44m                   \e[49m\e[0m\n\r");
	  UART2_SendStr("\e[41m                   \e[49m\e[0m\n\r");
	  UART2_SendStr("\e[41m                   \e[49m\e[0m\n\r");
	  UART2_SendStr("\n\r\e[32m-=< K1986BE92QI >=-\e[39m\e[49m\e[0m\n\r\a");
}


int main (void){

	RST_CLK_FreqTypeDef RST_CLK_Clocks;
	TaskParam xTP1;

	xTP1.port = MDR_PORTA;
	xTP1.led_num = PORT_Pin_9;
	xTP1.period = 100L;

	// Configure MCU clocks
	clock_configure();

	// Enable used ports
	init_all_ports();

	// Configure pins and enable UART2
	init_UART2();
	printLogo();

	// Just for checking MCU clocks in "RST_CLK_Clocks"
	RST_CLK_GetClocksFreq( &RST_CLK_Clocks );

	xUART2RXQueue = xQueueCreate( 255, sizeof( uint16_t ) );

	xTaskCreate( vTaskFunctionBlinkLED, ( char * ) "LED", configMINIMAL_STACK_SIZE*10,  (void*)&xTP1, 1, NULL );
	xTaskCreate( vTaskFunctionUART2echo, ( char * ) "UART2", configMINIMAL_STACK_SIZE*10,  (void*)&xTP1, 1, NULL );

	vTaskStartScheduler();

	while(1){
	}

	return 0;
}

/*--------------------------------------------------------------*/
/* Idle Task Function                                           */
/*--------------------------------------------------------------*/
void vApplicationIdleHook( void )
{
}

/*--------------------------------------------------------------*/
/* Idle Task Function                                           */
/*--------------------------------------------------------------*/
void vApplicationTickHook( void )
{
}

void vApplicationMallocFailedHook( void ){
	while(1);
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName ){
	while(1);
}

