#include <string.h>
#include "header.h"
#include "irq_routine.h"
#include "uart.h"
#include "stm32f4xx.h"
#include "dev_data_routine.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

#include "dev_data_routine.h"
#include "dev_rtos_routine.h"
#include "adc_routine.h"

extern xQueueHandle xQueueStringDataIn;
extern void xPortSysTickHandler(void);

cyrcle_buffer uart_in_buf = {0,0,0};	/// USART1 DMA RX structure

extern xSemaphoreHandle	xSemaphore_ADC_DATA_READY;
extern xSemaphoreHandle xSemaphore_PORT_RELOAD;
extern xSemaphoreHandle xSemaphore_UART_TX_END;

extern struct device_cntrl_table dev_cntrl_tbl;

/**
	*	@brief
	*/
void NMI_Handler(void)
{
	printf("NMI_Handler...\r\n");
	while(1){}
}
/**
	*	@brief
	*/
void HardFault_Handler(void)
{
	printf("HardFault_Handler...\r\n");
	while(1){}
}
/**
	*	@brief
	*/
void MemManage_Handler(void)
{
	printf("MemManage_Handler...\r\n");
	while(1){}
}
/**
	*	@brief
	*/
void BusFault_Handler(void)
{
	printf("BusFault_Handler...\r\n");
	while(1){}
}
/**
	*	@brief
	*/
void UsageFault_Handler(void)
{
	printf("UsageFault_Handler...\r\n");
	while(1){}
}
/**
	*	@brief
	*/
void SysTick_Handler(void)
{
	xPortSysTickHandler();
}
/**
	*	@brief UART DMA transmit complete interrupt handler
	*/
void UART_DMA_TX_IRQ_HANDLER(void)
{
	struct device_cntrl_table *dev = &dev_cntrl_tbl;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	if (DMA_GetFlagStatus(UART_DMA_TX_STREAM, UART_DMA_TX_FULL_FLAG))
	{
		DMA_ClearITPendingBit(UART_DMA_TX_STREAM, UART_DMA_TX_FULL_FLAG);
		if (xSemaphore_UART_TX_END != NULL)
		{
			if (xSemaphoreGiveFromISR(xSemaphore_UART_TX_END, &xHigherPriorityTaskWoken) != pdPASS)
			{
				/// error
				dev->error_status = memory_error;
			}			
		}
	}
	if (xHigherPriorityTaskWoken != pdFALSE)
	{
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	}
}
#if 1
/**
	*	@brief DMA interrupt for end of transmittion ADC conversions to memory
	*/
void ADC_DMA_IRQ_HANDLER(void)
{
	struct device_cntrl_table *dev = &dev_cntrl_tbl;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	if (DMA_GetFlagStatus(ADC_DMA_STREAM, ADC_DMA_TC_END_FLAG))
	{
		DMA_ClearITPendingBit(ADC_DMA_STREAM, ADC_DMA_TC_END_FLAG);
		if (xSemaphore_ADC_DATA_READY != NULL)
		{
			if (xSemaphoreGiveFromISR(xSemaphore_ADC_DATA_READY, &xHigherPriorityTaskWoken) != pdPASS)
			{
				/// error
				dev->error_status = memory_error;
			}
		}
	}
	if (xHigherPriorityTaskWoken != pdFALSE)
	{
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	}
}
#else
/**
	*	@brief
	*/
void ADC_IRQHandler(void)
{
	if (ADC1->SR & ADC_SR_EOC)
  {
		ADC_ClearFlag(ADC1, ADC_SR_EOC);
		uint16_t adc_data =	ADC1->DR;
		float adc_vlt = (ADC1->DR*ADC_INT_VREF)/ADC_MAX_COUNT;
	#if 0
		printf("Vlt: %.3f\r\n", adc_vlt);
	#else
		float temp = ((adc_data*3.3f/4096.0f) - 0.76f)/2.5E-3f + 25.0f;
		printf("temp = %.3f\r\n", temp);
	#endif
  }
}
#endif
/**
	*	@brief
	*/
void TIM3_IRQHandler(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	struct device_cntrl_table *dev = &dev_cntrl_tbl;
	if (TIM3->SR & TIM_IT_Update)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		if (xSemaphore_ADC_DATA_READY != NULL)
		{
			if (xSemaphoreGiveFromISR(xSemaphore_PORT_RELOAD, &xHigherPriorityTaskWoken) != pdPASS)
			{
				dev->error_status = memory_error;
			}
		}		
	}
	if (xHigherPriorityTaskWoken != pdFALSE)
	{
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	}
}
/**
	*	@brief USART1 interrupt handler
	*/
