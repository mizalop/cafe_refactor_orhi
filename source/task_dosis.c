/**
 * \file task_dosis.c
 * \brief Funciones para configurar las dosis de 1 y 2 cafés.
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

#include <stdio.h>

#include "board.h"
#include "fsl_debug_console.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "rtos_time.h"

#include "main.h"
#include "AG5_define.h"
#include "eeprom.h"
#include "teclado.h"
#include "hmi.h"
#include "task_main.h"
#include "task_dosis.h"


/*******************************************************************************
 * Definiciones
 ******************************************************************************/

// Limites para programación de dosis.
// *********** MODELO QUIMBOA **************
#define DOSIS_MAX_S1_QBA    7000  // 7s
#define DOSIS_MAX_S2_QBA    12000 // 12s
#define DOSIS_MINIMA_S      1000  // 1s  --> Pruebas de vida de componentes 0,1s
#define DOSIS_INC_DEC       50    // 50ms
#define DOSIS_FAST_INC_DEC  100   // 100ms

#define TIEMPO_MAXIMO_SIN_TECLA		15000	// 15 segundos

#define DOSIS_QUEUE_MAX_ITEMS  2                    // Maximum number of of items on Motor queue
#define DOSIS_QUEUE_ITEM_SIZE  sizeof (DOSIS_MSG_t) // Motor queue item size in bytes

/**
 * @brief Estructura de mensaje para dosis.
 */
typedef struct
{
	uint8_t      servicio; /**< Tipo de servicio */
} DOSIS_MSG_t;


/*******************************************************************************
 * Variables Privadas
 ******************************************************************************/

static uint16_t dosis_prov_c1 = 0;
static uint16_t dosis_prov_c2 = 0;

static xQueueHandle handle_queue_dosis = 0;
static DOSIS_MSG_t dosis_msg;


/*******************************************************************************
 * Funciones
 ******************************************************************************/

/**
 * @brief Función que lee las dosis C1 y C2 desde la EEPROM.
 */
static void leer_dosis(void)
{
	// Leo las dosis almacenadas en formato tiempo: centésimas
	dosis_prov_c1 = leer_word_eeprom(DIRECC_DOSIS_1C);
	dosis_prov_c2 = leer_word_eeprom(DIRECC_DOSIS_2C);
}


/**
 * @brief Función que graba las dosis C1 y C2 en la EEPROM.
 */
static void grabar_dosis(void)
{
	grabar_word_eeprom(DIRECC_DOSIS_1C, dosis_prov_c1);
	grabar_word_eeprom(DIRECC_DOSIS_2C, dosis_prov_c2);
}


/**
 * @brief Función que manda una acción a un motor
 * @see   Fichero Header para ver los detalles de cómo usar esta función.
 */
uint8_t programar_dosis(TIPO_SERVICIO_e servicio)
{
	dosis_msg.servicio = servicio;
	return xQueueSendToBack(handle_queue_dosis, &dosis_msg, 0);
}


/**
 * @brief Función establece y graba las dosis C1 y C2 a los valores por defecto.
 * @see   Fichero Header para ver los detalles de cómo usar esta función.
 */
void borrar_dosis(void)
{
	// Pongo valores de fábrica en RAM.
	dosis_prov_c1 = DOSIS_1C_FAB;
	dosis_prov_c2 = DOSIS_2C_FAB;
	grabar_dosis();
}


/**
 * @brief Función que comprueba el estado correcto de las dosis.
 * @see   Fichero Header para ver los detalles de cómo usar esta función.
 */
uint8_t dosis_ok(void)
{
	// Leo las dosis almacenadas en formato tiempo: centésimas
	leer_dosis();

	if (dosis_prov_c1 > DOSIS_MAX_S1_QBA) return 0;
	if (dosis_prov_c2 > DOSIS_MAX_S2_QBA) return 0;
	if (dosis_prov_c1 < DOSIS_MINIMA_S) return 0;
	if (dosis_prov_c2 < DOSIS_MINIMA_S) return 0;

	if (dosis_prov_c1 > dosis_prov_c2) return 0;

	return 1; // Correcto
}


/**
 * @brief Tarea del autómata de configuración de dosis.
 * @see   Fichero Header para ver los detalles de cómo usar esta función.
 */
