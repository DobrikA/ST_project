#include "adc_timer.h"
#include "stm32f4xx.h"
#include "header.h"
#include "uart.h"

/**
	*	@brief Configure TIM2 for clocking ADC1 (TIM2_TRGO, output compare)
	* frequncy of ADC1 conversion - 100 Hz
	*/
void AdcTimerInit(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_DeInit(TIM2);
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	/// APB1 Fmax = SystemCoreClock/2u for timers
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	/// time of 1 tick - devider for timer clock: (SystemCoreClock/2)/TIM_ClockDivision/TIM_Prescaler:	
	TIM_TimeBaseStructure.TIM_Prescaler = 8400u - 1u;	// timer CNT frequency is 10000 Hz
	TIM_TimeBaseStructure.TIM_Period 		= 100u - 1u;	// timer period is 10 ms (100 Hz)
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);	/// tim2 update event starts ADC1 conversion
	TIM_Cmd(TIM2, ENABLE);
}

