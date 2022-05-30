#include <string.h>
#include "header.h"
#include "uart.h"
#include "dev_rtos_routine.h"
#include "crc_calc.h"
#include "adc_routine.h"
#include "adc_timer.h"
#include "dma_routine.h"
#include "gpio_timer.h"

#include "FreeRTOS.h"
#include "task.h"

/**
	*	@brief	Main function
	*/
int main(void)
{
	DMA2EnableSupply();	/// Enable DMA2 for ADC1 and USART1
	UART_Init();
	///printf("Hello world\r\n");
	///printf("Max payload = %d\r\n", MAX_COMMAND_SIZE);
	///printf("CRC test: 0x%X\r\n", crc8_calculation((uint8_t*)"123456789", strlen("123456789"), 0x31));
	ADCInit();
	AdcTimerInit();	
	DeviceInit();
	DevRtosRoutineInit();
	///printf("Free space in the heap (rtos) = %zd bytes\r\n", xPortGetFreeHeapSize());
	vTaskStartScheduler();
	while(1)
	{
	}
}
