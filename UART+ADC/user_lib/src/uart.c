#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>
#include "uart.h"
#include "stm32f4xx.h"

#include "FreeRTOS.h"
#include "semphr.h"

static char uart_printf_buf[RBUF_SIZE] = {0};

extern cyrcle_buffer uart_in_buf;
extern xSemaphoreHandle xSemaphore_UART_TX_END;
extern xQueueHandle xQueueStringDataOut;

///cyrcle_buffer uart_out_buf = {0,0,0};	/// USART1 DMA TX structure

/**
	*	@brief
	*/
void UART_Init(void)
{
	USART_InitTypeDef 	USART_InitStructure;
	GPIO_InitTypeDef  	GPIO_InitStructure;
	NVIC_InitTypeDef  	NVIC_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	/// enable clock for GPIOB (for USART1 RX/TX)
	GPIO_InitStructure.GPIO_Pin  = USART1_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(USART1_TX_PIN_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin  = USART1_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(USART1_RX_PIN_PORT, &GPIO_InitStructure);
	GPIOB->AFR[0] |= 0x77000000;	/// PB6, PB7
#ifdef USE_UART_DMA
	DMA_InitTypeDef DMA_InitStructure;
  DMA_DeInit(UART_DMA_RX_STREAM);
  while (DMA_GetCmdStatus(UART_DMA_RX_STREAM) != DISABLE)
	{
	}  
  DMA_InitStructure.DMA_Channel = UART_DMA_RX_CHANNEL;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&uart_in_buf.buf;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = MAX_COMMAND_SIZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  ///DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;	/// work w/0 FIFO buffer (16 bytes)
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(UART_DMA_RX_STREAM, &DMA_InitStructure);
#endif
	/// UART configure
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	/// USART1 clock enable
	USART_InitStructure.USART_BaudRate 							= 460800u;
	USART_InitStructure.USART_WordLength 						= USART_WordLength_8b;
	USART_InitStructure.USART_StopBits 							= USART_StopBits_1;
	USART_InitStructure.USART_Parity 								= USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl 	= USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	/// Interrupt configuration:
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = (uint8_t)(15);
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#ifdef USE_UART_DMA
	USART1->CR1 |= USART_CR1_IDLEIE|USART_CR1_UE;
	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);	
	DMA_Cmd(UART_DMA_RX_STREAM, ENABLE);
#else
	USART1->CR1 |= USART_CR1_RXNEIE|USART_CR1_IDLEIE|USART_CR1_UE;
#endif
}
/**
	*	@brief
	*/
void UART_printf(char *arg_list, ...)
{	
	DMA_InitTypeDef 		DMA_TX_Struct;
	NVIC_InitTypeDef  	NVIC_InitStructure;
	uint32_t str_size;
	va_list arg_buffer;
	va_start(arg_buffer, arg_list);
	memset(uart_printf_buf, 0, sizeof(uart_printf_buf));
	str_size = vsprintf(uart_printf_buf, arg_list, arg_buffer);
	/// Configure DMA for send data
	DMA_DeInit(UART_DMA_TX_STREAM);
  while (DMA_GetCmdStatus(UART_DMA_TX_STREAM) != DISABLE)
	{
	}  
  DMA_TX_Struct.DMA_Channel = UART_DMA_TX_CHANNEL;
  DMA_TX_Struct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
  DMA_TX_Struct.DMA_Memory0BaseAddr = (uint32_t)&uart_printf_buf;
  DMA_TX_Struct.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_TX_Struct.DMA_BufferSize = str_size;
  DMA_TX_Struct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_TX_Struct.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_TX_Struct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_TX_Struct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_TX_Struct.DMA_Mode = DMA_Mode_Normal;
  DMA_TX_Struct.DMA_Priority = DMA_Priority_Low;
	DMA_TX_Struct.DMA_FIFOMode = DMA_FIFOMode_Disable;	/// work w/0 FIFO buffer (16 bytes)
  DMA_TX_Struct.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_TX_Struct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_TX_Struct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(UART_DMA_TX_STREAM, &DMA_TX_Struct);
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStructure.NVIC_IRQChannel = UART_DMA_TX_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = (uint8_t)(15);
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	DMA_ClearFlag(UART_DMA_TX_STREAM, UART_DMA_TX_FULL_FLAG);
	DMA_ITConfig(UART_DMA_TX_STREAM, DMA_IT_TC, ENABLE);
	DMA_Cmd(UART_DMA_TX_STREAM, ENABLE);
	if (xSemaphoreTake(xSemaphore_UART_TX_END, portMAX_DELAY) != pdPASS)
	{
	}
	va_end(arg_buffer);
}
