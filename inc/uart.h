#ifndef __UART_H
#define __UART_H

#include "MDR32F9Qx_port.h"
#include "MDR32F9Qx_rst_clk.h"
#include "MDR32F9Qx_uart.h"
#include <string.h>

#define UART2_RX_BUF_SIZE	256
#define MAX_UART_LINE		UART2_RX_BUF_SIZE

extern uint8_t uart2_rx_buf[UART2_RX_BUF_SIZE];
extern uint16_t uart2_rx_buf_indx;

extern uint8_t uart2_IT_RX_flag;

void init_UART2( void );
void UART2_Send(char* buf, unsigned int buf_size);
void UART2_SendStr(char *buf);
void get_line_UART2_rx( char * buf );
char Check_UART2_RX( void );

#endif /* __UART_H */
