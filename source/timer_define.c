/*
 * timer_define.c
 *
 *  Created on: 7 jun. 2022
 *      Author: mikel
 */

#include "FreeRTOS.h"
#include "main.h"
#include "queue.h"
#include "rtos_time.h"
#include "timer_define.h"

// Array de Software Timers
TimerHandle_t xTimers[NUM_TIMERS];

void vTimerCallback(TimerHandle_t xTimer)
{
	/* Optionally do something if the pxTimer parameter is NULL. */
	configASSERT(xTimer);

	xTimerStop(xTimer, 0);

	if((uint32_t)pvTimerGetTimerID(xTimer) == ID_ERRORPOWER)
	{
		setVar(ESTADO_AG, AG_EST_SOFT_INI);
		xEventGroupSetBits(task_events, EV_ERRORPOWER_OFF);
	} else if (xTimer == xTimers[TIMER_C1C2])
	{
		setVar(TIPO_SERVICIO_PREV, NUM_SERV); // Resetear el tipo de servicio previo
	}
}

uint8_t ucTimerActive(uint8_t id_timer)
{
	if ((uint32_t)pvTimerGetTimerID(xTimers[TIMER_UNIV]) == (uint32_t)id_timer)
	{
		if (xTimerIsTimerActive(xTimers[TIMER_UNIV]))
			return pdTRUE;
	}
	return pdFALSE;
}

void vStopUnivTimer(uint8_t id_timer)
{
	if (((uint32_t)pvTimerGetTimerID(xTimers[TIMER_UNIV]) == (uint32_t) id_timer) \
			&& (xTimerIsTimerActive(xTimers[TIMER_UNIV]) != pdFALSE))
		xTimerStop(xTimers[TIMER_UNIV],0);
}

/*  En alguna ocasión muy concreta y sólo con el timer FILTROMAG se ha quedado el periodo
 *  sin modificar (presumiblemente por cuestiones de prioridades en FreeRTOS. Se añaden
 *  chequeos para verificar que el proceso va adelante de manera correcta. No se emplean
 *  en las llamadas a función que ya estaban funcionando pero sí tras cambiar de FILTROMAG a MAG
 *  para asegurar de que lo hace bien. También se podría dar un tiempo de bloqueo para asegurar
 *  que puede hacer el cambio (actualmente 0 ticks) y no hacer las comprobaciones.
 *
 *  PELIGRO: que devuelva false en alguna situación sin hacer nada con el timer. en todo caso,
 *  si ocurriera eso ahora significaría que anteriormente utilizaba un periodo falso y el problema
 *  es similar
 * */
void vStartUnivTimer(uint32_t id_timer)
{
	TickType_t check_periodo;

	xTimerStop(xTimers[TIMER_UNIV], TIME_5MSEC);

	switch(id_timer)
	{
		case ID_DOBLEPULS:
			check_periodo = TIEMPO_SEGUNDO_CLICK;
			break;
		case ID_MAG:
			check_periodo = TIEMPO_MAXIMO_MAG;
			break;
		case ID_ERRORPOWER:
			check_periodo = TIEMPO_ERRORPOWER;
			break;
		case ID_FILTROMAG:
			check_periodo = TIEMPO_FILTROMAG;
			break;
		default:
			break;
	}
	xTimerChangePeriod(xTimers[TIMER_UNIV], check_periodo, TIME_5MSEC);
	vTaskDelay(TIME_10MSEC);

	vTimerSetTimerID(xTimers[TIMER_UNIV], (void *)id_timer);
	vTaskDelay(TIME_10MSEC);

	xTimerStart(xTimers[TIMER_UNIV],TIME_5MSEC);

	/*if(((uint32_t)pvTimerGetTimerID(xTimers[TIMER_UNIV]) != (uint32_t) id_timer) ||\
			(xTimerGetPeriod(xTimers[TIMER_UNIV]) != check_periodo))
		return pdFALSE;
	return pdTRUE;*/
}

void vCreaTimers(void)
{
	xTimers[TIMER_C1C2] = xTimerCreate("Timer C1C2", TIEMPO_RETENCION_SERVICIO, pdFALSE, ( void * ) 0, vTimerCallback);
	if (leer_word_eeprom(DIRECC_MODO_IDENTIF) == DOBLE_CLICK)
		xTimers[TIMER_UNIV] = xTimerCreate("Timer Univ.", TIEMPO_SEGUNDO_CLICK, pdFALSE, ( void * ) ID_DOBLEPULS, vTimerCallback);
	else
		xTimers[TIMER_UNIV] = xTimerCreate("Timer Univ.", TIEMPO_MAXIMO_MAG, pdFALSE, ( void * ) ID_MAG, vTimerCallback);
}
