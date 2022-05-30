#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "dev_data_routine.h"
#include "dev_rtos_routine.h"
#include "stm32f4xx.h"
#include "header.h"
#include "uart.h"
#include "crc_calc.h"
#include "gpio_timer.h"
#include "FreeRTOS.h"
#include "semphr.h"

/// Command system:
/*
	A - preambule, cnt , type - B, length
*/

extern xSemaphoreHandle	xMutex_PORT_ACCESS;

static const uint8_t pack_preamble 			= 'A';	/// enter your preambula
static const uint8_t max_cnt 						= 'B';	/// enter maximum number of cnt
static const uint8_t new_port_frequency = 'D';	/// command for set new frequency for port reload
static const uint8_t new_port_state			= 'E';	/// command for set new port state

struct device_cntrl_table dev_cntrl_tbl;

typedef struct
{
	uint8_t preamble;
	uint8_t cnt;
	uint8_t type;
	uint8_t length;
}Packet;

static Packet packet;
static Packet *pack_ptr = &packet;
static cyrcle_buffer data_buf = {0,0,0};	/// array for saving input packet (after preambula detection)
static cyrcle_buffer *p = &data_buf;

/**
	*	@brief Set device start variables and modes
	*/
void DeviceInit(void)
{
	struct device_cntrl_table *p = &dev_cntrl_tbl;
	p->adc_result = 0;
	p->error_status = no_error;
	p->portA.port_name = GPIOA;
	p->portA.port_freq_reload = 1000u;
	p->portA.port.state = 0;	/// set all pins to 0
	GpioCntrlTimerInit();
	SetAdcFrequency(TIM3, p->portA.port_freq_reload);
}
/**
	* @brief Function decode input packet for execute commands
	* and send responce with modified field "type"
	*/
