/**
 * \file task_dosis.h
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

#ifndef SOURCE_TASK_DOSIS_H_
#define SOURCE_TASK_DOSIS_H_


/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "AG5_define.h"


/*******************************************************************************
 * Definiciones
 ******************************************************************************/

// Dosis de fábrica en tiempo:
// Tiempos en miléismas de segundo
#define DOSIS_1C_FAB        1800  // 1,8s
#define DOSIS_2C_FAB        3100  // 3,1s


/*******************************************************************************
 * Funciones
 ******************************************************************************/

/**
 * @brief Función que manda una acción a un motor
 *
 * @param servicio [in]
 *           tipo de servicio que se quiere programar.
 *
 * @return pdTRUE si se ha ejecutado correctamente, si no errQUEUE_FULL.
 */
uint8_t programar_dosis(TIPO_SERVICIO_e servicio);

/**
 * @brief Función establece y graba las dosis C1 y C2 a los valores por defecto.
 */
void borrar_dosis(void);

/**
 * @brief Función que comprueba el estado correcto de las dosis.
 *
 * @return 1 si la comprobación ha sido correcta, si no 0.
 */
uint8_t dosis_ok(void);

/**
 * @brief Tarea del autómata de configuración de dosis.
 */
void dosis_task(void *pvParameters);

#endif /* SOURCE_TASK_DOSIS_H_ */
