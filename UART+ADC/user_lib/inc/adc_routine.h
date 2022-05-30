#ifndef __ADC_ROUTINE_H
#define __ADC_ROUTINE_H

#include <math.h>
#include <inttypes.h>

#define ADC_MAX_COUNT 	pow(2,12)
#define ADC_INT_VREF		3.3f

/// Defines for DMA2
#define ADC_DMA_STREAM				DMA2_Stream0
#define ADC_DMA_CHANNEL				DMA_Channel_0
#define ADC_DMA_IRQ_HANDLER		DMA2_Stream0_IRQHandler
#define ADC_DMA_IRQ						DMA2_Stream0_IRQn
#define ADC_DMA_TC_END_FLAG		DMA_FLAG_TCIF0
#define ADC_AVG_SIZE					32u	/// ADC1 samples for averaging

void ADCInit(void);
uint32_t AdcAverageCalculation(void);

#endif