void USART1_IRQHandler(void)
{
#ifdef USE_UART_DMA	/// UART + IDLE interrupt + DMA UART RX
	struct device_cntrl_table *dev = &dev_cntrl_tbl;
	portBASE_TYPE queue_status = pdPASS;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	cyrcle_buffer *p = &uart_in_buf;
	uint16_t in_data_size = 0;
	queue_in_data message;
	static uint16_t rx_last = MAX_COMMAND_SIZE;	
	if (USART1->SR & USART_FLAG_IDLE)
	{
		/// the rx in the idle state
		USART1->SR &= ~USART_FLAG_IDLE;
		in_data_size = USART1->DR;	/// right now we have cleared IDLE interrupt flag
		if (DMA_GetFlagStatus(UART_DMA_RX_STREAM, UART_DMA_RX_FULL_FLAG) && (p->in + in_data_size) > MAX_COMMAND_SIZE)
		{
			DMA_ClearITPendingBit(UART_DMA_RX_STREAM, UART_DMA_RX_FULL_FLAG);
			rx_last = MAX_COMMAND_SIZE;
			if (xQueueStringDataIn != NULL)
			{
				message.data 	= &p->buf[p->out];
				message.size 	= rx_last - p->in;	/// send all data before end of buffer
				queue_status = xQueueSendToBackFromISR(xQueueStringDataIn, &message, &xHigherPriorityTaskWoken);
				if (queue_status != pdPASS)
				{
					dev->error_status = memory_error;
					printf("Error: xQueueComPortReceive(USART1) is FULL\r\n");
				}
				else
				{
					in_data_size = rx_last - DMA_GetCurrDataCounter(UART_DMA_RX_STREAM);
					rx_last -= in_data_size;
					p->out = 0;
					p->in = in_data_size;
					message.data 	= &p->buf[p->out];
					message.size 	= p->in - p->out;
					queue_status = xQueueSendToBackFromISR(xQueueStringDataIn, &message, &xHigherPriorityTaskWoken);
					if (queue_status != pdPASS)
					{
						/// error
						printf("Error: xQueueComPortReceive(USART1) is FULL\r\n");
						dev->error_status = memory_error;
					}
					else
					{
						p->out = p->in;
					}
				}
			}
			else
			{
				dev->error_status = memory_error;
			}
		}
		else
		{
			in_data_size = rx_last - DMA_GetCurrDataCounter(UART_DMA_RX_STREAM);
			rx_last -= in_data_size;
			p->in += in_data_size;
			if (xQueueStringDataIn != NULL)
			{
				message.data 	= &p->buf[p->out];	/// set pointer to the data begin
				message.size 	= (p->in - p->out);	/// set size of data in bytes
				queue_status = xQueueSendToBackFromISR(xQueueStringDataIn, &message, &xHigherPriorityTaskWoken);
				if (queue_status != pdPASS)
				{
					/// error
					dev->error_status = memory_error;
					printf("Error: xQueueComPortReceive(USART1) is FULL\r\n");
				}
				else
				{
					p->out = p->in;
				}
			}
			else
			{
				/// error
				dev->error_status = memory_error;
			}
		}
	}
	if (xHigherPriorityTaskWoken != pdFALSE)
	{
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	}
#else	/// UART + IDLE interrupt w/o DMA
	portBASE_TYPE queue_status = pdPASS;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	cyrcle_buffer *p = &uart_in_buf;
	uint8_t RxData = 0;
	queue_in_data message;
	if (USART1->SR & USART_FLAG_RXNE)
	{
		// rx one byte complited
		USART1->SR &= ~USART_FLAG_RXNE;
		p->buf[p->in] = USART1->DR;
		p->in++;
	}
	else if (USART1->SR & USART_FLAG_IDLE)
	{
		/// the rx in the idle state
		USART1->SR &= ~USART_FLAG_IDLE;
		RxData = USART1->DR;	/// right now we have cleared IDLE interrupt flag
		if (xQueueStringDataIn != NULL)	/// check if the xQueueStringDataIn queue is valid
		{
			message.data 	= &p->buf[p->out];	/// set pointer to the data begin
			message.size 	= (p->in - p->out);	/// set size of data in bytes
			queue_status = xQueueSendToBackFromISR(xQueueStringDataIn, &message, &xHigherPriorityTaskWoken);
			if (queue_status != pdPASS)
			{
				printf("Error: xQueueComPortReceive(USART1) is FULL\r\n");
			}
			else
			{
				if (p->in > (RBUF_SIZE - MAX_COMMAND_SIZE))
				{
					p->in = 0;
				}
				p->out = p->in;
			}
		}
		else
		{
			printf("xQueueStringDataIn error...\r\n");
		}
	}
	if (xHigherPriorityTaskWoken != pdFALSE)
	{
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	}
#endif
}
