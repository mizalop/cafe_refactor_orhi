/**
 * \file teclado.c
 * \brief Funciones de lectura de teclado.
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

#include "board.h"
#include "pin_mux.h"
#include "fsl_gpio.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "task_main.h"
#include "teclado.h"

//-----------------------------------------------------------------------
//        - DEFINICION DE SIMBOLOS PRIVADOS -
//-----------------------------------------------------------------------
// --- Variables hardware para leer teclas
#define LECTURA      ( (GPIO_PinRead(TEC2_GPIO, TEC2_PIN) << 4) | \
		(GPIO_PinRead(TEC4_GPIO, TEC4_PIN) << 3) | \
		(GPIO_PinRead(TEC3_GPIO, TEC3_PIN) << 2) | \
		(GPIO_PinRead(TEC1_GPIO, TEC1_PIN) << 1) | \
		(GPIO_PinRead(TEC5_GPIO, TEC5_PIN) << 0) )

// Tiempos de teclado
#define TIEMPO_TECLADO            10    // Tiempo de lectura de teclado (en milisegundos)
#define TIEMPO_TECLA_FILTRO       2     // Filtro para validar pulsación (en función de TIEMPO_TECLADO)
#define TIEMPO_TECLA_CORTO        5     // Tiempo de pulsación corta (en función de TIEMPO_TECLADO)
#define TIEMPO_TECLA_LARGO        120   // Tiempo de pulsación larga (en función de TIEMPO_TECLADO)
#define TIEMPO_TECLA_REP          30    // Tiempo de repetición de pulsación larga (en función de TIEMPO_TECLADO)

// Número de teclas
#define TECLA_MAX                 6

// Posición teclas en máscara
#define POS_C1					  1
#define POS_C2					  2
#define POS_AVANCE				  3
#define POS_C1C2				  4
#define POS_C1AVANCE			  5


//-----------------------------------------------------------------------
//          - VARIABLES PRIVADAS DEL MODULO -
//-----------------------------------------------------------------------

// --- Variables en data (RAM interna) ---
static uint32_t tiempo_teclado_high[TECLA_MAX];
static uint32_t tiempo_teclado_low[TECLA_MAX];
static uint8_t pulsacion_larga_flag[TECLA_MAX];
static uint8_t teclado_mascara[TECLA_MAX] = { TECLA_ON, TECLA_C1, TECLA_C2, TECLA_AVANCE, TECLA_C1C2, TECLA_C1AVANCE };
static uint8_t tecla;


//-----------------------------------------------------------------------
//            - FUNCIONES DEL MODULO -
//-----------------------------------------------------------------------

/**
 * @brief Función que inicializa las variables del teclado.
 */
static void inicializa_teclado(void)
{
	uint8_t i;

	for (i = 0; i < TECLA_MAX; i++)
	{
		tiempo_teclado_high[i] = 0;
		tiempo_teclado_low[i] = 0;
	}
}

/**
 * @brief Función que detecta/valida pulsaciones de teclas.
 *
 * @param dato_teclado_in [in]
 *           Dato de una fila de botones.
 */
static void tecla_pulsada(uint8_t dato_teclado_in)
{
	uint8_t dato_valido = ~(dato_teclado_in);
	uint8_t evento = 0;
	uint8_t i;

	for (i = 0; i < TECLA_MAX; i++)
	{
		if ((dato_valido & teclado_mascara[i]) == teclado_mascara[i]) // Tecla pulsada
		{
			tiempo_teclado_high[i]++;
			//filtros para que las combinaciones de tecla no activen también las dos teclas
			//  por independiente
			if (i == POS_C1 || i == POS_C2){
				tiempo_teclado_high[i] = tiempo_teclado_high[i]*(tiempo_teclado_high[POS_C1C2] == 0);
			}
			if (i == POS_C1 || i == POS_AVANCE){
				tiempo_teclado_high[i] = tiempo_teclado_high[i]*(tiempo_teclado_high[POS_C1AVANCE] == 0);
			}
			if (tiempo_teclado_high[i] > TIEMPO_TECLA_FILTRO)
			{
				if (tiempo_teclado_high[i] > TIEMPO_TECLA_LARGO) // Pulsación larga
				{
					pulsacion_larga_flag[i] = 1;
					tiempo_teclado_high[i] = TIEMPO_TECLA_LARGO - TIEMPO_TECLA_REP;
					evento = 1;
					tecla |= teclado_mascara[i];
					//PRINTF("pulsación larga: %d\r\n", i);
				}
				tiempo_teclado_low[i] = 0;
			}
		}
		else // Tecla no pulsada
		{
			tiempo_teclado_low[i]++;
			if (tiempo_teclado_low[i] > TIEMPO_TECLA_FILTRO) // Tecla soltada validada
			{
				if ((pulsacion_larga_flag[i] == 0) && (tiempo_teclado_high[i] > TIEMPO_TECLA_CORTO) && (tiempo_teclado_high[i] < TIEMPO_TECLA_LARGO)) // pulsación corta
				{
					evento = 1;
					tecla |= teclado_mascara[i];
					//PRINTF("pulsación corta: %d\r\n", i);
				}
				tecla &= !teclado_mascara[i];
				tiempo_teclado_high[i] = 0;
				pulsacion_larga_flag[i] = 0;
			}
		}

		if (evento == 1)
		{
			//PRINTF("enviar tecla: %d, %d\r\n", teclado_mascara[i], pulsacion_larga_flag[i]);
			enviar_tecla(teclado_mascara[i], pulsacion_larga_flag[i]);
			evento = 0;
		}
	}
}

/**
 * @brief Tarea de lectura de teclas.
 */
void teclado_task(void *pvParameters)
{
	uint8_t dato_teclado = 0;

	inicializa_teclado();

	for (;;)
	{
		// Captura de los datos de teclas
		dato_teclado = LECTURA;

		tecla_pulsada(dato_teclado);

		vTaskDelay(TIEMPO_TECLADO);
	}
}
