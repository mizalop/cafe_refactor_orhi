/**
 * \file task_cafe.c
 * \brief Módulo de autómata de molido y dosificación
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

#include "main.h"
#include "eeprom.h"
#include "AG5_define.h"
#include "motores.h"
#include "task_dosis.h"
#include "hmi.h"
#include "rtos_time.h"
#include "avisoFresas.h"
#include "task_main.h"

#include "task_cafe.h"


/*******************************************************************************
 * Definiciones
 ******************************************************************************/

// Estructura mensaje moler_dosificar
typedef struct
{
	MODO_TRABAJO_e      modo;
	MODO_TRABAJO_e      modo_prev;
	ESTADO_TRAMPILLAS_e estado_tramp;
	TIPO_SERVICIO_e     servicio;
} ORHI_MSG_t;

#define ORHI_QUEUE_MAX_ITEMS   2		             // Maximum number of of items on Motor queue
#define ORHI_QUEUE_ITEM_SIZE   sizeof(ORHI_MSG_t)    // Motor queue item size in bytes

#define TIEMPO_DESCARGA        TIME_500MSEC
#define TIEMPO_VISUALIZACION   TIME_500MSEC

#define MAXIMO_CONTADORES	   100000000			 // Si no se ha grabado antes vale  0xFFFFFFFF=4.294.967.295

/*******************************************************************************
 * Variables Privadas
 ******************************************************************************/

static uint32_t dosis_1c = DOSIS_1C_FAB;      // Tiempo de dosificación en milisegundos
static uint32_t dosis_2c = DOSIS_2C_FAB;
static uint32_t tiempo_moliendo = 0;

static xQueueHandle handle_queue_orhi = 0;
static ORHI_MSG_t orhi_msg;


/*******************************************************************************
 * Funciones
 ******************************************************************************/

static void incrementar_contadores(uint8_t servicio)
{
	uint32_t contador;

	switch (servicio)
	{
	case SIMPLE:
		// Leer, incrementar y grabar contador C1
		contador = leer_int_eeprom(DIRECC_CONTADOR_1);
		contador++;
		grabar_int_eeprom(DIRECC_CONTADOR_1, contador);
		// Leer, incrementar y grabar contador T1
		contador = leer_int_eeprom(DIRECC_CONTADOR_T1);
		contador++;
		grabar_int_eeprom(DIRECC_CONTADOR_T1, contador);
		break;

	case DOBLE:
		// Leer, incrementar y grabar contador C2
		contador = leer_int_eeprom(DIRECC_CONTADOR_2);
		contador++;
		grabar_int_eeprom(DIRECC_CONTADOR_2, contador);
		// Leer, incrementar y grabar contador T2
		contador = leer_int_eeprom(DIRECC_CONTADOR_T2);
		contador++;
		grabar_int_eeprom(DIRECC_CONTADOR_T2, contador);
		break;

	default:
		break;
	}
	if (servicio <= DOBLE) decr_cont(servicio + 1);
	// decrementa contador de aviso de cambio de fresas.
	//   servicio es 0 si es SIMPLE, 1 si es DOBLE. en microdosis, etc. no se decrementa.
}

