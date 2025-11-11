/**
 * \file teclado.h
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

#ifndef SOURCE_TASK_TECLADO_H_
#define SOURCE_TASK_TECLADO_H_

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


//-----------------------------------------------------------------------
//        - DEFINICION DE SIMBOLOS GENERALES -
//-----------------------------------------------------------------------

// Combinaciones de teclas del teclado membrana
#define TECLA_ON              0x01                      // Tecla con barra en c�rculo
#define TECLA_C1              0x02                      // Tecla gorda con una taza
#define TECLA_C2              0x04                      // Tecla gorda con 2 tazas
#define TECLA_AVANCE          0x08                      // Tecla tecla con flecha
#define TECLA_C1C2            (TECLA_C1|TECLA_C2)		// Combinación teclas C1 y C2
#define TECLA_C1AVANCE		  (TECLA_C1|TECLA_AVANCE)	// Combinación teclas C1 y AVANCE

//-----------------------------------------------------------------------
//      - PROTOTIPOS DE FUNCIONES PUBLICAS DEL MODULO -
//-----------------------------------------------------------------------

/**
 * @brief Tarea de lectura de teclas.
 */
void teclado_task(void *pvParameters);

#endif /* SOURCE_TASK_TECLADO_H_ */
