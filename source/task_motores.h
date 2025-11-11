/**
 * \file task_motores.h
 * \brief Funciones para controlar los motores de las trampillas y el motor del molino
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

#ifndef SOURCE_TASK_MOTORES_H_
#define SOURCE_TASK_MOTORES_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include <stdint.h>
#include "pin_mux.h"
#include "fsl_gpio.h"

/*******************************************************************************
 * Definiciones
 ******************************************************************************/

/**
 * @brief Motores de las trampillas
 */
typedef enum
{
	MOTOR_SUPERIOR = 0, /**< Motor superior */
	MOTOR_INFERIOR, /**< Motor inferior */
	MOTOR_SUPERIOR_INFERIOR /**< Motor superior e inferior */
} MOTOR_e;

/**
 * @brief Acciones de los motores de las trampillas
 */
typedef enum
{
	ACCION_CERRAR = 0, /**< Cerrar trampilla */
	ACCION_ABRIR /**< Abrir trampilla */
} ACCION_e;

#define _motor_ac_on_()   GPIO_PinWrite(GRINDER_GPIO, GRINDER_PIN, 0)
#define _motor_ac_off_()  GPIO_PinWrite(GRINDER_GPIO, GRINDER_PIN, 1)

// Tiempos de autómatas
#define TIEMPO_MOVER_TRAMP       50 // Unidades de 1ms

#define DUTY_FASE1               100
#define DUTY_FASE2               15

/*******************************************************************************
 * Funciones
 ******************************************************************************/

/**
 * @brief Función que manda una acción a un motor
 *
 * @param motor [in]
 *           motor al que se quiere mandar la acción.
 * @param accion [in]
 *           acción que se quiere mandar.
 *
 * @return pdTRUE si se ha ejecutado correctamente, si no errQUEUE_FULL.
 */
BaseType_t accion_motor(MOTOR_e motor, ACCION_e accion);

/**
 * @brief Tarea del autómata de motores de trampilla superior e inferior.
 */
void motores_task(void *pvParameters);

#endif /* SOURCE_TASK_MOTORES_H_ */
