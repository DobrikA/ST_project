#include "dma_routine.h"
#include "stm32f4xx.h"

/**
	*	@brief
	*/
void DMA2EnableSupply(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
}
/**
	*	@brief
	*/
void DMA1EnableSupply(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
}
