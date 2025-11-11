/*
 * timer_define.h
 *
 *  Created on: 7 jun. 2022
 *      Author: mikel
 */

#ifndef TIMER_DEFINE_H_
#define TIMER_DEFINE_H_

#include "timers.h"
#include "eeprom.h"
#include "AG5_define.h"
#include "task_main.h"
#include "rtos_time.h"

//  Para la identificación de porta por doble click:
//  Tiempo mínimo que se suelta el micro para contar como doble clik.
#define TIEMPO_FILTRO_REBOTES     TIME_20MSEC // en milisegundos
//  Tiempo máximo desde que se suelta hasta que se vuelve a pulsar
//  para validar el servicio doble
#define TIEMPO_SEGUNDO_CLICK      TIME_300MSEC// en milisegundos
#define TIEMPO_RETENCION_SERVICIO TIME_10SEC  // en milisegundos
#define TIEMPO_MAXIMO_MAG         TIME_1MIN   // en milisegundos
#define TIEMPO_ERRORPOWER         TIME_1MIN   // 1 min de mensaje al volver de reset
//  Cuando se deja porta colocado en molino, para evitar problema de
//  autoajuste de offset de MAG, en 4 segundos se apaga la tarjeta.
#define TIEMPO_FILTROMAG		  TIME_5SEC

typedef enum //Diferentes timers que se cubren con el universal
{
	ID_DOBLEPULS = 0,
	ID_MAG,
	ID_ERRORPOWER,
	ID_FILTROMAG
} UNIVTIMER_e;

typedef enum            // Timers del RTOS
{
	TIMER_C1C2 = 0,       // Timer para mantener selección de C1 o C2
	TIMER_UNIV,           // Timer universal para: detección de sensor magnético
						  // /segundo click de porta/gestión mensaje Error power
	NUM_TIMERS            // Número máximo de timers
} TIMER_e;

// Array de Software Timers
extern TimerHandle_t xTimers[NUM_TIMERS];

//Funciones
void vTimerCallback(TimerHandle_t xTimer);

uint8_t ucTimerActive(uint8_t id_timer);

void vStopUnivTimer(uint8_t id_timer);

void vStartUnivTimer(uint32_t id_timer);

void vCreaTimers(void);


#endif /* TIMER_DEFINE_H_ */
