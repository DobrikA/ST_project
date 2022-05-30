#ifndef __DEV_RTOS_ROUTINE_H
#define __DEV_RTOS_ROUTINE_H

#include <inttypes.h>

/**
	* @brief Element for exchange data between UART+DMA+RX
	* and for 
	*/
typedef struct
{
	uint8_t			*data;	/// pointer to data
	uint32_t 		size;		/// data size
}queue_in_data;

/// For create FreeRTOS tasks we should to add name, stack size, priority
#define DecodeInputDataTaskName 				"In_data"
#define DecodeInputDataTaskStackSize		250
#define DecodeInputDataTaskPrio 				(tskIDLE_PRIORITY + 1)

#define AdcAvgCalcTaskName							"ADC"
#define AdcAvgCalcTaskStackSize					250
#define AdcAvgCalcTaskPrio 							(tskIDLE_PRIORITY + 1)

#define PortReloadTaskName							"PORT_RELOAD"
#define PortReloadTaskStackSize					250
#define PortReloadTaskPrio 							(tskIDLE_PRIORITY + 1)

void DevRtosRoutineInit(void);

#endif
