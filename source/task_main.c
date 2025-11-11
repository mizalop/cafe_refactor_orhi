/**
 * \file task_main.c
 * \brief Módulo principal del control de molino automático.
 *
 * \version Markibar_Control_1_0
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

#include "board.h"
#include "fsl_debug_console.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "event_groups.h"

#include "fsl_gpio.h"
#include "rtos_time.h"

#include "main.h"
#include "AG5_define.h"
#include "eeprom.h"
#include "backlight.h"
#include "watchdog.h"
#include "pwm.h"
#include "power.h"
#include "teclado.h"
#include "task_motores.h"
#include "porta.h"
#include "task_cafe.h"
#include "task_dosis.h"
#include "task_config_gb.h"
#include "hmi.h"
#include "task_comunica.h"
#include "rs232_uart.h"
#include "avisoFresas.h"
#include "timer_define.h"
#include "inicializa.h"

#include "task_main.h"


/*******************************************************************************
 * Definiciones
 ******************************************************************************/


/*******************************************************************************
 * Variables Globales
 ******************************************************************************/

xQueueHandle  handle_queue_teclado = 0;


/*******************************************************************************
 * Variables Privadas
 ******************************************************************************/

static AG_EST_e estado_ag = AG_EST_INICIO;        // E.automata principal
static SERV_e tipo_servicio = SERV_AUTOMATICO_1;  // Según sensor ...
static SERV_e tipo_servicio_prev = NUM_SERV;      // Según sensor ...
static TAG_e defecto_tag = TAG_C2;

static TECLADO_MSG_t teclado_msg;

static TaskHandle_t configHandle;


/*******************************************************************************
 * Funciones
 ******************************************************************************/

static void conf_gama(void)
{
	lcd_init();
	xTaskCreate(dosis_task,     "Tarea Dosis",     configMINIMAL_STACK_SIZE + 64, NULL, dosis_task_PRIORITY,   NULL);
	xTaskCreate(teclado_task,   "Tarea Teclado",   configMINIMAL_STACK_SIZE,      NULL, teclado_task_PRIORITY, NULL);
	xTaskCreate(lcd_task,       "Tarea LCD",       configMINIMAL_STACK_SIZE,      NULL, lcd_task_PRIORITY,     NULL);
	xTaskCreate(config_task_gb, "Tarea Config GB", configMINIMAL_STACK_SIZE + 96, NULL, config_task_PRIORITY,  &configHandle);
	// Tarea de menú de configuración suspendida por defecto
	vTaskSuspend(configHandle);
}

void enviar_tecla(uint8_t tecla, uint8_t pulsacion_larga)
{
	teclado_msg.tecla = tecla;
	teclado_msg.pulsacion_larga = pulsacion_larga;
	xQueueSend(handle_queue_teclado, &teclado_msg, 0);
}

uint8_t getVar(uint8_t var)
{
	switch(var)
	{
		case TIPO_SERVICIO:
			return tipo_servicio;
			break;
		default:
			return 0xFF;
			break;
	}
}

void setVar(uint8_t var, uint8_t val)
{
	switch(var)
	{
		case TIPO_SERVICIO_PREV:
			tipo_servicio_prev = val;
			break;
		case ESTADO_AG:
			estado_ag = val;
			break;
		default:
			break;
	}
}

uint8_t check_typeA(void){
	if (leer_word_eeprom(DIRECC_COD_CLIENTE) == ORHI_STD_A)
		return pdTRUE;
	return pdFALSE;
}

uint8_t check_typeB(void){
	if (leer_word_eeprom(DIRECC_COD_CLIENTE) == ORHI_STD_B)
		return pdTRUE;
	return pdFALSE;
}

/*
 *  Función auxiliar para mensaje_display()
 *
 *  Se le mete un valor uint8_t correspondiente al modo de trabajo y
 *  devuelve uint8_t que corresponde con la id de texto correcta
 *
 *  equivalente a usar en mensaje_display():
 *    un if más para ver si es ondemand o preground
 *    ó
 *    eTXT_PREG - leer_word_eeprom(DIRECC_MODO_TRABAJO)
 *
 */
