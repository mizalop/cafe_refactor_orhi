/**
 * \file task_motores.c
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

/*******************************************************************************
 * Includes
 ******************************************************************************/

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "rtos_time.h"
#include "main.h"
#include "AG5_define.h"
#include "pwm.h"

#include "task_motores.h"

/*******************************************************************************
 * Definiciones
 ******************************************************************************/

/**
 * @brief Estructura de mensaje para motores.
 */
typedef struct
{
	MOTOR_e motor; /**< Motor o motores */
	ACCION_e accion; /**< Acción que se quiere ejecutar */
} MOTOR_MSG_t;

// Control de los motores DC 24V (Mabuchi) de trampillas
// Controles NEGADOS: con 0 se activan, con 1 están en reposo
#define COM_OPEN(x)  GPIO_PinWrite(COM_OP_GPIO, COM_OP_PIN, x);         // Comun OPEN --> Q4  Rama común JP1 JP2 (+)
#define M1_OPEN(x)   pwm_set_duty(PWM_M1_OPEN_FTM_CHANNEL,   (!x)*100); // M1 OPEN --> Q10 Motor inferior JP1 (-)
#define M2_OPEN(x)   pwm_set_duty(PWM_M2_OPEN_FTM_CHANNEL,   (!x)*100); // M2 OPEN --> Q11 Motor superior JP2 (-)
#define COM_CLOSE(x) pwm_set_duty(PWM_COM_CLOSE_FTM_CHANNEL, (!x)*100); // Comun CLOSE --> Q9 Rama común JP1 JP2 (-)
#define M1_CLOSE(x)  GPIO_PinWrite(M1_CL_GPIO, M1_CL_PIN, x);           // M1 CLOSE --> Q2 Motor inferior JP1 (+)
#define M2_CLOSE(x)  GPIO_PinWrite(M2_CL_GPIO, M2_CL_PIN, x);           // M2 CLOSE --> Q3 Motor superior JP2 (+)

#define MOTOR_QUEUE_MAX_ITEMS  2                                        // Maximum number of of items on Motor queue
#define MOTOR_QUEUE_ITEM_SIZE  sizeof (MOTOR_MSG_t)                     // Motor queue item size in bytes

/*******************************************************************************
 * Variables Privadas
 ******************************************************************************/

static xQueueHandle handle_queue_motor = 0;
static MOTOR_MSG_t motor_msg;

/*******************************************************************************
 * Funciones
 ******************************************************************************/

/**
 * @brief Función que desactiva los motores
 */
static void desactiva_motores(void)
{
	pwm_set_duty(PWM_COM_CLOSE_FTM_CHANNEL, 0); // Primero los canales "N"
	pwm_set_duty(PWM_M1_OPEN_FTM_CHANNEL, 0);
	pwm_set_duty(PWM_M2_OPEN_FTM_CHANNEL, 0);

	COM_OPEN(0);
	M1_CLOSE(0);
	M2_CLOSE(0);
}

/**
 * @brief Función que manda una acción a un motor
 */
BaseType_t accion_motor(MOTOR_e motor, ACCION_e accion)
{
	motor_msg.motor = motor;
	motor_msg.accion = accion;

	return xQueueSendToBack(handle_queue_motor, &motor_msg, 0);
}

/**
 * @brief Tarea del autómata de motores de trampilla superior e inferior.
 */
