#include "uart.h"

#include <FreeRTOS.h>
#include <queue.h>


extern QueueHandle_t xUART2RXQueue;

static UART_InitTypeDef UART_InitStructure;
PORT_InitTypeDef UART_PORT_InitStructure;


uint8_t uart2_rx_buf[UART2_RX_BUF_SIZE] = {0};
uint16_t uart2_rx_buf_indx = 0;
uint8_t uart2_IT_RX_flag = RESET;

void UART2_Send(char* buf, unsigned int buf_size)
{
	unsigned int index;
	for(index = 0; index < buf_size; index++){
		// Check TXFE flag
		while (UART_GetFlagStatus(MDR_UART2, UART_FLAG_TXFE ) != SET);
		UART_SendData(MDR_UART2, (uint8_t) buf[index]);
	}
}

void UART2_SendStr(char *buf)
{
	UART2_Send(buf, strlen(buf));
}

void init_UART2( void ){
	// Enables the HSI clock on PORTC, PORTD
	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTF, ENABLE);

	// Enables the CPU_CLK clock on UART2
	RST_CLK_PCLKcmd(RST_CLK_PCLK_UART2, ENABLE);

	// Fill PortInit structure
	UART_PORT_InitStructure.PORT_PULL_UP = PORT_PULL_UP_ON;
	UART_PORT_InitStructure.PORT_PULL_DOWN = PORT_PULL_DOWN_OFF;
	UART_PORT_InitStructure.PORT_PD_SHM = PORT_PD_SHM_OFF;
	UART_PORT_InitStructure.PORT_PD = PORT_PD_DRIVER;
	UART_PORT_InitStructure.PORT_GFEN = PORT_GFEN_OFF;
	UART_PORT_InitStructure.PORT_FUNC = PORT_FUNC_OVERRID;
	UART_PORT_InitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;
	UART_PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;

	// Configure PORTF pin 0 (UART2_RX) as input
	UART_PORT_InitStructure.PORT_OE = PORT_OE_IN;
	UART_PORT_InitStructure.PORT_Pin = PORT_Pin_0;
	PORT_Init(MDR_PORTF, &UART_PORT_InitStructure);

	// Configure PORTF pin 1 (UART2_TX) as output
	UART_PORT_InitStructure.PORT_OE = PORT_OE_OUT;
	UART_PORT_InitStructure.PORT_Pin = PORT_Pin_1;
	PORT_Init(MDR_PORTF, &UART_PORT_InitStructure);


	// Set the HCLK division factor = 1 for UART1,UART2
	UART_BRGInit(MDR_UART2, UART_HCLKdiv1 );
	NVIC_EnableIRQ( UART2_IRQn );

	// Initialize UART_InitStructure
	UART_InitStructure.UART_BaudRate = 9600;
	UART_InitStructure.UART_WordLength = UART_WordLength8b;
	UART_InitStructure.UART_StopBits = UART_StopBits1;
	UART_InitStructure.UART_Parity = UART_Parity_No;
	UART_InitStructure.UART_FIFOMode = UART_FIFO_OFF;
	UART_InitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_RXE | UART_HardwareFlowControl_TXE;

	// Configure UART2 parameters
	UART_Init(MDR_UART2, &UART_InitStructure);

	// Configure DMA for UART2
	UART_DMAConfig(MDR_UART2, UART_IT_FIFO_LVL_2words,	UART_IT_FIFO_LVL_14words );
	UART_DMACmd(MDR_UART2, UART_DMA_TXE, ENABLE); // | UART_DMA_ONERR

	// Configure UART2 receive interrupt
	UART_ITConfig(MDR_UART2, UART_IT_RX, ENABLE);

	// Enables UART2 peripheral
	UART_Cmd(MDR_UART2, ENABLE);

	NVIC_EnableIRQ(UART2_IRQn);

}

char Check_UART1_RX( void ){
	uint16_t rx = 0;
	if( uart2_IT_RX_flag == SET ){
		rx  = UART_ReceiveData( MDR_UART2 ) & 0x00FF;
		uart2_IT_RX_flag = RESET;
	}
	return rx;
}

void get_line_UART1_rx( char * buf ){
	assert_param( buf != NULL );

	uint16_t rx;
	int i=0;
	char flag = 1;

	memset( buf, 0, MAX_UART_LINE * sizeof(uint8_t) );

	while( flag ){
		if( uart2_IT_RX_flag == SET ){
			rx  = UART_ReceiveData( MDR_UART2 ) & 0x00FF;
			uart2_IT_RX_flag = RESET;

			if ( ( (char)rx == '\n') || ( (char)rx == '\r')) {
				flag = 0;
			} else {
				buf[i] = (char)rx;
				UART2_SendStr( &buf[i] );
				if( i == MAX_UART_LINE ) flag = 0;
				i++;
			}
		}
	}
}

/*******************************************************************************
* Function Name  : UART1_IRQHandler
* Description    : This function handles UART1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART1_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : UART2_IRQHandler
* Description    : This function handles UART2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART2_IRQHandler(void)
{

	BaseType_t xHigherPriorityTaskWoken;

	uint16_t rx;

	// We have not woken a task at the start of the ISR.
	xHigherPriorityTaskWoken = pdFALSE;

	if (UART_GetITStatusMasked(MDR_UART2, UART_IT_RX) == SET)
	{

		rx  = UART_ReceiveData( MDR_UART2 ) & 0x00FF;
		xQueueSendFromISR( xUART2RXQueue, &rx, &xHigherPriorityTaskWoken );
	}

	UART_ClearITPendingBit(MDR_UART2, UART_IT_RX);
	//NVIC_ClearPendingIRQ(UART2_IRQn);

	// Now the buffer is empty we can switch context if necessary.
	if( xHigherPriorityTaskWoken )
	{
		/* Actual macro used here is port specific. */
		//portYIELD_FROM_ISR ();
	}
}