ErrorStatus DecodeInputData(queue_in_data *in_data)
{
	static bool preambula_flag = FALSE;	/// information about finding start of packet
	struct device_cntrl_table *dev = &dev_cntrl_tbl;
	ErrorStatus status = SUCCESS;
	if (p->in > (RBUF_SIZE - MAX_COMMAND_SIZE))
	{
		p->in = 0;
		p->out = p->in;
		status = ERROR;
	}
	else
	{		
		if (!preambula_flag)
		{
			for (uint32_t i = 0; i < (*in_data).size; i++)
			{
				if ((*in_data).data[i] == pack_preamble || ((*in_data).data[i] == pack_preamble && (*in_data).data[i + 1] == max_cnt))
				{
					/// we find start of packet in data array
					preambula_flag = TRUE;
					memcpy(&p->buf[p->in], &(*in_data).data[i], (*in_data).size - i);
					p->in += in_data->size - i;
				}
			}
			if (preambula_flag)
			{
				if ((p->in - p->out) > 4)	/// preamble + cnt + type + lenght + data/crc8
				{
					pack_ptr = (Packet *)&p->buf[p->out];
					if ((p->in - p->out) == pack_ptr->length + 4)
					{
						char str_crc[1] = {0};
						sprintf(str_crc, "%d", p->buf[p->out + 4 + pack_ptr->length - 1]);	/// was length
						uint8_t crc8 = atoi(str_crc);
						printf("Code = %d\r\n", crc8);
						printf("crc8 = %d\r\n", crc8_calculation(&p->buf[p->out], (pack_ptr->length + 4 - 1), 0x31));	/// length
						if (crc8 == crc8_calculation(&p->buf[p->out], (pack_ptr->length + 4 - 1), 0x31))	/// was length
						{
							/// Command handlers:
							if (pack_ptr->type == new_port_frequency)	/// set new frequency for port reload
							{
								if (xSemaphoreTake(xMutex_PORT_ACCESS, portMAX_DELAY) != pdPASS)	/// take mutex
								{
									dev->error_status = memory_error;
								}
								else
								{
									for (int i = 0; i < pack_ptr->length - 1; i++)
									{
										dev->portA.port_freq_reload |= (p->buf[p->out + 4 + i] << (pack_ptr->length - 1 - i)*8);
									}
									printf("New port frequency: %u\r\n", dev->portA.port_freq_reload);
									SetAdcFrequency(TIM3, dev->portA.port_freq_reload);
									if (xSemaphoreGive(xMutex_PORT_ACCESS) != pdPASS)	/// return mutex
									{
										dev->error_status = memory_error;
									}
								}
							}
							else if (pack_ptr->type == new_port_state)	/// set new port state
							{
								if (xSemaphoreTake(xMutex_PORT_ACCESS, portMAX_DELAY) != pdPASS)	/// take mutex for stop access for port variables
								{
									dev->error_status = memory_error;
								}
								else
								{
									for (int i = 0; i < pack_ptr->length - 1; i++)
									{
										dev->portA.port.state |= (p->buf[p->out + 4 + i] << (pack_ptr->length - 1 - i)*8);
									}
									if (xSemaphoreGive(xMutex_PORT_ACCESS) != pdPASS)	/// return access to port variables
									{
										dev->error_status = memory_error;
									}
								}
							}
							pack_ptr->type |= 0x80;
							p->buf[p->in - 1] = crc8_calculation(&p->buf[p->out], (pack_ptr->length + 4 - 1), 0x31);	/// was length
							printf("New CRC8: %d\r\n", p->buf[p->in - 1]);
							printf("Packet: %s\r\n", &p->buf[p->out]);
						}
						preambula_flag = FALSE;
						p->in = 0;
						p->out = p->in;
					}
				}
			}
		}
		else
		{
			memcpy(&p->buf[p->in], in_data->data, in_data->size);
			p->in += in_data->size;
			if ((p->in - p->out) > 4)
			{
				pack_ptr = (Packet *)&p->buf[p->out];	/// must to check if all packet is recieved
				if ((p->in - p->out) == pack_ptr->length + 4)
				{
					/// Command handler:
					if (pack_ptr->type == new_port_frequency)	/// set new frequency for port reload
					{
						if (xSemaphoreTake(xMutex_PORT_ACCESS, portMAX_DELAY) != pdPASS)	/// take mutex
						{
							dev->error_status = memory_error;
						}
						else
						{
							for (int i = 0; i < pack_ptr->length - 1; i++)
							{
								dev->portA.port_freq_reload |= (p->buf[p->out + 4 + i] << (pack_ptr->length - 1 - i)*8);
							}
							printf("New port frequency: %u\r\n", dev->portA.port_freq_reload);
							SetAdcFrequency(TIM3, dev->portA.port_freq_reload);
							if (xSemaphoreGive(xMutex_PORT_ACCESS) != pdPASS)	/// return mutex
							{
								dev->error_status = memory_error;
							}
						}
					}
					else if (pack_ptr->type == new_port_state)	/// set new port state
					{
						if (xSemaphoreTake(xMutex_PORT_ACCESS, portMAX_DELAY) != pdPASS)	/// take mutex for stop access for port variables
						{
							dev->error_status = memory_error;
						}
						else
						{
							for (int i = 0; i < pack_ptr->length - 1; i++)
							{
								dev->portA.port.state |= (p->buf[p->out + 4 + i] << (pack_ptr->length - 1 - i)*8);
							}
							if (xSemaphoreGive(xMutex_PORT_ACCESS) != pdPASS)	/// return access to port variables
							{
								dev->error_status = memory_error;
							}
						}
					}					
					char str_crc[1] = {0};
					sprintf(str_crc, "%d", p->buf[p->out + 4 + pack_ptr->length - 1]);	/// was lenght
					uint8_t crc8 = atoi(str_crc);
					printf("Code = %d\r\n", crc8);
					printf("crc8 = %d\r\n", crc8_calculation(&p->buf[p->out], (pack_ptr->length + 4 - 1), 0x31));	/// was length
					if (crc8 == crc8_calculation(&p->buf[p->out], (pack_ptr->length + 4 - 1), 0x31))	/// was length
					{
						pack_ptr->type |= 0x80;
						p->buf[p->in - 1] = crc8_calculation(&p->buf[p->out], (pack_ptr->length + 4 - 1), 0x31);	/// was length
						printf("New CRC8: %d\r\n", p->buf[p->in - 1]);
						printf("Packet: %s\r\n", &p->buf[p->out]);
					}
					preambula_flag = FALSE;
					p->in = 0;
					p->out = p->in;						
				}
			}			
		}
	}
	return status;
}