// Cuando hay que hacer un servicio, desde el main_task llaman a esta
// rutina para poner en el buzón el mensaje correspondiente.
uint8_t moler_dosificar(MODO_TRABAJO_e modo, TIPO_SERVICIO_e servicio)
{
	orhi_msg.modo = modo;
	orhi_msg.servicio = servicio;
	if (xQueueSendToBack(handle_queue_orhi, &orhi_msg, 0) == pdPASS)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void borrar_contadores(CONTADORES_e tipo_cont)
{
	switch (tipo_cont)
	{
	case PARCIALES:
		grabar_int_eeprom(DIRECC_CONTADOR_1, 0);
		grabar_int_eeprom(DIRECC_CONTADOR_2, 0);
		break;

	case TOTALES:
		grabar_int_eeprom(DIRECC_CONTADOR_T1, 0);
		grabar_int_eeprom(DIRECC_CONTADOR_T2, 0);
		break;

	default:
		break;
	} // switch
}  // borrar_contadores


uint8_t cont_servicios_ok(void)
{
	if (leer_int_eeprom(DIRECC_CONTADOR_1) > MAXIMO_CONTADORES) return 0;
	if (leer_int_eeprom(DIRECC_CONTADOR_2) > MAXIMO_CONTADORES) return 0;
	return 1; // Correcto
}


uint8_t cont_totales_ok(void)
{
	if (leer_int_eeprom(DIRECC_CONTADOR_T1) > MAXIMO_CONTADORES) return 0;
	if (leer_int_eeprom(DIRECC_CONTADOR_T2) > MAXIMO_CONTADORES) return 0;
	return 1; // Correcto
}

static inline uint8_t dosis_simple(){
	if ((getVar(TIPO_SERVICIO) == SERV_SIMPLE) || (getVar(TIPO_SERVICIO) == SERV_AUTOMATICO_1) \
			|| (getVar(TIPO_SERVICIO) == SERV_SIMPLE_PROV) || (getVar(TIPO_SERVICIO) == SERV_PROV))
		return pdTRUE;

	return pdFALSE;
}
static inline uint8_t dosis_doble(){
	if ((getVar(TIPO_SERVICIO) == SERV_DOBLE) || (getVar(TIPO_SERVICIO) == SERV_AUTOMATICO_2) \
			|| (getVar(TIPO_SERVICIO) == SERV_DOBLE_PROV))
		return pdTRUE;

	return pdFALSE;
}

void moler_dosificar_task(void *pvParameters)
{
	while (handle_queue_orhi == 0)
	{
		// Create the queue (N item x M bytes)
		handle_queue_orhi = xQueueCreate(ORHI_QUEUE_MAX_ITEMS, ORHI_QUEUE_ITEM_SIZE);
		vTaskDelay(TIME_10MSEC);
	}

	for(;;)
	{
		// check new messages on queue
		if (xQueueReceive(handle_queue_orhi, &orhi_msg, portMAX_DELAY) == pdPASS)
		{
			// Leer la dosis almacenada en milésimas
			dosis_1c = leer_word_eeprom(DIRECC_DOSIS_1C);
			dosis_2c = leer_word_eeprom(DIRECC_DOSIS_2C);
			switch (orhi_msg.servicio)
			{
			case DOBLE:
				// Elegir secuencia: PREMOLIDO (FAST) o INSTANTANEO (FRESH)
				if (orhi_msg.modo == PREMOLIDO)
				// FAST o PREMOLIDO: dosificar y recargar trampillas
				{
					// Abrir motor superior y esperar a que termine
					accion_motor(MOTOR_SUPERIOR, ACCION_ABRIR);
					//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);

					// Abrir motor inferior y esperar a que termine
					accion_motor(MOTOR_INFERIOR, ACCION_ABRIR);
					//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);

					escribir_ln_id_box(eTXT_EMPTY_BOX, getVar(TIPO_SERVICIO));
					vTaskDelay(TIEMPO_DESCARGA);

					// Cerrar motor inferior y esperar a que termine
					accion_motor(MOTOR_INFERIOR, ACCION_CERRAR);
					//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);

					// Encender Motor AC
					_motor_ac_on_();

					// Esperar "dosis_1c" milisegundos
					tiempo_moliendo = dosis_1c;
					escribir_ln_id_llenarbox(eTXT_C2_PREM, LINEA_2, tiempo_moliendo);
					vTaskDelay(tiempo_moliendo);

					// Apagar Motor AC
					_motor_ac_off_();

					vTaskDelay(TIEMPO_DESCARGA);

					// Cerrar motor superior y esperar a que termine
					accion_motor(MOTOR_SUPERIOR, ACCION_CERRAR);
					//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);

					// Esperamos que la tarea Lcd este libre, podría estar acabando la animacion
					// habría que asegurarse de que hay lcd si se metiera un display diferente:
					// if(lcd()) xEventGroup... o sustituir EV_LCD por EV_FINANIMACION
					xEventGroupWaitBits(task_events, EV_LCD, pdTRUE, pdFALSE, portMAX_DELAY);

					// Calcular tiempo de molido restante
					tiempo_moliendo = dosis_2c - dosis_1c;

					// Encender Motor AC si el tiempo restante es superior a 0
					if (tiempo_moliendo != 0)
					{
						_motor_ac_on_();
					}

					escribir_ln_id_llenarbox(eTXT_C2_PREM, LINEA_1, tiempo_moliendo);
					// Esperar "dosis_2c - dosis_1c" milisegundos
					vTaskDelay(tiempo_moliendo);

					// Apagar Motor AC
					_motor_ac_off_();

					incrementar_contadores(orhi_msg.servicio);

					vTaskDelay(TIEMPO_VISUALIZACION); //Para que de tiempo a ver el valor molido
					// Esperamos que la tarea Lcd esté libre, podría estar acabando la animacion
					// habría que asegurarse de que hay lcd si se metiera un display diferente:
					// if(lcd()) xEventGroup... o sustituir EV_LCD por EV_FINANIMACION
					xEventGroupWaitBits(task_events, EV_LCD, pdTRUE, pdFALSE, portMAX_DELAY);

					escribir_ln_id_box(eTXT_PREG, getVar(TIPO_SERVICIO));
				}
				else
				// FRESH o INSTANTANEO: Muele la dosis sin retener trampilla y dosifica.
				{
					if (leer_word_eeprom(DIRECC_ESTADO_TRAMPILLA) == NO_CAFE)
					// Dosificador vacío ya estaba en FRESH --> Moler dosis doble
					{
						// Encender Motor AC
						_motor_ac_on_();

						// Esperar "dosis_2c" milisegundos
						tiempo_moliendo = dosis_2c;

						escribir_ln_id_llenarbox(eTXT_C2_ONDEM, LINEA_1_2, tiempo_moliendo);
						vTaskDelay(tiempo_moliendo);

						// Apagar Motor AC
						_motor_ac_off_();

						incrementar_contadores(orhi_msg.servicio);

						// Esperamos que la tarea Lcd este libre, podría estar acabando la animacion
						// habría que asegurarse de que hay lcd si se metiera un display diferente:
						// if(lcd()) xEventGroup... o sustituir EV_LCD por EV_FINANIMACION
						xEventGroupWaitBits(task_events, EV_LCD, pdTRUE, pdFALSE, portMAX_DELAY);
					}
					else if (leer_word_eeprom(DIRECC_ESTADO_TRAMPILLA) == T1CAFE)
					//Café solo en trampilla inferior --> Moler diferencia
					{
						// Calcular tiempo de molido restante
						tiempo_moliendo = dosis_2c - dosis_1c;

						// Encender Motor AC si el tiempo restante es superior a 0
						if (tiempo_moliendo != 0)
						{
							_motor_ac_on_();
						}

						escribir_ln_id_llenarbox(eTXT_C2_ONDEM, LINEA_1, tiempo_moliendo);
						// Esperar "dosis_2c - dosis_1c" milisegundos
						vTaskDelay(tiempo_moliendo);

						// Apagar Motor AC
						_motor_ac_off_();

						// Esperamos que la tarea Lcd este libre, podría estar acabando la animacion
						// habría que asegurarse de que hay lcd si se metiera un display diferente:
						// if(lcd()) xEventGroup... o sustituir EV_LCD por EV_FINANIMACION
						xEventGroupWaitBits(task_events, EV_LCD, pdTRUE, pdFALSE, portMAX_DELAY);
					}
					else if (leer_word_eeprom(DIRECC_ESTADO_TRAMPILLA) == T1T2CAFE)
					// Venimos de FAST Café arriba y abajo --> No moler nada
					{
						// Abrir motor superior y esperar a que termine
						accion_motor(MOTOR_SUPERIOR, ACCION_ABRIR);
						//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);
					}
					// Abrir motor inferior y esperar a que termine
					accion_motor(MOTOR_INFERIOR, ACCION_ABRIR);
					//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);

					escribir_ln_id_box(eTXT_EMPTY_BOX, getVar(TIPO_SERVICIO));
					vTaskDelay(TIEMPO_DESCARGA);
					escribir_ln_id_box(eTXT_ONDE, getVar(TIPO_SERVICIO));

					// Cerrar motor inferior y esperar a que termine
					accion_motor(MOTOR_INFERIOR, ACCION_CERRAR);
					//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);

					// Guardar dato de dosificador vacío
					grabar_word_eeprom(DIRECC_ESTADO_TRAMPILLA, NO_CAFE);

					// Fin de cambio de premolido (FAST) a instantaneo (FRESH)
				} //else FRESH o sea instantaneo
				// Fin servicio, reset de teclado, flag fin secuencia
				if ((xEventGroupGetBits(task_events) & EV_INTENSIVA_ON) == 0 ) //si estamos en prueba intensiva no resetea teclado
					xQueueReset(handle_queue_teclado);						   //  para detectar cuando se quiere parar la prueba
				xEventGroupSetBits(task_events, EV_FIN_SERV);
				break; // case DOBLE:

			case SIMPLE:
				// Otra vez elegir secuencia: PREMOLIDO (FAST) o INSTANTANEO (FRESH)
				if (orhi_msg.modo == PREMOLIDO)
				// FAST o PREMOLIDO: dosificar y recargar trampillas
				{
					// Abrir motor inferior y esperar a que termine
					accion_motor(MOTOR_INFERIOR, ACCION_ABRIR);
					//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);

					escribir_ln_id_box(eTXT_EMPTY_BOX, getVar(TIPO_SERVICIO));
					vTaskDelay(TIEMPO_DESCARGA);

					// Cerrar motor inferior y esperar a que termine
					accion_motor(MOTOR_INFERIOR, ACCION_CERRAR);
					//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);

					// Abrir motor superior y esperar a que termine
					accion_motor(MOTOR_SUPERIOR, ACCION_ABRIR);
					//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);

					// Preparo la dosis a sumar sobre lo que había en la trampilla
					// superior para obtener la dosis de 1 café.
					tiempo_moliendo = dosis_1c * 2 - dosis_2c;

					// Si la dosis C2 es justo doble que C1 no muelo.
					if (tiempo_moliendo != 0)
					{
						_motor_ac_on_();
					}

					escribir_ln_id_llenarbox(eTXT_C1_PREM, LINEA_2, tiempo_moliendo);
					// Esperar "dosis_1c * 2 - dosis_2c" milisegundos
					vTaskDelay(tiempo_moliendo);

					// Apagar Motor AC
					_motor_ac_off_();

					vTaskDelay(TIEMPO_DESCARGA);

					// Cerrar motor superior y esperar a que termine
					accion_motor(MOTOR_SUPERIOR, ACCION_CERRAR);
					//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);

					// Esperamos que la tarea  Oled este libre, podría estar acabando la animacion
					// habría que asegurarse de que hay lcd si se metiera un display diferente:
					// if(lcd()) xEventGroup... o sustituir EV_LCD por EV_FINANIMACION
					xEventGroupWaitBits(task_events, EV_LCD, pdTRUE, pdFALSE, portMAX_DELAY);

					// Valor para la trampilla superior
					tiempo_moliendo = dosis_2c - dosis_1c;

					// Si la dosis C2 es igual que C1 tiempo_moliendo es cero.
					if (tiempo_moliendo != 0)
					{
						_motor_ac_on_();
					}

					escribir_ln_id_llenarbox(eTXT_C1_PREM, LINEA_1, tiempo_moliendo);
					// Esperar "dosis_2c - dosis_1c" milisegundos
					vTaskDelay(tiempo_moliendo);

					// Apagar Motor AC
					_motor_ac_off_();

					vTaskDelay(TIEMPO_VISUALIZACION); //Para que de tiempo a ver el valor molido
					// Esperamos que la tarea  Oled este libre, podría estar acabando la animacion
					// habría que asegurarse de que hay lcd si se metiera un display diferente:
					// if(lcd()) xEventGroup... o sustituir EV_LCD por EV_FINANIMACION
					xEventGroupWaitBits(task_events, EV_LCD, pdTRUE, pdFALSE, portMAX_DELAY);

					// Fin servicio: pantalla, contadores
					escribir_ln_id_box(eTXT_PREG, getVar(TIPO_SERVICIO));
					incrementar_contadores(orhi_msg.servicio);
				}
				else
				// FRESH o INSTANTANEO: Muele la dosis sin retener trampilla y dosifica.
				{
					if (leer_word_eeprom(DIRECC_ESTADO_TRAMPILLA) == NO_CAFE)
					// Vacío totalmente, ya estaba en FRESH --> Moler dosis 1c
					{
						// Encender Motor AC
						_motor_ac_on_();

						// Esperar "dosis_1c" milisegundos
						tiempo_moliendo = dosis_1c;

						escribir_ln_id_llenarbox(eTXT_C1_ONDEM, LINEA_2, tiempo_moliendo);
						vTaskDelay(tiempo_moliendo);

						// Apagar Motor AC
						_motor_ac_off_();

						// Abrir motor inferior y esperar a que termine
						accion_motor(MOTOR_INFERIOR, ACCION_ABRIR);
						//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);

						vTaskDelay(TIEMPO_DESCARGA);
						// Esperamos que la tarea  Lcd este libre, podría estar acabando la animacion
						// habría que asegurarse de que hay lcd si se metiera un display diferente:
						// if(lcd()) xEventGroup... o sustituir EV_LCD por EV_FINANIMACION
						xEventGroupWaitBits(task_events, EV_LCD, pdTRUE, pdFALSE, portMAX_DELAY);
						escribir_ln_id_box(eTXT_ONDE, getVar(TIPO_SERVICIO));	// Mensaje al display

						// Cerrar motor inferior y esperar a que termine
						accion_motor(MOTOR_INFERIOR, ACCION_CERRAR);
						//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);

						// No se limpia pantalla en todos los casos, este si
						incrementar_contadores(orhi_msg.servicio);
					}
					else if (leer_word_eeprom(DIRECC_ESTADO_TRAMPILLA) == T1CAFE)
					// Café solo en trampilla abajo --> No hay que moler
					{
						// Abrir motor inferior y esperar a que termine
						accion_motor(MOTOR_INFERIOR, ACCION_ABRIR);
						//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);

						escribir_ln_id_box(eTXT_ONDE, getVar(TIPO_SERVICIO));	// Mensaje al display
						vTaskDelay(TIEMPO_DESCARGA);

						// Cerrar motor inferior y esperar a que termine
						accion_motor(MOTOR_INFERIOR, ACCION_CERRAR);
						//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);

						// Termina la secuencia de cambio de tipo servicio
						grabar_word_eeprom(DIRECC_ESTADO_TRAMPILLA, NO_CAFE);
					}
					else if (leer_word_eeprom(DIRECC_ESTADO_TRAMPILLA) == T1T2CAFE)
					//  Estaba lleno abajo y arriba --> Dosificar y completar dosis
					{
						// Abrir motor inferior y esperar a que termine
						accion_motor(MOTOR_INFERIOR, ACCION_ABRIR);
						//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);

						escribir_ln_id_box(eTXT_EMPTY_BOX, getVar(TIPO_SERVICIO));  // Display caja vacia
						vTaskDelay(TIEMPO_DESCARGA);

						// Cerrar motor inferior y esperar a que termine
						accion_motor(MOTOR_INFERIOR, ACCION_CERRAR);
						//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);

						// Abrir motor superior y esperar a que termine
						accion_motor(MOTOR_SUPERIOR, ACCION_ABRIR);
						//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);

						// Preparo la dosis a sumar sobre lo que había en la trampilla
						// superior para obtener la dosis de 1 café.
						tiempo_moliendo = dosis_1c * 2 - dosis_2c;

						// Si la dosis C2 es justo doble que C1 no muelo.
						if (tiempo_moliendo != 0)
						{
							_motor_ac_on_();
						}

						escribir_ln_id_llenarbox(eTXT_C1_ONDEM, LINEA_2, tiempo_moliendo);
						// Esperar "dosis_1c * 2 - dosis_2c" milisegundos
						vTaskDelay(tiempo_moliendo);

						// Apagar Motor AC
						_motor_ac_off_();

						vTaskDelay(TIEMPO_DESCARGA);
						// Esperamos que la tarea  Oled este libre, podría estar acabando la animacion
						// habría que asegurarse de que hay lcd si se metiera un display diferente:
						// if(lcd()) xEventGroup... o sustituir EV_LCD por EV_FINANIMACION
						xEventGroupWaitBits(task_events, EV_LCD, pdTRUE, pdFALSE, portMAX_DELAY);

						// No limpiamos pantalla, escribimos "cambio"
						escribir_ln_id_box(eTXT_CHAN2, getVar(TIPO_SERVICIO));
						grabar_word_eeprom(DIRECC_ESTADO_TRAMPILLA, T1CAFE);
					}
				}
				// Fin servicio, reset de teclado, flag fin secuencia
				if ((xEventGroupGetBits(task_events) & EV_INTENSIVA_ON) == 0 ) //si estamos en prueba intensiva no resetea teclado
					xQueueReset(handle_queue_teclado);						   //  para detectar cuando se quiere parar la prueba
				xEventGroupSetBits(task_events, EV_FIN_SERV);
				break; //case SIMPLE:

			case ORHI_INICIAR:
				// Elegir secuencia: PREMOLIDO o INSTANTANEO
				if (orhi_msg.modo == PREMOLIDO)
				// PREMOLIDO: Dosificador lleno, si esta vacío lo llenamos.
				{
					// Si se ha programado algo incoherente lo corregimos
					if (leer_word_eeprom(DIRECC_DOSIS_2C) > (leer_word_eeprom(DIRECC_DOSIS_1C)*2) )
					{
						grabar_word_eeprom(DIRECC_DOSIS_2C, leer_word_eeprom(DIRECC_DOSIS_1C)*2);
					}

					if (leer_word_eeprom(DIRECC_ESTADO_TRAMPILLA) == NO_CAFE)
					// Vacío totalmente (venimos de FRESH) hay que llenarlo
					{
						// Cerrar motor inferior y esperar a que termine
						accion_motor(MOTOR_INFERIOR, ACCION_CERRAR);
						//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);

						// Encender Motor AC
						_motor_ac_on_();

						// Esperar "dosis_1c" milisegundos
						tiempo_moliendo = dosis_1c;

						if (dosis_simple())
							escribir_ln_id_llenarbox(eTXT_C1_PREM, LINEA_2, tiempo_moliendo);
						else if (dosis_doble())
							escribir_ln_id_llenarbox(eTXT_C2_PREM, LINEA_2, tiempo_moliendo);
						vTaskDelay(tiempo_moliendo);

						// Apagar Motor AC
						_motor_ac_off_();

						vTaskDelay(TIEMPO_DESCARGA);

						// Cerrar motor superior y esperar a que termine
						accion_motor(MOTOR_SUPERIOR, ACCION_CERRAR);
						//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);

						// Esperamos que la tarea  Oled este libre, podría estar acabando la animacion
						// habría que asegurarse de que hay lcd si se metiera un display diferente:
						// if(lcd()) xEventGroup... o sustituir EV_LCD por EV_FINANIMACION
			            xEventGroupWaitBits(task_events, EV_LCD, pdTRUE, pdFALSE, portMAX_DELAY);

						// Calcular tiempo de molido restante
						tiempo_moliendo = dosis_2c - dosis_1c;

						// Encender Motor AC si el tiempo restante es superior a 0
						if (tiempo_moliendo != 0)
						{
							_motor_ac_on_();
						}

						if (dosis_simple())
							escribir_ln_id_llenarbox(eTXT_C1_PREM, LINEA_1, tiempo_moliendo);
						else if (dosis_doble())
							escribir_ln_id_llenarbox(eTXT_C2_PREM, LINEA_1, tiempo_moliendo);
						// Esperar "dosis_2c - dosis_1c" milisegundos
						vTaskDelay(tiempo_moliendo);

						// Apagar Motor AC
						_motor_ac_off_();
						incrementar_contadores(DOBLE);
						// Esperamos que la tarea  Oled este libre, podría estar acabando la animacion
						// habría que asegurarse de que hay lcd si se metiera un display diferente:
						// if(lcd()) xEventGroup... o sustituir EV_LCD por EV_FINANIMACION
						xEventGroupWaitBits(task_events, EV_LCD, pdTRUE, pdFALSE, portMAX_DELAY);
					}
					else if (leer_word_eeprom(DIRECC_ESTADO_TRAMPILLA) == T1CAFE) // Hay café abajo pero no arriba
					{
						// Cerrar motor superior y esperar a que termine
						accion_motor(MOTOR_SUPERIOR, ACCION_CERRAR);
						//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);

						// Calcular tiempo de molido restante
						tiempo_moliendo = dosis_2c - dosis_1c;

						// Encender Motor AC si el tiempo restante es superior a 0
						if (tiempo_moliendo != 0)
						{
							_motor_ac_on_();
						}

						if (dosis_simple())
							escribir_ln_id_llenarbox(eTXT_C1_PREM, LINEA_1, tiempo_moliendo);
						else if (dosis_doble())
							escribir_ln_id_llenarbox(eTXT_C2_PREM, LINEA_1, tiempo_moliendo);
						// Esperar "dosis_2c - dosis_1c" milisegundos
						vTaskDelay(tiempo_moliendo);

						// Apagar Motor AC
						_motor_ac_off_();

						// Esperamos que la tarea  Oled este libre, podría estar acabando la animacion
						// habría que asegurarse de que hay lcd si se metiera un display diferente:
						// if(lcd()) xEventGroup... o sustituir EV_LCD por EV_FINANIMACION
						xEventGroupWaitBits(task_events, EV_LCD, pdTRUE, pdFALSE, portMAX_DELAY);
					}

					// Hemos llenado el dosificador si no estaba ya lleno
					grabar_word_eeprom(DIRECC_ESTADO_TRAMPILLA, T1T2CAFE);

					// Fin de cambio de premolido (FAST) a instantaneo (FRESH)
					// Fin servicio: pantalla LCD, flag fin secuencia
				}
				else
				// INSTANTANEO o FRESH: Dosificador debe estar vacío.
				{
					/*if (leer_word_eeprom(DIRECC_ESTADO_TRAMPILLA) != NO_CAFE)
						// Lleno total o parcialmente, venimos de FAST
					{
						// Mensaje "cambio"
						escribir_ln_id(eTXT_CAMBIO, LINEA_2, PARPADEO_NO);
					}*/
					if (leer_word_eeprom(DIRECC_ESTADO_TRAMPILLA) == T1T2CAFE)
					{
						// Lleno totalmente, venimos de PREMOLIDO
						escribir_ln_id_box(eTXT_CHAN1, getVar(TIPO_SERVICIO)); 	// Mensaje caja llena
					}
					else if (leer_word_eeprom(DIRECC_ESTADO_TRAMPILLA) == T1CAFE)
					{
						// Medio lleno, queda dosis de 1C
						escribir_ln_id_box(eTXT_CHAN2, getVar(TIPO_SERVICIO)); // Mensaje caja medio llena
					}
					else
					{
						escribir_ln_id_box(eTXT_ONDE, getVar(TIPO_SERVICIO));	// Mensaje al display
					}
					//xEventGroupSetBits(task_events, EV_MUTEXCONFIG);//sólo necesario al pasar a ondemand por tema de tiempos
				}
				// Fin servicio, reset de teclado, flag fin secuencia
				if ((xEventGroupGetBits(task_events) & EV_INTENSIVA_ON) == 0 ) //si estamos en prueba intensiva no resetea teclado
					xQueueReset(handle_queue_teclado);						   //  para detectar cuando se quiere parar la prueba
				xEventGroupSetBits(task_events, EV_FIN_SERV);
				break; // case ORHI_INICIAR:

			default:
				break;
			} // switch (orhi_msg.servicio)
		} // if (xQueueReceive(handle_queue_orhi, &orhi_msg, portMAX_DELAY) == pdPASS)
	} // for(;;)
}
