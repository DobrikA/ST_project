#ifndef __DEV_DATA_ROUTINE_H
#define __DEV_DATA_ROUTINE_H

#include <inttypes.h>
#include "stm32f4xx.h"
#include "dev_rtos_routine.h"
#include "gpio_cntrl.h"

/**
	*	@brief Device error status description
	*/
typedef enum
{
	no_error = 0u,
	memory_error = 1u,
	command_error = 2u,
	ERR_STATUS_NUMBERS = 2u
}ERR_STATUS;

/**
	*	@brief Device state description table
	*/
struct device_cntrl_table
{
	uint32_t				adc_result;			/// ADC average result after measurement
	PORT_Cntrl			portA;					/// Device port (connected to MCU GPIOA)
	ERR_STATUS			error_status;		/// Device error status
};

ErrorStatus DecodeInputData(queue_in_data *in_data);
void DeviceInit(void);

#endif
