#ifndef __IRQ_ROUTINE_H
#define __IRQ_ROUTINE_H

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SysTick_Handler(void);
#if 1
	void ADC_DMA_IRQ_HANDLER(void);
#else
	void ADC_IRQHandler(void);
#endif
void USART1_IRQHandler(void);
void TIM3_IRQHandler(void);
void UART_DMA_TX_IRQ_HANDLER(void);

#endif
