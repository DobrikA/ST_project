#ifndef __GPIO_TIMER_H
#define __GPIO_TIMER_H

#include <inttypes.h>
#include "stm32f4xx.h"

void SetAdcFrequency(TIM_TypeDef *TIM_NUM, uint32_t frequency);
void GpioCntrlTimerInit(void);

#endif
