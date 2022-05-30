#include "dev_rtos_routine.h"
#include "stm32f4xx.h"
#include "header.h"
#include "uart.h"
#include "adc_routine.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

static const UBaseType_t xQueueStandartSize = 32;

/// RTOS tasks, queue, semaphores and etc. init:
static void DataExchRtosVarInit(void);
static void DataExchRtosTasksInit(void);
/// RTOS tasks:
static void DecodeInputDataTask(void * pvParameters);
static void AdcAvgCalcTask(void * pvParameters);
static void PortReloadTask(void * pvParameters);

/// RTOS queue for data exchange:
xQueueHandle xQueueStringDataIn;	/// Queue for receiving USART1 data

/// RTOS semaphores and mutexes for work with interrupts
///	and separated access:
xSemaphoreHandle	xSemaphore_ADC_DATA_READY;
xSemaphoreHandle	xSemaphore_PORT_RELOAD;
xSemaphoreHandle	xMutex_PORT_ACCESS;
xSemaphoreHandle  xSemaphore_UART_TX_END;

extern cyrcle_buffer uart_in_buf;
extern struct device_cntrl_table dev_cntrl_tbl;

/**
	*	@brief
	*/
void DevRtosRoutineInit(void)
{
	DataExchRtosVarInit();
	DataExchRtosTasksInit();
}
/**
	*	@brief
	*/
static void DataExchRtosVarInit(void)
{
	/// Queue for send UART input data (position in DMA buffer) to command(packet) decode fucntion
	/// queue give/send from UART IDLE irq handler
	xQueueStringDataIn = xQueueCreate(xQueueStandartSize, sizeof(queue_in_data));
	if (xQueueStringDataIn == NULL)
	{
		/// error
	}
	/// Semaphore for adc data ready for averaging
	vSemaphoreCreateBinary(xSemaphore_ADC_DATA_READY);
	if (!xSemaphore_ADC_DATA_READY)
	{
		/// error
	}
	else
	{
		if (xSemaphoreTake(xSemaphore_ADC_DATA_READY, portMAX_DELAY) != pdPASS)
		{
			/// error
		}
	}
	/// Semaphore for reload port state in task (semaphore give from IRQ)
	vSemaphoreCreateBinary(xSemaphore_PORT_RELOAD);
	if (!xSemaphore_PORT_RELOAD)
	{
		/// error
	}
	else
	{
		if (xSemaphoreTake(xSemaphore_PORT_RELOAD, portMAX_DELAY) != pdPASS)
		{
			/// error
		}
	}
	/// Mutex for access to port state (for save reload)
	xMutex_PORT_ACCESS = xSemaphoreCreateMutex();// Make mutex for devide access for data in the list
	if (!xMutex_PORT_ACCESS)
	{
		/// error
	}
	/// Semaphore for UART DMA TC
	vSemaphoreCreateBinary(xSemaphore_UART_TX_END);
	if (!xSemaphore_UART_TX_END)
	{
		/// error
	}
	else
	{
		if (xSemaphoreTake(xSemaphore_UART_TX_END, portMAX_DELAY) != pdPASS)
		{
			/// error
		}
	}
}
/**
	*	@brief
	*/
static void DataExchRtosTasksInit(void)
{
	/// Create decoding input packet task from queue
	if (xTaskCreate(DecodeInputDataTask,
									DecodeInputDataTaskName,
									DecodeInputDataTaskStackSize,
									NULL,
									DecodeInputDataTaskPrio,
									NULL) == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
	{
		/// error
	}
	/// Create Task for reload port state
	if (xTaskCreate(PortReloadTask,
									PortReloadTaskName,
									PortReloadTaskStackSize,
									NULL,
									PortReloadTaskPrio,
									NULL) == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
	{
		/// error
	}
	/// Create ADC averaging data task
	if (xTaskCreate(AdcAvgCalcTask,
									AdcAvgCalcTaskName,
									AdcAvgCalcTaskStackSize,
									NULL,
									AdcAvgCalcTaskPrio,
									NULL) == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
	{
		/// error
	}
}
/**
	*	@brief Input command (packet) decode task
	* The task waits (blocked) data from queue. The data consists of pointer
	* to data in uart_in_buf.buf (UART DMA RX buffer) and data size.
	*/
static void DecodeInputDataTask(void * pvParameters)
{	
	(void)pvParameters;
	queue_in_data input_str;
	struct device_cntrl_table *dev = &dev_cntrl_tbl;
	for (;;)
	{
		if (xQueueReceive(xQueueStringDataIn, &input_str, portMAX_DELAY) != pdPASS)
		{
			dev->error_status = memory_error;
		}
		else
		{
			if (!DecodeInputData(&input_str))	/// try to decode input symbols from UART
			{
				dev->error_status = command_error;
			}
		}
	}
	vQueueDelete(xQueueStringDataIn);
	vTaskDelete(NULL);
}
/**
	*	@brief PORT state reload task.
	* The task waits semaphore xSemaphore_PORT_RELOAD from TIM3 interrupt.
	* TIM3 interrupt event is TIM_IT_Update. So port update period is the same as TIM3 period.
	* We use mutex to separate access for port variables (if we want to change port variables in another place)
	*/
static void PortReloadTask(void * pvParameters)
{
	(void)pvParameters;
	struct device_cntrl_table *dev = &dev_cntrl_tbl;
	for (;;)
	{
		if (xSemaphoreTake(xSemaphore_PORT_RELOAD, portMAX_DELAY) != pdPASS)
		{
			dev->error_status = memory_error;
		}
		else
		{
			if (xSemaphoreTake(xMutex_PORT_ACCESS, portMAX_DELAY) != pdPASS)	/// take mutex
			{
				dev->error_status = memory_error;
			}
			GpioSetCntrl(&dev->portA);
			if (xSemaphoreGive(xMutex_PORT_ACCESS) != pdPASS)	/// return mutex
			{
				dev->error_status = memory_error;
			}
		}
	}
	vTaskDelete(NULL);
}
/**
	*	@brief ADC data average calculation task
	*/
static void AdcAvgCalcTask(void * pvParameters)
{
	(void)pvParameters;
	struct device_cntrl_table *dev = &dev_cntrl_tbl;
	for (;;)
	{
		if (xSemaphoreTake(xSemaphore_ADC_DATA_READY, portMAX_DELAY) != pdPASS)
		{
			dev->error_status = memory_error;
		}
		else
		{
			dev->adc_result = AdcAverageCalculation();
			///printf("temp = %.3f C\r\n", ((dev->adc_result*ADC_INT_VREF/ADC_MAX_COUNT) - 0.76f)/2.5E-3f + 25.0f);
			/// Example of set bit for port;
			if (xSemaphoreTake(xMutex_PORT_ACCESS, portMAX_DELAY) != pdPASS)	/// take mutex for access to port variables
			{
				dev->error_status = memory_error;
			}
			else
			{
				dev->portA.port.state = dev->adc_result&0xFFF;	/// take 12 bits from adc result
				dev->portA.port.line.pin15 = 1;									/// example of using bit fields
				dev->portA.port.line.pin14 = 1; 								/// example of using bit fields
				if (xSemaphoreGive(xMutex_PORT_ACCESS) != pdPASS)	/// return mutex for access to port variables
				{
					dev->error_status = memory_error;
				}
			}
		}
	}
	vTaskDelete(NULL);
}
/**
	*	@brief
	*/
void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed portCHAR *pcTaskName)
{
	printf("Error: stack overflow task %s.\r\n", pcTaskName);
}
