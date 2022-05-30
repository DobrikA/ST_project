#ifndef __GPIO_CNTRL_H
#define __GPIO_CNTRL_H

#include <inttypes.h>
#include "stm32f4xx.h"

/**
	*	@brief Description of 16-pinouts port via bit field
	*/	
typedef struct
{
	uint32_t 			pin0 : 1;
	uint32_t 			pin1 : 1;
	uint32_t 			pin2 : 1;
	uint32_t 			pin3 : 1;
	uint32_t 			pin4 : 1;
	uint32_t 			pin5 : 1;
	uint32_t 			pin6 : 1;
	uint32_t 			pin7 : 1;
	uint32_t 			pin8 : 1;
	uint32_t 			pin9 : 1;
	uint32_t 			pin10 : 1;
	uint32_t 			pin11 : 1;
	uint32_t 			pin12 : 1;
	uint32_t 			pin13 : 1;
	uint32_t 			pin14 : 1;
	uint32_t 			pin15 : 1;
}PORT_PIN_OUTS;

/**
	*	@brief Controller port description
	*/
typedef struct
{
	uint32_t					port_freq_reload;	/// port state/pinouts update frequency in Hz
	GPIO_TypeDef 			*port_name;				/// port type (set GPIOA/GPIOB and etc. from controller memory map)
	/* 
		For reading port state it's simpler to use one variable state, consist of 16 pinout state.
		For set each pin to 0/1 use variable "line".
	*/
	union
	{
		PORT_PIN_OUTS 	line;
		uint16_t				state;
	}port;
}PORT_Cntrl;

/**
	* @brief
	*/
void GpioSetCntrl(PORT_Cntrl * init);

#endif
