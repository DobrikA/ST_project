#include "gpio_cntrl.h"
#include "header.h"
#include "uart.h"

/**
	*	@brief Donwload port state to GPIOx->ODR
	* @param *init - pointer to struct with type of PORT_Cntrl
	*/
void GpioSetCntrl(PORT_Cntrl * init)
{
	///printf("Port: 0x%X\r\n", (uint16_t)init->port.state);
	
	///GPIO_Write(init->port_name, init->port.state);	/// download port state to GPIOx->ODR
}