void dosis_task(void *pvParameters)
{
	TECLADO_MSG_t teclado_msg;
	while(handle_queue_dosis == 0)
	{
		// Crear cola de mensajes Dosis
		handle_queue_dosis = xQueueCreate(DOSIS_QUEUE_MAX_ITEMS, DOSIS_QUEUE_ITEM_SIZE);
		vTaskDelay(TIME_10MSEC);
	}

	uint8_t dosis_enable = 0;
	char str[8];

	for(;;)
	{
		// Leer mensajes en la cola (esperar indefinidamente)
		if (xQueueReceive(handle_queue_dosis, &dosis_msg, portMAX_DELAY) == pdPASS)
		{
			dosis_enable = 0;
			leer_dosis();
			limpiar_pantalla();
			switch (dosis_msg.servicio)
			{
			case SIMPLE:
				// Mostrar dato en pantalla
				sprintf(str, "C1:%2u.%03u", dosis_prov_c1/1000, dosis_prov_c1%1000);
				escribir_ln_str(str, LINEA_1, PARPADEO_NO);
				while (xQueueReceive(handle_queue_teclado, &teclado_msg, TIEMPO_MAXIMO_SIN_TECLA) == pdPASS)
				{
					if (teclado_msg.pulsacion_larga == 0) // Habilitar modificacion de dosis cuando se deje de pulsar la tecla
					{
						dosis_enable = 1;
					}
					if (dosis_enable == 1)
					{
						if (teclado_msg.tecla == TECLA_C2)
						{
							if (teclado_msg.pulsacion_larga == 1) //todo absurdo. si no se quita el if anterior nunca va a entrar aquí. también en doble y con la tecla C1
							{
								dosis_prov_c1+=DOSIS_FAST_INC_DEC; // Incrementos de 0,1s
							}
							else
							{
								dosis_prov_c1+=DOSIS_INC_DEC; // Incrementos de 0,05s
							}
						}
						else if (teclado_msg.tecla == TECLA_C1)
						{
							if (teclado_msg.pulsacion_larga == 1)
							{
								dosis_prov_c1-=DOSIS_FAST_INC_DEC; // Decrementos de 0,1s
							}
							else
							{
								dosis_prov_c1-=DOSIS_INC_DEC; // Decrementos de 0,05s
							}
						}
					}

					// Permitimos a c1 variar el rango completo de MIN a MAX.
					// Hay que programar primero esta dosis c1.
					// Si ponemos limites a las dos se vuelve confusa la programacion.
					if (dosis_prov_c1 > DOSIS_MAX_S1_QBA)
					{
						dosis_prov_c1 = DOSIS_MINIMA_S;
					}
					else if (dosis_prov_c1 < DOSIS_MINIMA_S)
					{
						dosis_prov_c1 = DOSIS_MAX_S1_QBA;
					}
					else if (teclado_msg.tecla == TECLA_AVANCE)
					{
						// PREMOLIDO (FAST): La variacion de C2 la limitamos dentro de margenes
						// fijados por C1 segun el criterio:  C1 < C2 < 2*C1
						// 2018-12-11: Modificacion anadida a peticion de VILAMATIC (Novell).
						// Si hemos hecho C1 demasiado pequena, hay que bajar C2.
						// o1.4: Sólo se queda libre de márgenes en ondemand y sin café en dosificador
						if (!((leer_word_eeprom(DIRECC_MODO_TRABAJO) == ONDEMAND) && (leer_word_eeprom(DIRECC_FASTCHANGE_ON) == 0)
								&& (leer_word_eeprom(DIRECC_ESTADO_TRAMPILLA) == NO_CAFE)))
						{
							if (dosis_prov_c2 > (dosis_prov_c1*2)) dosis_prov_c2 = dosis_prov_c1*2;
						}
						// Los dos casos PREMOLIDO y ONDEMAND
						// Si hemos hecho C1 demasiado grande, hay que igualar C2.
						if (dosis_prov_c2 < dosis_prov_c1) dosis_prov_c2 = dosis_prov_c1;
						// Grabar dosis
						grabar_dosis();
						break;
					}

					// Mostrar dato en pantalla
					sprintf(str, "C1:%2u.%03u", dosis_prov_c1/1000, dosis_prov_c1%1000);
					escribir_ln_str(str, LINEA_1, PARPADEO_NO);

					xEventGroupSetBits(wdt_events, EV_WDT_MAIN);
				}
				break;

			case DOBLE:
				// Mostrar dato en pantalla
				sprintf(str, "C2:%2u.%03u", dosis_prov_c2/1000, dosis_prov_c2%1000);
				escribir_ln_str(str, LINEA_1, PARPADEO_NO);
				while (xQueueReceive(handle_queue_teclado, &teclado_msg, TIEMPO_MAXIMO_SIN_TECLA) == pdPASS)
				{
					if (teclado_msg.pulsacion_larga == 0) // Habilitar modificacion de dosis cuando se deje de pulsar la tecla
					{
						dosis_enable = 1;
					}
					if (dosis_enable == 1)
					{
						if (teclado_msg.tecla == TECLA_C2)
						{
							if (teclado_msg.pulsacion_larga == 1)
							{
								dosis_prov_c2+=DOSIS_FAST_INC_DEC; // Incrementos de 0,1s
							}
							else
							{
								dosis_prov_c2+=DOSIS_INC_DEC; // Incrementos de 0,05s
							}

							// PREMOLIDO (FAST): La variación de C2 la limitamos dentro de márgenes
							// fijados por C1 según el criterio:  C1 < C2 < 2*C1
							// 2018-12-11: Modificacion añadida a petición de VILAMATIC (Novell).
							// o1.4: Sólo se queda libre de márgenes en ondemand y sin café en dosificador
							if (!((leer_word_eeprom(DIRECC_MODO_TRABAJO) == ONDEMAND) && (leer_word_eeprom(DIRECC_FASTCHANGE_ON) == 0)
									&& (leer_word_eeprom(DIRECC_ESTADO_TRAMPILLA) == NO_CAFE)))
							{
								if ((dosis_prov_c2 > (dosis_prov_c1*2)) || (dosis_prov_c2 > DOSIS_MAX_S2_QBA))
								{
									dosis_prov_c2 = dosis_prov_c1;
								}
							}
							else // ONDEMAND (FRESH): Solo límite máximo
							{
								if (dosis_prov_c2 > DOSIS_MAX_S2_QBA)
								{
									dosis_prov_c2 = dosis_prov_c1;
								}
							}
						}
						else if (teclado_msg.tecla == TECLA_C1)
						{
							if (teclado_msg.pulsacion_larga == 1)
							{
								dosis_prov_c2-=DOSIS_FAST_INC_DEC; // Decrementos de 0,1s
							}
							else
							{
								dosis_prov_c2-=DOSIS_INC_DEC; // Decrementos de 0,05s
							}

							if ((dosis_prov_c2 < dosis_prov_c1) || (dosis_prov_c2 < DOSIS_MINIMA_S))
							{
								// PREMOLIDO (FAST): La variación de C2 la limitamos dentro de márgenes
								// fijados por C1 según el criterio:  C1 < C2 < 2*C1.
								// o1.4: Sólo se queda libre de márgenes en ondemand y sin café en dosificador
								if (!((leer_word_eeprom(DIRECC_MODO_TRABAJO) == ONDEMAND) && (leer_word_eeprom(DIRECC_FASTCHANGE_ON) == 0)
										&& (leer_word_eeprom(DIRECC_ESTADO_TRAMPILLA) == NO_CAFE)))
								{
									dosis_prov_c2 = dosis_prov_c1*2;
									if (dosis_prov_c2 > DOSIS_MAX_S2_QBA)
									{
										dosis_prov_c2 = DOSIS_MAX_S2_QBA;
									}
								}
								// ONDEMAND (FRESH): Solo límite máximo
								else dosis_prov_c2 = DOSIS_MAX_S2_QBA;
							}
						}
						else if (teclado_msg.tecla == TECLA_AVANCE)
						{
							// Grabar dosis
							grabar_dosis();
							break;
						}
					}

					// Mostrar dato en pantalla
					sprintf(str, "C2:%2u.%03u", dosis_prov_c2/1000, dosis_prov_c2%1000);
					escribir_ln_str(str, LINEA_1, PARPADEO_NO);

					xEventGroupSetBits(wdt_events, EV_WDT_MAIN);
				}
				break;

			default:
				break;
			}

			xEventGroupSetBits(task_events, EV_DOSIS);
		}
	}
}
