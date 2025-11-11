/**
 * \file watchdog.c
 * \brief Funciones Watchdog y Heartbit.
 *
 * \version 1.0
 * \copyright Copyright (c) 2017 ULMA Embedded Solutions. All rights reserved.
 *
 *                       ULMA Embedded Solutions
 *                     ---------------------------
 *                        Embedded Design House
 *
 *                     http://www.ulmaembedded.com
 *                        info@ulmaembedded.com
 *
 *******************************************************************************
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 ******************************************************************************/


/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "pin_mux.h"
#include "fsl_gpio.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "fsl_wdog32.h"

#include "AG5_define.h"
#include "main.h"
#include "rtos_time.h"

#include "watchdog.h"


/*******************************************************************************
 * Definiciones
 ******************************************************************************/


/*******************************************************************************
 * Variables Globales
 ******************************************************************************/


/*******************************************************************************
 * Variables Privadas
 ******************************************************************************/

static WDOG_Type *wdog32_base = WDOG;
static wdog32_config_t config;
static TickType_t tick_count;


/*******************************************************************************
 * Funciones
 ******************************************************************************/

/**
 * @brief Función para hacer toggle del heartbit.
 */
static void heartbeat_toggle(void)
{
	GPIO_PortToggle(LED_5VDC_GPIO, 1U << LED_5VDC_PIN);
}


static void watchdog_init(void)
{
	/*
	 * config.enableWdog32 = true;
	 * config.clockSource = kWDOG32_ClockSource1; -> 128 kHz LPO Clock (LPO_CLK)
	 * config.prescaler = kWDOG32_ClockPrescalerDivide1;
	 * config.testMode = kWDOG32_TestModeDisabled;
	 * config.enableUpdate = true;
	 * config.enableInterrupt = false;
	 * config.enableWindowMode = false;
	 * config.windowValue = 0U;
	 * config.timeoutValue = 0xFFFFU;
	 */
	WDOG32_GetDefaultConfig(&config);

	config.prescaler = kWDOG32_ClockPrescalerDivide256; // 128kHz/256 = 500Hz
	config.timeoutValue = 30000; // 500Hz*60 = 30000

	WDOG32_Init(wdog32_base, &config);
}


void watchdog_task(void *pvParameters)
{
	(void)pvParameters; // Not used

	watchdog_init();

	for (;;)
	{
		EventBits_t uxBits;

		// Esperar a que las tareas notifiquen que siguen vivas
		uxBits = xEventGroupWaitBits(wdt_events, EV_WDT_MAIN, pdTRUE, pdTRUE, TIME_200MSEC);
		if (uxBits & (EV_WDT_MAIN))
		{
			WDOG32_Refresh(wdog32_base);
		}

		if ((xTaskGetTickCount() - tick_count) >= TIME_200MSEC)
		{
			tick_count = xTaskGetTickCount();
			heartbeat_toggle();
		}
	}
}
