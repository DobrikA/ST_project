#ifndef __UART_H
#define __UART_H

#include <inttypes.h>
#include "dev_data_routine.h"

#define RBUF_SIZE						500
#define MAX_COMMAND_SIZE		260
	
typedef struct 
{
	uint32_t 		in;
  uint32_t 		out;
  uint8_t 		buf[RBUF_SIZE];
}cyrcle_buffer;

#define USART1_RX_PIN					GPIO_Pin_7		/// PB7
#define USART1_TX_PIN					GPIO_Pin_6		/// PB6
#define USART1_RX_PIN_PORT		GPIOB
#define USART1_TX_PIN_PORT		GPIOB

#ifdef USE_UART_DMA
	/// for DMA transfers
	#define UART_DMA_RX_CHANNEL				DMA_Channel_4
	#define UART_DMA_RX_STREAM				DMA2_Stream2
	#define UART_DMA_RX_FULL_FLAG			DMA_FLAG_TCIF2

	#define UART_DMA_TX_CHANNEL				DMA_Channel_4
	#define UART_DMA_TX_STREAM				DMA2_Stream7
	#define UART_DMA_TX_FULL_FLAG			DMA_FLAG_TCIF7
	#define UART_DMA_TX_IRQ_HANDLER		DMA2_Stream7_IRQHandler
	#define UART_DMA_TX_IRQ 				  DMA2_Stream7_IRQn
#endif

void UART_Init(void);
void UART_printf(char *arg_list, ...);
void SendDataToHost(queue_in_data *out_data);

#endif
