#include "stm32f4xx.h"
#include "gpio_timer.h"
#include "header.h"
#include "uart.h"

/**
	*	@brief Set TIMx frequency (calculate new ARR number)
	* @param frequency - new frequency in Hz
	*/
void SetAdcFrequency(TIM_TypeDef *TIM_NUM, uint32_t frequency)
{
	uint16_t prescaler = TIM_GetPrescaler(TIM_NUM);
	uint32_t tim_period = ((SystemCoreClock/2u)/(prescaler + 1u))/frequency;
	///printf("prescaler = %d, TIM ARR = %d\r\n", prescaler, tim_period);
	TIM_SetAutoreload(TIM_NUM, tim_period);
}
/**
	*	@brief Init TIM3 (update mode + interrupt) for modify port state
	*/
void GpioCntrlTimerInit(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef  			 NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_DeInit(TIM3);
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	/// APB1 Fmax = SystemCoreClock/2u for timers
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	/// time of 1 tick - devider for timer clock: (SystemCoreClock/2)/TIM_ClockDivision/TIM_Prescaler:	
	TIM_TimeBaseStructure.TIM_Prescaler = 8400u - 1u;		/// timer frequency is 1000000 Hz
	TIM_TimeBaseStructure.TIM_Period 		= 5000u - 1u;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	/// Interrupt configuration:
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = (uint8_t)(15);
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
}