static TEXT_ID_e texto(MODO_TRABAJO_e modo)
{
	switch(modo)
	{
		case PREMOLIDO:
			return eTXT_PREG;
			break;
		case ONDEMAND:
			return eTXT_ONDE;
			break;
		default:
			break;
	}
	return 0;
}

static void mensaje_display()
{
	if (leer_word_eeprom(DIRECC_MODO_TRABAJO) == ONDEMAND && leer_word_eeprom(DIRECC_ESTADO_TRAMPILLA) == T1T2CAFE)
		escribir_ln_id_box(eTXT_CHAN1, tipo_servicio);
	else if (leer_word_eeprom(DIRECC_MODO_TRABAJO) == ONDEMAND && leer_word_eeprom(DIRECC_ESTADO_TRAMPILLA) == T1CAFE)
		escribir_ln_id_box(eTXT_CHAN2, tipo_servicio);
	else
		escribir_ln_id_box(texto(leer_word_eeprom(DIRECC_MODO_TRABAJO)),tipo_servicio);
}

/*static void reset_eeprom(){
	for (uint8_t i = 0; i <= DIRECC_FASTCHANGE_ON; i+=2)
		grabar_word_eeprom(i, 0xFFFF);
	while(1);
}//*/

void main_task(void *pvParameters)
{
	TECLADO_MSG_t teclado_msg;
	EventBits_t uxBits;
	status_t result = STATUS_UNDEFINED;
	uint8_t mensaje = pdFALSE, flag_porta_olvidado = pdFALSE, puenteConf = pdFALSE;

	while(handle_queue_teclado == 0)
	{
		// Create the queue (N item x M bytes)
		handle_queue_teclado = xQueueCreate(TECLADO_QUEUE_MAX_ITEMS, TECLADO_QUEUE_ITEM_SIZE);
		vTaskDelay(TIME_10MSEC);
	}

	// Crear Timers para timeouts
	vCreaTimers();
	vTaskDelay(TIME_1SEC);

	for (;;)
	{
		switch (estado_ag) // Autómata principal molino AG5
		{
		case AG_EST_INICIO: // Inicializaciones rutinas
			pwm_init();
			backlight_init();

			conf_gama();

			// Inicializar eeprom
			taskENTER_CRITICAL();
			while (result != kStatus_FLASH_Success)
			{
				result = inicializar_eeprom();
			}
			taskEXIT_CRITICAL();
			//reset_eeprom();

			estado_ag = AG_EST_SOFT_INI;
			break; // AG_EST_INICIO

		case AG_EST_SOFT_INI:
			tipo_servicio_prev = NUM_SERV;

			// Inicializar configuración
			inicializar_config();
			inicializar_idioma();
			uxBits = xEventGroupGetBits(task_events);
			if (power_ok() == 1 || ((uxBits & EV_ERRORPOWER_OFF) == EV_ERRORPOWER_OFF))
			{
				mensaje = pdTRUE;
				moler_dosificar(leer_word_eeprom(DIRECC_MODO_TRABAJO), ORHI_INICIAR);

				estado_ag = AG_EST_COMPROBAR;
			}
			else
			{
				vStartUnivTimer(ID_ERRORPOWER);
				limpiar_pantalla();
				escribir_id(eTXT_ERROR_POWER, PARPADEO_NO);
				estado_ag = AG_EST_ERROR_POWER;
			}
			break; // AG_EST_INI_E2PROM

		case AG_EST_COMPROBAR:
			// Esperar a que termine la inicialización del molino-dosificador
			uxBits = xEventGroupWaitBits(task_events, EV_FIN_SERV, pdTRUE, pdFALSE, 0);
			if ((uxBits & EV_FIN_SERV) == EV_FIN_SERV)
			{
				//filtro acceso a configuración
				/*uxBits = xEventGroupGetBits(task_events);
				if ((uxBits & EV_MUTEXCONFIG) != 0){
					vTaskDelay(TIME_1SEC);
					xQueueReset(handle_queue_teclado);
					xEventGroupClearBits(task_events, EV_MUTEXCONFIG);
				}*/
				// Arranca comunicacion_nube
				ini_comunica();
				// Encender sensor magnético
				sensor_mag_on();
				if (leer_word_eeprom(DIRECC_MODO_IDENTIF) == SENSOR_MAG)
					vStartUnivTimer(ID_MAG);
				estado_ag = AG_EST_ON;
			}
			break; // AG_EST_COMPROBAR

		case AG_EST_ON: // Elige el modo de identificación de portas
			if (micro_porta_pulsado())
			{
				// Estamos en automático: el sensor decide el tipo servicio
				// Si se ha elegido por teclado (o por sensor) un servicio simple
				if ((tipo_servicio == SERV_SIMPLE) || (tipo_servicio == SERV_AUTOMATICO_1) || (tipo_servicio == SERV_SIMPLE_PROV))
				{
					tipo_servicio_prev = tipo_servicio;
					estado_ag = AG_EST_MOLER_1;
				}
				// Se ha elegido servicio doble
				else if ((tipo_servicio == SERV_DOBLE) || (tipo_servicio == SERV_AUTOMATICO_2) || (tipo_servicio == SERV_DOBLE_PROV))
				{
					tipo_servicio_prev = tipo_servicio;
					estado_ag = AG_EST_MOLER_2;
				}
				// Posible doble click
				else if (tipo_servicio == SERV_PROV)
				{
					// TOD0: Implementar Timer con TIEMPO_FILTRO_REBOTES?
					tipo_servicio = SERV_SIMPLE_PROV;
					estado_ag = AG_EST_FILTRA_REBOTE;
				}
			}
			else  // Micro no pulsado
			{
				if (xQueueReceive(handle_queue_teclado, &teclado_msg, 0) == pdPASS)
				{
					switch (teclado_msg.tecla)
					{
						case TECLA_C1:
							estado_ag = AG_EST_PULSADO_C1;
							break;

						case TECLA_C2:
							estado_ag = AG_EST_PULSADO_C2;
							break;

						case TECLA_AVANCE:
							//uxBits = xEventGroupGetBits(task_events);
							if (teclado_msg.pulsacion_larga == pdTRUE) // && ((uxBits & EV_MUTEXCONFIG) == 0)) // Pulsación larga de tecla ENTER: Acceso a menús
							{
								vTaskResume(configHandle);
								estado_ag = AG_EST_CONFIGURACION;
							}
							/*else if (teclado_msg.pulsacion_larga == pdFALSE) // se deja como en el izaga, sólo reset de imán automático
							{
								estado_ag = AG_EST_RST_MAG;
							}*/
							break;

						case TECLA_ON:
							estado_ag = AG_EST_PULSADO_OFF;
							break;

						default:
							break;
					}
				}
				else // Si no hay teclas pulsadas
				{
					if (xTimerIsTimerActive(xTimers[TIMER_C1C2]) == pdFALSE) // Si ha pasado un tiempo sin pulsar C1 o C2
					{
						switch (leer_word_eeprom(DIRECC_MODO_IDENTIF))
						{
						case SENSOR_MAG:
							if (mag_detectado()) // Imán detectado
							{
								if (ucTimerActive(ID_MAG) == pdFALSE) // Si ha pasado un tiempo detectando sensor magnético
								{
									estado_ag = AG_EST_RST_MAG;
								}
								if (defecto_tag == TAG_C2) // Imán en porta C2
								{
									tipo_servicio = SERV_AUTOMATICO_2;
									if (tipo_servicio_prev != SERV_AUTOMATICO_2)
									{
										mensaje = pdTRUE;
										tipo_servicio_prev = SERV_AUTOMATICO_2;
									}
								}
								else // Imán en porta C1
								{
									tipo_servicio = SERV_AUTOMATICO_1;
									if (tipo_servicio_prev != SERV_AUTOMATICO_1)
									{
										mensaje = pdTRUE;
										tipo_servicio_prev = SERV_AUTOMATICO_1;
									}
								}
							}
							else // Imán no detectado
							{
								vStartUnivTimer(ID_MAG); // Iniciar timer detección sensor magnético
								if (defecto_tag == TAG_C2)          // Imán en porta C2
								{
									tipo_servicio = SERV_AUTOMATICO_1;
									if (tipo_servicio_prev != SERV_AUTOMATICO_1)
									{
										mensaje = pdTRUE;
										tipo_servicio_prev = SERV_AUTOMATICO_1;
									}
								}
								else // Imán en porta C1
								{
									tipo_servicio = SERV_AUTOMATICO_2;
									if (tipo_servicio_prev != SERV_AUTOMATICO_2)
									{
										mensaje = pdTRUE;
										tipo_servicio_prev = SERV_AUTOMATICO_2;
									}
								}
							}
							break;

						case DOBLE_CLICK:
							tipo_servicio = SERV_PROV;
							if (tipo_servicio_prev != SERV_PROV)
							{
								mensaje = pdTRUE;
								tipo_servicio_prev = SERV_PROV;
							}
							break;

						case TECLADO_C1:
							tipo_servicio = SERV_SIMPLE;
							if (tipo_servicio_prev != SERV_SIMPLE)
							{
								mensaje = pdTRUE;
								tipo_servicio_prev = SERV_SIMPLE;
							}
							break;

						case TECLADO_C2:
							tipo_servicio = SERV_DOBLE;
							if (tipo_servicio_prev != SERV_DOBLE)
							{
								mensaje = pdTRUE;
								tipo_servicio_prev = SERV_DOBLE;
							}
							break;

						default: // TECLADO_ULT
							if ((tipo_servicio == SERV_SIMPLE) && (tipo_servicio_prev != SERV_SIMPLE))
							{
								mensaje = pdTRUE;
								tipo_servicio_prev = SERV_SIMPLE;
							}
							else if ((tipo_servicio == SERV_DOBLE) && (tipo_servicio_prev != SERV_DOBLE))
							{
								mensaje = pdTRUE;
								tipo_servicio_prev = SERV_DOBLE;
							}
							else if ((tipo_servicio != SERV_SIMPLE) && (tipo_servicio != SERV_DOBLE))
							{
								tipo_servicio = SERV_SIMPLE;
								tipo_servicio_prev = SERV_SIMPLE;
								mensaje = pdTRUE;
							}
							break;
						} // switch (modo_identif)
						if (mensaje == pdTRUE){
							mensaje_display();
							mensaje = pdFALSE;
						}
					} // if( xTimerIsTimerActive( xTimer ) == pdFALSE )
				} // if (xQueueReceive(handle_queue_teclado, &teclado_msg, 0) == pdPASS)
			} // Micro no pulsado
			break; // AG_EST_ON

		case AG_EST_PULSADO_C1:
			if (teclado_msg.pulsacion_larga == pdTRUE) // Pulsación larga es entrada a Programación Dosis
			{
				programar_dosis(SIMPLE);
				estado_ag = AG_EST_PROGRAMACION;
			}
			else // Pulsación corta es una selección de tipo servicio
			{
				tipo_servicio = SERV_SIMPLE;
				mensaje_display();
				xTimerStart(xTimers[TIMER_C1C2], 0); // Iniciar timer para tiempo esperando a servicio simple
				mensaje = pdFALSE;
				estado_ag = AG_EST_ON;               // Vuelvo a donde estaba
			}
			break; // AG_EST_PULSADO_C1

		case AG_EST_PULSADO_C2:
			if (teclado_msg.pulsacion_larga == pdTRUE)
			{
				programar_dosis(DOBLE);
				estado_ag = AG_EST_PROGRAMACION;
			}
			else
			{
				tipo_servicio = SERV_DOBLE;
				mensaje_display();
				xTimerStart(xTimers[TIMER_C1C2], 0); // Iniciar timer para tiempo esperando a servicio simple
				mensaje = pdFALSE;
				estado_ag = AG_EST_ON;               // Vuelvo a donde estaba
			}
			break; // AG_EST_PULSADO_C2

		case AG_EST_RST_MAG:
			limpiar_pantalla();
			escribir_id(eTXT_RESETMAG, PARPADEO_TODO);
			sensor_mag_off();
			vTaskDelay(TIME_2SEC);
			sensor_mag_on();
			mensaje = pdTRUE;
			estado_ag = AG_EST_ON;
			break; // AG_EST_PULSADO_AVANCE

		case AG_EST_FILTRA_REBOTE:
			// Espero un tiempo muy corto para asegurar que no hay
			// rebotes en el microruptor.
			vTaskDelay(TIEMPO_FILTRO_REBOTES);
			vStartUnivTimer(ID_DOBLEPULS); // Iniciar timer de espera a segundo click
			estado_ag = AG_EST_ESPERA_C1;
			break; // AG_EST_FILTRA_REBOTE

		case AG_EST_ESPERA_C1:
			if (!micro_porta_pulsado())
			{
				// Han soltado el micro porta -> Posible doble click
				vStartUnivTimer(ID_DOBLEPULS); // Iniciar timer de espera a segundo click
				estado_ag = AG_EST_ESPERA_DOBLE;
			}
			else if (ucTimerActive(ID_DOBLEPULS) == pdFALSE)
			{
				// Sigue pulsado el micro porta -> servicio C1
				// Ya está seleccionado tipo_servicio = SERV_SIMPLE
				estado_ag = AG_EST_ON;
			}
			break; // AG_EST_ESPERA_C1

		case AG_EST_ESPERA_DOBLE:
			if (micro_porta_pulsado())
			{
				// Segunda pulsación correcta -> servicio C2
				// Con el servicio ya seleccionado vuelvo a EST_ON
				tipo_servicio = SERV_DOBLE_PROV;
				estado_ag = AG_EST_ON;
			}
			else if (ucTimerActive(ID_DOBLEPULS) == pdFALSE)
			{
				// Si pasa el tiempo no vuelven a pulsar anulo servicio
				estado_ag = AG_EST_ON;
			}
			break; // AG_EST_ESPERA_DOBLE

		case AG_EST_MOLER_1:
			moler_dosificar(leer_word_eeprom(DIRECC_MODO_TRABAJO), SIMPLE);
			uxBits = xEventGroupWaitBits(task_events, EV_FIN_SERV, pdTRUE, pdFALSE, portMAX_DELAY);
			if ((uxBits & EV_FIN_SERV) == EV_FIN_SERV)
				estado_ag = AG_EST_FIN_MOLIDO;
			break; // AG_EST_MOLER_1

		case AG_EST_MOLER_2:
			moler_dosificar(leer_word_eeprom(DIRECC_MODO_TRABAJO), DOBLE);
			uxBits = xEventGroupWaitBits(task_events, EV_FIN_SERV, pdTRUE, pdFALSE, portMAX_DELAY);
			if ((uxBits & EV_FIN_SERV) == EV_FIN_SERV)
				estado_ag = AG_EST_FIN_MOLIDO;
			break; // AG_EST_MOLER_2

		case AG_EST_FIN_MOLIDO:
			if (!micro_porta_pulsado()) // Porta retirado
			{
				xTimerStop(xTimers[TIMER_C1C2], 0);
				envia_sensores();
				if (check_aviso() == pdTRUE) estado_ag = AG_EST_AVISOFRESAS;
				else estado_ag = AG_EST_ON;
			}
			else
			{
				// En OnDemand damos fallo tras un tiempo
				if (leer_word_eeprom(DIRECC_MODO_TRABAJO) == ONDEMAND)
				{
					vTaskDelay(TIME_1SEC);
					if (micro_porta_pulsado()) // Compruebo segunda vez sigue pulsado
					{
						limpiar_pantalla();
						escribir_id(eTXT_RETIRE_PORTA, PARPADEO_TODO);
						estado_ag = AG_EST_MICRO_PULSADO;
					}
				}
				else // En premolido damos fallo inmediatamente
				{
					limpiar_pantalla();
					escribir_id(eTXT_RETIRE_PORTA, PARPADEO_TODO);
					estado_ag = AG_EST_MICRO_PULSADO;
				}
				/*Iniciar timer para apagar la tarjeta si pasan x (4 - 5?) segundos
				* En un inicio se apaga la tarjeta cuando acaba el timer  de detección c2
				* Pero con el problema encontrado en la tarjeta magnética es mejor apagarla
				* antes. Se mantiene encendida durante x segundos por si hay un despiste y
				* el barista deja el porta durante un instante, que no haga falta que se resetee
				* ya que es difícil que le ocurra el problema en ese instante.
				*/
				//sólo es útil si se está detectando el porta doble y si está activo el sensor mangético
				//pero hace un raro si se le acerca un imán después. (en el siguiente estado sale como
				//timer no activo. de todas maneras es un caso anormal y no obstante la conclusión sería
				//la misma si es que el imán se acercara tras 5 segundos pulsando porta
				if ((mag_detectado()) && \
						(leer_word_eeprom(DIRECC_MODO_IDENTIF) == SENSOR_MAG))
					vStartUnivTimer(ID_FILTROMAG);
			}
			break; // AG_EST_FIN_MOLIDO

		case AG_EST_MICRO_PULSADO:
			if (!micro_porta_pulsado()) // Porta retirado
			{
				//Timer para reset automático de la magnética se resetea porque se acaba de soltar,
				//  luego el tiempo que marcaba C2 no era lectura falsa
				//Sólo se pone aquí para evitar situaciones en las que acabe el timer antes de llegar a ON.
				vStartUnivTimer(ID_MAG);

				if (flag_porta_olvidado == SI){
					escribir_id(eTXT_RESETMAG, PARPADEO_NO);
					vTaskDelay(TIME_3SEC); // retardo por si estuvieran
										   //   retirando el porta
					sensor_mag_on();
				}
				else vTaskDelay(TIME_400MSEC); //Para evitar falsas pulsaciones
				                               //  en el caso de flag_porta_olvidado
											   //  ya hay un delay de 3segundos para el reset que evita pulsaciones
				xTimerStop(xTimers[TIMER_C1C2], 0);
				flag_porta_olvidado = pdFALSE;
				xQueueReset(handle_queue_teclado);
				envia_sensores();
				if (check_aviso() == pdTRUE) estado_ag = AG_EST_AVISOFRESAS;
				else estado_ag = AG_EST_MENSAJE_ON; // Para recuperar mensaje
			}
			else if (flag_porta_olvidado == pdFALSE)
			{
				//Micro sigue pulsado, imán activo y salta reset de mag.
				//ojo que ucTimerActive también devuelve false si está en doble
				//pulsación. En principio chequeando el modo de identif. se
				//soluciona esa cuestión.
				if (((mag_detectado()) && (!ucTimerActive(ID_FILTROMAG)) && \
						(leer_word_eeprom(DIRECC_MODO_IDENTIF) == SENSOR_MAG)) || \
						puenteConf)
				//puenteConf es una variable booleana que siempre está a 0. se pone a 1
				//  cuando se sale del menú de configuración con el porta puesto. si no,
				//  se reinicia el imán y si está puesto el porta doble hace raros.
				{
					sensor_mag_off();
					puenteConf = pdFALSE;
					flag_porta_olvidado = pdTRUE;
				}
			}
			break; // AG_EST_FALLO_MICRO

		case AG_EST_PULSADO_OFF:
			xTimerStop(xTimers[TIMER_C1C2], 0);
			estado_ag = AG_EST_MENSAJE_OFF;
			break; // AG_EST_PULSADO_OFF

		case AG_EST_MENSAJE_OFF:
			limpiar_pantalla();
			escribir_ln_id(eTXT_OFF, LINEA_1, PARPADEO_NO);
			sensor_mag_off();
			vTaskDelay(TIME_1SEC);
			limpiar_pantalla();
			backlight_off();
			estado_ag = AG_EST_OFF;
			break; // AG_EST_MENSAJE_OFF

		case AG_EST_OFF:
			if (xQueueReceive(handle_queue_teclado, &teclado_msg, 0) == pdPASS)
			{
				if (teclado_msg.tecla == TECLA_ON){
					backlight_set_eeprom();
					escribir_id(eTXT_ON, PARPADEO_NO);
					sensor_mag_on();
					estado_ag = AG_EST_MENSAJE_ON;
				}
				else if (teclado_msg.tecla == TECLA_C1C2 || teclado_msg.tecla == TECLA_C1AVANCE){
					estado_ag = AG_EST_CONFIGURACION;
					//se reenvía la tecla pulsada para que la reciba la tarea de configuración
					xQueueSend(handle_queue_teclado, &teclado_msg, 0);
					backlight_set_eeprom();
					vTaskResume(configHandle);
				}
			}
			break; // AG_EST_OFF

		case AG_EST_MENSAJE_ON:
			vTaskDelay(TIME_500MSEC);
			mensaje_display();
			mensaje = pdFALSE;
			estado_ag = AG_EST_ON;
			break; // AG_EST_MENSAJE_ON

		case AG_EST_PROGRAMACION:
			uxBits = xEventGroupWaitBits(task_events, EV_DOSIS, pdTRUE, pdFALSE, portMAX_DELAY);
			if ((uxBits & EV_DOSIS) == EV_DOSIS)
			{
				envia_sensores();
				limpiar_pantalla();
				estado_ag =  AG_EST_MENSAJE_ON;  // Antes AG_EST_SOFT_INI;
			}
			break; // AG_EST_PROGRAMACION

		case AG_EST_CONFIGURACION:
			uxBits = xEventGroupWaitBits(task_events, EV_CONFIG, pdTRUE, pdFALSE, portMAX_DELAY);
			if (( uxBits & EV_CONFIG ) != 0)
			{
				vTaskSuspend(configHandle);
				if(micro_porta_pulsado()){
					if (leer_word_eeprom(DIRECC_MODO_IDENTIF) == SENSOR_MAG) puenteConf = pdTRUE;
					if (leer_word_eeprom(DIRECC_INICIALIZADO) != SI) inicializar_idioma();
					flag_porta_olvidado = pdFALSE;
					escribir_id(eTXT_RETIRE_PORTA, PARPADEO_TODO);
					estado_ag = AG_EST_MICRO_PULSADO;
				}
				else
					estado_ag = AG_EST_SOFT_INI;
			}
			break; // AG_EST_CONFIGURACION

		case AG_EST_ERROR_POWER:
			// Por seguridad, si hubiera alguna situación no prevista en la que llega a este estado, si
			//   el flag está activo, se sale.
			uxBits = xEventGroupGetBits(task_events);
			if ((uxBits & EV_ERRORPOWER_OFF) == EV_ERRORPOWER_OFF)
				estado_ag = AG_EST_SOFT_INI;
			break; // AG_EST_ERROR_POWER

		case AG_EST_AVISOFRESAS:
			limpiar_pantalla();
			escribir_id(eTXT_MENSAJECAMBIO, PARPADEO_NO);
			vTaskDelay(TIME_1SEC);
			mensaje = pdTRUE;
			estado_ag = AG_EST_ON;
			break;

		default: // Estado erróneo
			break;
		} // End switch(estado_ag)

		xEventGroupSetBits(wdt_events, EV_WDT_MAIN);
		vTaskDelay(TIME_10MSEC);
	} // End bucle infinito for(;;)
}
