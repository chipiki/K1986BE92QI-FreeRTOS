/*
 * config.c
 *
 *  Created on: 28 џэт. 2019 у.
 *      Author: DA.Tsekh
 */


#include "config.h"

#include "MDR32F9Qx_config.h"
#include "MDR32F9Qx_port.h"
#include "MDR32F9Qx_rst_clk.h"
#include "MDR32F9Qx_uart.h"
#include "MDR32F9Qx_eeprom.h"
#include "MDR32F9Qx_power.h"
#include "MDR32F9Qx_adc.h"
#include "MDR32F9Qx_dma.h"
#include "MDR32F9Qx_usb_CDC.h"


void init_all_ports ( void )
{
	PORT_InitTypeDef PORT_InitStructure;

	/* Enable the RTCHSE clock on all ports */
	RST_CLK_PCLKcmd(ALL_PORTS_CLK, ENABLE);

	/* Configure all ports to the state as after reset, i.e. low power consumption */
	PORT_StructInit(&PORT_InitStructure);
	PORT_Init(MDR_PORTA, &PORT_InitStructure);
	PORT_Init(MDR_PORTB, &PORT_InitStructure);
	PORT_Init(MDR_PORTC, &PORT_InitStructure);
	PORT_Init(MDR_PORTD, &PORT_InitStructure);
	PORT_Init(MDR_PORTE, &PORT_InitStructure);
	PORT_Init(MDR_PORTF, &PORT_InitStructure);

	/* Disable the RTCHSE clock on all ports */
	RST_CLK_PCLKcmd(ALL_PORTS_CLK, DISABLE);
}


/*******************************************************************************
* Function Name  : ClockConfigure
* Description    : Configures the CPU_PLL and RTCHSE clock.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void clock_configure(void)
{
	/* Enable HSE (High Speed External) clock */
/*	RST_CLK_HSEconfig(RST_CLK_HSE_ON);
	if (RST_CLK_HSEstatus() == ERROR) {
		while (1);
	}
*/

	RST_CLK_DeInit();
#ifdef USE_HSI
	RST_CLK_HSIcmd(ENABLE);
	RST_CLK_HSIclkPrescaler(RST_CLK_HSIclkDIV1);
	RST_CLK_HSIadjust( 0 );
	/* Configures the CPU_PLL clock source */
	RST_CLK_CPU_PLLconfig(RST_CLK_CPU_PLLsrcHSIdiv1, RST_CLK_CPU_PLLmul10);
#elif USE_HSE
	  /* Enable HSE (High Speed External) clock */
	  RST_CLK_HSEconfig(RST_CLK_HSE_ON);
	  if (RST_CLK_HSEstatus() == ERROR) {
	    while (1);
	  }
		/* Configures the CPU_PLL clock source */
		RST_CLK_CPU_PLLconfig(RST_CLK_CPU_PLLsrcHSEdiv1, RST_CLK_CPU_PLLmul5);
#endif


	/* Enables the CPU_PLL */
	RST_CLK_CPU_PLLcmd(ENABLE);
	if (RST_CLK_CPU_PLLstatus() == ERROR) {
		while (1);
	}

	/* Enables the RST_CLK_PCLK_EEPROM */
	RST_CLK_PCLKcmd(RST_CLK_PCLK_EEPROM, ENABLE);
	/* Sets the code latency value */
	EEPROM_SetLatency(EEPROM_Latency_3);

	/* Select the CPU_PLL output as input for CPU_C3_SEL */
	RST_CLK_CPU_PLLuse(ENABLE);
	/* Set CPUClk Prescaler */
	RST_CLK_CPUclkPrescaler(RST_CLK_CPUclkDIV1);

	/* Select the CPU clock source */
	RST_CLK_CPUclkSelection(RST_CLK_CPUclkCPU_C3);

	/* Enables the RTCHSE clock on all ports */
	RST_CLK_PCLKcmd(ALL_PORTS_CLK, ENABLE);

	MDR_RST_CLK->PER_CLOCK = 0xFFFFFFFF;

	POWER_DUccMode( POWER_DUcc_over_80MHz);

}


