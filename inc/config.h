/*
 * config.h
 *
 *  Created on: 28 џэт. 2019 у.
 *      Author: DA.Tsekh
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

#define USE_HSE				1
//#define USE_HSI				1

void clock_configure(void);
void init_all_ports ( void );

#define ALL_PORTS_CLK (RST_CLK_PCLK_PORTA | RST_CLK_PCLK_PORTB | \
                       RST_CLK_PCLK_PORTC | RST_CLK_PCLK_PORTD | \
                       RST_CLK_PCLK_PORTE | RST_CLK_PCLK_PORTF)


#endif /* INC_CONFIG_H_ */
