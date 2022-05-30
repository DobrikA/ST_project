/*
    FreeRTOS V6.0.1 - Copyright (C) 2009 Real Time Engineers Ltd.

    ***************************************************************************
    *                                                                         *
    * If you are:                                                             *
    *                                                                         *
    *    + New to FreeRTOS,                                                   *
    *    + Wanting to learn FreeRTOS or multitasking in general quickly       *
    *    + Looking for basic training,                                        *
    *    + Wanting to improve your FreeRTOS skills and productivity           *
    *                                                                         *
    * then take a look at the FreeRTOS eBook                                  *
    *                                                                         *
    *        "Using the FreeRTOS Real Time Kernel - a Practical Guide"        *
    *                  http://www.FreeRTOS.org/Documentation                  *
    *                                                                         *
    * A pdf reference manual is also available.  Both are usually delivered   *
    * to your inbox within 20 minutes to two hours when purchased between 8am *
    * and 8pm GMT (although please allow up to 24 hours in case of            *
    * exceptional circumstances).  Thank you for your support!                *
    *                                                                         *
    ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    ***NOTE*** The exception to the GPL is included to allow you to distribute
    a combined work that includes FreeRTOS without being obliged to provide the
    source code for proprietary components outside of the FreeRTOS kernel.
    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public 
    License and the FreeRTOS license exception along with FreeRTOS; if not it 
    can be viewed here: http://www.freertos.org/a00114.html and also obtained 
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE. 
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

/* Работа в режиме вытесняющей многозадачности с разделением по времени */
/*#define configUSE_PREEMPTION			1*/											
/* В случае configUSE_PREEMPTION = 0 используется кооперативная многозадачность */
/*
	Значение соnfigUSE__PREEMPTION, равное 1, дает предписание ядру FreeRTOS работать
	в режиме вытесняющей многозадачности.
	Если включить режим кооперативной многозадачности то будет выполняться только одна
	задача (первая запущенная).
	Это происходит из-за того, что планировщик никогда не получает управления и не
	может запустить на выполнение другую задачу. Теперь обязанность запуска планировщика
	ложится на программиста.
	Если добавить в функцию, реализующую задачу, явный вызов планировщика API-
	функцией taskYIELD(), то переход к другой задаче становится возможным.
*/
#define configUSE_PREEMPTION			1
/* 
				Вытесняющая многозадачность без разделения времени.
		Ее идея заключается в том, что вызов планировщика происходит только в обработчиках
	прерываний. Задача выполняется до тех пор, пока не произойдет какое-
	либо прерывание. После чего она вытесняется задачей, ответственной за обработку
	внешнего события, связанного с этим прерыванием.
		Такой тип многозадачности (cooperative) более эффективен в отношении производительности, чем
	вытесняющая многозадачность с разделением времени. Процессорное время не
	тратится впустую на выполнение кода планировщика каждый квант времени.
	Для использования этого типа многозадачности макроопределение configUSE_
	PREEMPTION в файле FreeRTOSConfig.h должно быть равным 0 и каждый
	обработчик прерывания должен содержать явный вызов планировщика portYIELD_FROM_ISR().
*/

extern unsigned int SystemCoreClock;

#define INCLUDE_eTaskGetState						1	/*!< Add in C.3.4 for resume/suspend some tasks */

#define configUSE_IDLE_HOOK	        		0 /* 1 if use function vApplicationIdleHook() */
#define configUSE_TICK_HOOK	        		0 /* */
#define configCPU_CLOCK_HZ          		( ( unsigned long ) SystemCoreClock )
/* Время между переключениями задач (в Гц), например 1000 = 1 мс */
#define configTICK_RATE_HZ	        		( ( portTickType ) 1000 )
/* Количество используемых приоритетов задач */
#define configMAX_PRIORITIES						( ( unsigned portBASE_TYPE ) 2 )
/* Minimal stack size (in words) for task (for empty task) */
#define configMINIMAL_STACK_SIZE				( ( unsigned short ) 128 ) /* for IdLE Task */
/* Размер кучи (памяти) для FreeRTOS в байтах */
#define configTOTAL_HEAP_SIZE						( ( size_t ) ( 10*1024 ) )
/* Максимальный размер имени задачи (кол-во симаолов) */
#define configMAX_TASK_NAME_LEN					( 32 )
#define configUSE_TRACE_FACILITY				0
/* If = 1, use 16-bit variable for RTOS timer, if = 0, use 32-bit variable */
#define configUSE_16_BIT_TICKS					0
#define configIDLE_SHOULD_YIELD					1
/* Включение API-функций для работы с мьютексами */
#define configUSE_MUTEXES               1
/* Включение API-функций для работы с рекурсивными мьютексами */
#define configUSE_RECURSIVE_MUTEXES			1
/* Включение API-функций для работы со счётными семафорами */
#define configUSE_COUNTING_SEMAPHORES   1
#define configUSE_MALLOC_FAILED_HOOK    0
/* If use checking stack overflow set 1 (first method) or 2 (second method): 
   must write function vApplicationStackOverflowHook() */
#define configCHECK_FOR_STACK_OVERFLOW	2

/* also must se a00110.html for use other parameters */

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 				0
#define configMAX_CO_ROUTINE_PRIORITIES 	( 2 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet				1
#define INCLUDE_uxTaskPriorityGet				1
#define INCLUDE_vTaskDelete						1
#define INCLUDE_vTaskCleanUpResources			0
/* Связано с использованием ф-ий xQueueReceive() с параметром portMAX_DELAY: */
#define INCLUDE_vTaskSuspend					1
#define INCLUDE_vTaskDelayUntil					1
#define INCLUDE_vTaskDelay		        		1
/* API check stack overflow */
/* Allocate Task Stack Size */
#define INCLUDE_uxTaskGetStackHighWaterMark		0

/* This is the raw value as per the Cortex-M3 NVIC.  Values can be 255
(lowest) to 0 (1?) (highest). */
#define configKERNEL_INTERRUPT_PRIORITY         	255
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 		191 /* equivalent to 0xb0, or priority 11. */

/* This is the value being used as per the ST library which permits 16
priority values, 0 to 15.  This must correspond to the
configKERNEL_INTERRUPT_PRIORITY setting.  Here 15 corresponds to the lowest
NVIC value of 255. */
#define configLIBRARY_KERNEL_INTERRUPT_PRIORITY		15

#endif /* FREERTOS_CONFIG_H */