void motores_task(void *pvParameters)
{
	while (handle_queue_motor == 0)
	{
		// Crear cola de mensajes Motor
		handle_queue_motor = xQueueCreate(MOTOR_QUEUE_MAX_ITEMS, MOTOR_QUEUE_ITEM_SIZE);
		vTaskDelay(TIME_10MSEC);
	}

	for (;;)
	{
		// Leer mensajes en la cola (esperar indefinidamente)
		if (xQueueReceive(handle_queue_motor, &motor_msg, portMAX_DELAY) == pdPASS)
		{
			desactiva_motores();

			switch (motor_msg.accion)
			{
			case ACCION_ABRIR:
				switch (motor_msg.motor)
				{
				case MOTOR_SUPERIOR:
					COM_OPEN(1);                                       // Rama común abrir
					pwm_set_duty(PWM_M2_OPEN_FTM_CHANNEL, DUTY_FASE1); // Motor superior (M2) abrir
					vTaskDelay(TIEMPO_MOVER_TRAMP);
					pwm_set_duty(PWM_M2_OPEN_FTM_CHANNEL, DUTY_FASE2); // Motor superior (M2) PWM
					vTaskDelay(TIEMPO_MOVER_TRAMP);
					break;
				case MOTOR_INFERIOR:
					COM_OPEN(1);                                       // Rama común abrir
					pwm_set_duty(PWM_M1_OPEN_FTM_CHANNEL, DUTY_FASE1); // Motor inferior (M1) abrir
					vTaskDelay(TIEMPO_MOVER_TRAMP);
					pwm_set_duty(PWM_M1_OPEN_FTM_CHANNEL, DUTY_FASE2); // Motor inferior (M1) PWM
					vTaskDelay(TIEMPO_MOVER_TRAMP);
					break;
				case MOTOR_SUPERIOR_INFERIOR:
					COM_OPEN(1);                                       // Rama común abrir
					pwm_set_duty(PWM_M2_OPEN_FTM_CHANNEL, DUTY_FASE1); // Motor superior (M2) abrir
					pwm_set_duty(PWM_M1_OPEN_FTM_CHANNEL, DUTY_FASE1); // Motor inferior (M1) abrir
					vTaskDelay(TIEMPO_MOVER_TRAMP);
					pwm_set_duty(PWM_M2_OPEN_FTM_CHANNEL, DUTY_FASE2); // Motor superior (M2) PWM
					pwm_set_duty(PWM_M1_OPEN_FTM_CHANNEL, DUTY_FASE2); // Motor inferior (M1) PWM
					vTaskDelay(TIEMPO_MOVER_TRAMP);
					break;
				default:
					break;
				}
				break;

			case ACCION_CERRAR:
				switch (motor_msg.motor)
				{
				case MOTOR_SUPERIOR:
					M2_CLOSE(1);                                         // Motor superior (M2) cerrar
					pwm_set_duty(PWM_COM_CLOSE_FTM_CHANNEL, DUTY_FASE1); // Rama comun cerrar
					vTaskDelay(TIEMPO_MOVER_TRAMP);
					pwm_set_duty(PWM_COM_CLOSE_FTM_CHANNEL, DUTY_FASE2); // Rama comun PWM
					vTaskDelay(TIEMPO_MOVER_TRAMP);
					break;
				case MOTOR_INFERIOR:
					M1_CLOSE(1);                                         // Motor inferior (M1) cerrar
					pwm_set_duty(PWM_COM_CLOSE_FTM_CHANNEL, DUTY_FASE1); // Rama comun cerrar
					vTaskDelay(TIEMPO_MOVER_TRAMP);
					pwm_set_duty(PWM_COM_CLOSE_FTM_CHANNEL, DUTY_FASE2); // Rama comun PWM
					vTaskDelay(TIEMPO_MOVER_TRAMP);
					break;
				case MOTOR_SUPERIOR_INFERIOR:
					M2_CLOSE(1);                                         // Motor superior (M2) cerrar
					M1_CLOSE(1);                                         // Motor inferior (M1) cerrar
					pwm_set_duty(PWM_COM_CLOSE_FTM_CHANNEL, DUTY_FASE1); // Rama comun cerrar
					vTaskDelay(TIEMPO_MOVER_TRAMP);
					pwm_set_duty(PWM_COM_CLOSE_FTM_CHANNEL, DUTY_FASE2); // Rama comun PWM
					vTaskDelay(TIEMPO_MOVER_TRAMP);
					break;
				default:
					break;
				}
				break;

				default:
					break;
			}
			desactiva_motores();

			xEventGroupSetBits(task_events, EV_MOTOR);
		}
	}
}
