#include "adc_routine.h"
#include "stm32f4xx.h"
#include "header.h"
#include "uart.h"
#include "adc_timer.h"

static volatile uint16_t AdcDataBuffer[ADC_AVG_SIZE] = {0};

/**
	*	@brief 	Calculate ADC average result from DMA buffer AdcDataBuffer
	* @retval adc average data
	*/
uint32_t AdcAverageCalculation(void)
{
	uint32_t avg_result = 0;
	for (uint32_t i = 0; i < ADC_AVG_SIZE; i++)
	{
		avg_result += AdcDataBuffer[i];
	}
	avg_result /= ADC_AVG_SIZE;	
	return avg_result;
}

/**
	*	@brief ADC + DMA init for measurement
	*/
void ADCInit(void)
{
	ADC_InitTypeDef 				ADC_InitStructure;	
	ADC_CommonInitTypeDef 	ADC_CommonInitStructure;
	DMA_InitTypeDef 				DMA_InitStructure;
	NVIC_InitTypeDef 				NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

#if 1
	/// Configure DMA for taking data from ADC to memory
	/// we use DMA2 for sending ADC1 conversions results to memory AdcDataBuffer[ADC_AVG_SIZE]
  DMA_DeInit(ADC_DMA_STREAM);
  while (DMA_GetCmdStatus (ADC_DMA_STREAM) != DISABLE)
	{
	}  
  DMA_InitStructure.DMA_Channel = ADC_DMA_CHANNEL;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&AdcDataBuffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = ADC_AVG_SIZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(ADC_DMA_STREAM, &DMA_InitStructure);
  DMA_Cmd(ADC_DMA_STREAM, ENABLE);

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStructure.NVIC_IRQChannel = ADC_DMA_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = (uint8_t)(14);
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	DMA_ITConfig(ADC_DMA_STREAM, DMA_IT_TC, ENABLE);
#endif

	/* ADC1 configure for measurement stm32 core temperature*/
	ADC_DeInit();	
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;		/// for singles measurements
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;	/// for APB2 Fmax = (SystemCoreClock/2)/ADC_Prescaler_Div2 = 42 MHz
	ADC_CommonInitStructure.ADC_DMAAccessMode = DISABLE;				/// not use for independent mode
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;	/// minimal delay
	ADC_CommonInit(&ADC_CommonInitStructure);
	
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;				/// one channel mode conversion
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	/// single measurement
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;	/// for start from trigger
	ADC_InitStructure.ADC_NbrOfConversion = 1;	/// one conversion, regular group
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; /// data format
	ADC_Init(ADC1, &ADC_InitStructure);
	
	/// ADC_Channel_17 - use internal reference like source for ADC (stm32f427)
	/// ADC_Channel_18 - use internal temperature sensor like source for ADC (stm32f427)
	ADC_RegularChannelConfig(ADC1, ADC_Channel_18, 1, ADC_SampleTime_15Cycles);
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);	
#if 0	/// if we work w/o DMA - ADC1 + interrupts
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = (uint8_t)(14);
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);	/// End of conversion interrupt
#endif	
	ADC_DMACmd(ADC1, ENABLE);
  ADC_Cmd(ADC1, ENABLE);
	ADC_TempSensorVrefintCmd(ENABLE);	/// Enable temperature sensor and internal Vref (+1.2V)
}
