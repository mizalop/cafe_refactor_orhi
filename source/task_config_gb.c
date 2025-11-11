/**
 * \file task_config_gb.c
 * \brief Módulo de gestión de menú de configuración
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

#include <motores.h>
#include <stdio.h>

#include "board.h"
#include "fsl_debug_console.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#include "main.h"
#include "AG5_define.h"
#include "rtos_time.h"
#include "eeprom.h"
#include "teclado.h"
#include "backlight.h"
#include "porta.h"
#include "task_cafe.h"
#include "task_main.h"
#include "task_dosis.h"
#include "task_comunica.h"
#include "hmi.h"
#include "task_config_gb.h"
#include "avisoFresas.h"
#include "timer_define.h"
#include "inicializa.h"

/*******************************************************************************
 * Definiciones
 ******************************************************************************/

#define NUM_CICLOS_PRUEBA_MAX   500
#define NUM_CICLOS_PRUEBA_INI   100

#define PRUEBA_PESADAS_D1         3500 //ms
#define PRUEBA_PESADAS_D2         2000 //ms
#define PRUEBA_PESADAS_PAUSA TIME_3SEC

/**
 * @brief Estados del menú de configuración.
 */
typedef enum
{
	EST_PASS_U = 0,                   /**< Menú de password de usuario */
	EST_PASS_SU,                      /**< Menú de password de superusuario */
	EST_CONT_PAR,                     /**< Menú de contadores parciales */
	EST_CONT_PAR_C1,                  /**< Menú de contadores parciales: Servicios C1 */
	EST_CONT_PAR_C2,                  /**< Menú de contadores carciales: Servicios C2 */
	EST_CONT_PAR_SUMA,                /**< Menú de contadores parciales: Suma de servicios */
	EST_CONT_PAR_BORRAR,              /**< Menú de contadores parciales: Borrar? */
	EST_CONT_PAR_BORRAR_SEL,          /**< Menú de contadores parciales: Selección borrado */
	EST_CONT_PAR_SALIR,               /**< Menú de contadores parciales: Salir */
	EST_SAT,                          /**< Menú SAT */
	EST_SAT_VERSION,                  /**< Versión del SW */
	EST_SAT_CONT_TOT,                 /**< Menú de contadores totales */
	EST_SAT_CONT_TOT_C1,              /**< Menú de contadores totales: Servicios C1 */
	EST_SAT_CONT_TOT_C2,              /**< Menú de contadores totales: Servicios C2 */
	EST_SAT_CONT_TOT_SUMA,            /**< Menú de contadores totales: Suma de servicios */
	EST_SAT_CONT_TOT_SALIR,           /**< Menú de contadores totales: Salir */
	EST_SAT_IDENTIFICACION,           /**<  */
	EST_SAT_IDENTIFICACION_C1,        /**<  */
	EST_SAT_IDENTIFICACION_C2,        /**<  */
	EST_SAT_IDENTIFICACION_ULTIMA,    /**<  */
	EST_SAT_IDENTIFICACION_DOBLE,     /**<  */
	EST_SAT_IDENTIFICACION_MAG,       /**<  */
	EST_SAT_TRABAJO,                  /**<  */
	EST_SAT_TRABAJO_ONDEMAND,         /**<  */
	EST_SAT_TRABAJO_PREMOLIDO,        /**<  */
	EST_SAT_TRABAJO_CR,				  /**<  */
	EST_SAT_LIMPIEZA,                 /**<  */
	EST_SAT_LIMPIEZA_VACIO,           /**<  */
	EST_SAT_LIMPIEZA_CICLOS,          /**<  */
	EST_SAT_PRUEBA,                   /**<  */
	EST_SAT_PRUEBA_CONF,              /**<  */
	EST_SAT_FABRICA,                  /**<  */
	EST_SAT_FABRICA_VALIDAR,		  /**<  */
	EST_SAT_MODELO_CONF,			  /**<  */
	EST_SAT_SERIE,                    /**<  */
	EST_SAT_SERIE_CONF,               /**<  */
	EST_SAT_PASSWORD_U,               /**<  */
	EST_SAT_PASSWORD_U_SEL,           /**<  */
	EST_SAT_PASSWORD_U_CONF,          /**<  */
	EST_SAT_IDIOMA,                   /**<  */
	EST_SAT_IDIOMA_CONF,              /**<  */
	EST_SAT_BACKLIGHT,                /**<  */
	EST_SAT_BACKLIGHT_CONF,           /**<  */
	EST_SAT_AVFR,					  /**<  */
	EST_SAT_AVFR_CONF,				  /**<  */
	EST_SAT_AVFR_NCAFES,			  /**<  */
	EST_SAT_SALIR,                    /**<  */
	EST_SALIR,                        /**<  */
	EST_ENTRADA,					  /**< Centraliza y redirige la entrada al menú de configuración */
	EST_CAMBIO_MODO					  /**<  */
} EST_e;


/*******************************************************************************
 * Funciones
 ******************************************************************************/

static void rellenar_salir()
{
	accion_motor(MOTOR_INFERIOR, ACCION_CERRAR);
	//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY); //Espera que termine el motor
	if (leer_word_eeprom(DIRECC_MODO_TRABAJO) == PREMOLIDO)
	{
		escribir_id(eTXT_RECARGAR, PARPADEO_NO);
		// Rellenamos otra vez el dosificador
		_motor_ac_on_(); // Moler dosis
		vTaskDelay(leer_word_eeprom(DIRECC_DOSIS_1C));
		_motor_ac_off_();
		vTaskDelay(TIME_500MSEC); // Espera caida del café
		accion_motor(MOTOR_SUPERIOR, ACCION_CERRAR);
		//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);
		_motor_ac_on_(); // Moler dosis
		vTaskDelay(leer_word_eeprom(DIRECC_DOSIS_2C) - leer_word_eeprom(DIRECC_DOSIS_1C));
		_motor_ac_off_();
	}
}

/**
 * @brief Tarea del gestor del menú de configuración.
 */
void config_task_gb(void *pvParameters)
{
	EventBits_t uxBits;
	EST_e estado = EST_ENTRADA;
	TECLADO_MSG_t teclado_msg;
	uint32_t cont = 0;
	uint16_t nivel = 0;
	uint8_t borrar = 0;
	uint8_t activar = 0;
	MODELOS_e modelo = 0;
	uint32_t num_ciclos_prueba = NUM_CICLOS_PRUEBA_INI; // Antes 10
	uint32_t num_ciclos_prueba_temp = 10;
	TickType_t timeout_menu = TIME_30SEC;
	TickType_t tiempo_limpieza = 0;
	uint8_t idioma = INGLES;
	uint8_t posicion = 0;
	char serie[POSICIONES_DEVNAME];
	char pwd[POSICIONES_PASSWORD];
	char pwd_mem[POSICIONES_PASSWORD];
	char str[8];
	char version[8];
	uint16_t word = 0;
	uint8_t directa_limpieza = pdFALSE; //variable que indica si se ha entrado directo a la prueba de pesadas
	uint8_t mensaje_C1 = pdFALSE;
	uint16_t avfr_activa = 0;
	uint32_t ncafes = 0;
	int16_t incr = 0;

	for(;;)
	{
		if (directa_limpieza || xQueueReceive(handle_queue_teclado, &teclado_msg, timeout_menu) == pdPASS)
		{
			if (teclado_msg.tecla == TECLA_ON)
			{
				limpiar_pantalla();
				estado = EST_ENTRADA;
				xEventGroupSetBits(task_events, EV_CONFIG);
				/*Cuando se reactiva la tarea empieza a partir de aquí (se ha parado poniendo bits a 1).
				 *  la variable teclado_msg.tecla aquí es TECLA_ON
				 *
				 *  A diferencia de en programa Izaga, en Orhi "teclado_msg" es variable local (aunque tenga el mismo nombre)
				 *  Por tanto, si no se pasa por el xQueueReceive de la condición principal, mantiene el valor TECLA_ON y
				 *  falla al entrar al EST_ENTRADA.
				 *
				 *  En el caso del Izaga, es una variable global declarada en task_main.h, por lo que al actualizar la variable
				 *  en teclado.c los cambios llegan también aquí y no es necesario recibir su valor de la cola.
				 *
				 *  En este caso, se añade un xQueueReceive con tiempo de espera 0 y cargando el valor a teclado_msg en este
				 *  punto en el que sólo se puede entrar al rearmar el menú, por lo que no influye en otros aspectos, para tomar
				 *  el valor correcto.
				 */
				xQueueReceive(handle_queue_teclado, &teclado_msg, 0);
			}
			switch (estado) // Autómata Menú Configuración
			{
			case EST_ENTRADA:
				if (teclado_msg.tecla == TECLA_C1C2) //acceso directo a prueba de pesadas
				{
					directa_limpieza = pdTRUE;
					estado = EST_SAT_LIMPIEZA;
				}
				else if (teclado_msg.tecla == TECLA_C1AVANCE) //acceso directo a menús
				{
					escribir_ln_id(eTXT_SW_VERSION, LINEA_1, PARPADEO_NO);
					snprintf(version, sizeof(version), "%c%d.%d", PROGRAM_TYPE, VERSION_MAJOR, VERSION_MINOR);
					escribir_ln_str(version, LINEA_2, PARPADEO_NO);
					estado = EST_SAT_VERSION;
				}
				else
				{
					if (leer_word_eeprom(DIRECC_FASTCHANGE_ON) == 1){
						escribir_id(eTXT_CAMBIO_MODO,PARPADEO_NO);
						estado = EST_CAMBIO_MODO;
					}
					else{
						escribir_id(eTXT_CONTADOR_PARCIAL, PARPADEO_NO);
						estado = EST_CONT_PAR;
					}
				}
				xQueueReset(handle_queue_teclado);
				break;

			case EST_PASS_U:
				if (teclado_msg.tecla == TECLA_C1)
				{
					if (pwd[posicion] <= '0')
					{
						pwd[posicion] = '9';
					}
					else
					{
						pwd[posicion]--;
					}
					str[posicion + 2] = pwd[posicion];
					escribir_ln_str(str, LINEA_2, PARPADEO_3 + posicion);
				}
				else if (teclado_msg.tecla == TECLA_C2)
				{
					if (pwd[posicion] >= '9')
					{
						pwd[posicion] = '0';
					}
					else
					{
						pwd[posicion]++;
					}
					str[posicion + 2] = pwd[posicion];
					escribir_ln_str(str, LINEA_2, PARPADEO_3 + posicion);
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					posicion++;
					str[posicion + 2] = pwd[posicion];
					if (posicion == POSICIONES_PASSWORD)
					{
						for (posicion = 0; posicion < POSICIONES_PASSWORD; posicion+=2)
						{
							word = leer_word_eeprom(DIRECC_PASSWORD_U + posicion);
							pwd_mem[posicion] = word & 0x00FF;
							pwd_mem[posicion + 1] = (word & 0xFF00) >> 8;
						}
						if ((pwd[0] == pwd_mem[0]) && (pwd[1] == pwd_mem[1]) && (pwd[2] == pwd_mem[2]) && (pwd[3] == pwd_mem[3]))
						{
							cont = leer_int_eeprom(DIRECC_CONTADOR_1);
							escribir_id(eTXT_PARC_C1, PARPADEO_NO);
							escribir_ln_num(cont, LINEA_2, PARPADEO_NO);
							estado = EST_CONT_PAR_C1;
						}
						else
						{
							limpiar_pantalla();
							estado = EST_ENTRADA;
							xEventGroupSetBits(task_events, EV_CONFIG);
						}
					}
					else
					{
						escribir_ln_str(str, LINEA_2, PARPADEO_3 + posicion);
					}
				}
				break;

			case EST_CAMBIO_MODO:
				if (teclado_msg.tecla == TECLA_C2)
				{
					escribir_id(eTXT_CONTADOR_PARCIAL, PARPADEO_NO);
					estado = EST_CONT_PAR;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					//premolido = 0, ondemand = 1.
					//si es ondemand pasa a premolido y viceversa.
					uint8_t varAux = leer_word_eeprom(DIRECC_MODO_TRABAJO);
					grabar_word_eeprom(DIRECC_MODO_TRABAJO, (varAux == 0));
					//si hiciera falta corrige dosis
					if (leer_word_eeprom(DIRECC_DOSIS_2C) > (leer_word_eeprom(DIRECC_DOSIS_1C)*2))
					{
						grabar_word_eeprom(DIRECC_DOSIS_2C, (leer_word_eeprom(DIRECC_DOSIS_1C)*2));
					}
					//sale a pantalla principal
					limpiar_pantalla();
					estado = EST_ENTRADA;
					xEventGroupSetBits(task_events, EV_CONFIG);
				}
				break;

			case EST_CONT_PAR:
				if ((leer_word_eeprom(DIRECC_FASTCHANGE_ON) == 1) && teclado_msg.tecla == TECLA_C1)
				{
					escribir_id(eTXT_CAMBIO_MODO,PARPADEO_NO);
					estado = EST_CAMBIO_MODO;
				}
				else if (teclado_msg.tecla == TECLA_C2)
				{
					escribir_id(eTXT_SERVICIO_TECNICO, PARPADEO_NO);
					estado = EST_SAT;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					if (leer_word_eeprom(DIRECC_PASS_ACT) == 0)
					{
						cont = leer_int_eeprom(DIRECC_CONTADOR_1);
						escribir_id(eTXT_PARC_C1, PARPADEO_NO);
						escribir_ln_num(cont, LINEA_2, PARPADEO_NO);
						estado = EST_CONT_PAR_C1;
					}
					else
					{
						posicion = 0;

						pwd[0] = '0';
						pwd[1] = '0';
						pwd[2] = '0';
						pwd[3] = '0';

						str[0] = ' ';
						str[1] = ' ';
						str[2] = pwd[0];
						str[3] = pwd[1];
						str[4] = pwd[2];
						str[5] = pwd[3];
						str[6] = ' ';
						str[7] = ' ';

						escribir_id(eTXT_PASSWORD, PARPADEO_NO);
						escribir_ln_str(str, LINEA_2, PARPADEO_3 + posicion);

						estado = EST_PASS_U;
					}
				}
				break;

			case EST_SAT:
				if (teclado_msg.tecla == TECLA_C1)
				{
					escribir_id(eTXT_CONTADOR_PARCIAL, PARPADEO_NO);
					estado = EST_CONT_PAR;
				}
				else if (teclado_msg.tecla == TECLA_C2) // Salir
				{
					escribir_id(eTXT_SALIR, PARPADEO_NO);
					estado = EST_SALIR;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					posicion = 0;

					pwd[0] = '0';
					pwd[1] = '0';
					pwd[2] = '0';
					pwd[3] = '0';

					str[0] = ' ';
					str[1] = ' ';
					str[2] = pwd[0];
					str[3] = pwd[1];
					str[4] = pwd[2];
					str[5] = pwd[3];
					str[6] = ' ';
					str[7] = ' ';

					escribir_id(eTXT_PASSWORD, PARPADEO_NO);
					escribir_ln_str(str, LINEA_2, PARPADEO_3 + posicion);

					estado = EST_PASS_SU;
				}
				break;

			case EST_PASS_SU:
				if (teclado_msg.tecla == TECLA_C1)
				{
					if (pwd[posicion] <= '0')
					{
						pwd[posicion] = '9';
					}
					else
					{
						pwd[posicion]--;
					}
					str[posicion + 2] = pwd[posicion];
					escribir_ln_str(str, LINEA_2, PARPADEO_3 + posicion);
				}
				else if (teclado_msg.tecla == TECLA_C2)
				{
					if (pwd[posicion] >= '9')
					{
						pwd[posicion] = '0';
					}
					else
					{
						pwd[posicion]++;
					}
					str[posicion + 2] = pwd[posicion];
					escribir_ln_str(str, LINEA_2, PARPADEO_3 + posicion);
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					posicion++;
					str[posicion + 2] = pwd[posicion];
					if (posicion == POSICIONES_PASSWORD)
					{
						for (posicion = 0; posicion < POSICIONES_PASSWORD; posicion+=2)
						{
							word = leer_word_eeprom(DIRECC_PASSWORD_SU + posicion);
							pwd_mem[posicion] = word & 0x00FF;
							pwd_mem[posicion + 1] = (word & 0xFF00) >> 8;
						}
						if ((pwd[0] == pwd_mem[0]) && (pwd[1] == pwd_mem[1]) && (pwd[2] == pwd_mem[2]) && (pwd[3] == pwd_mem[3]))
						{
							escribir_ln_id(eTXT_SW_VERSION, LINEA_1, PARPADEO_NO);
							snprintf(version, sizeof(version), "%c%d.%d", PROGRAM_TYPE, VERSION_MAJOR, VERSION_MINOR);
							escribir_ln_str(version, LINEA_2, PARPADEO_NO);
							estado = EST_SAT_VERSION;
						}
						else
						{
							limpiar_pantalla();
							estado = EST_ENTRADA;
							xEventGroupSetBits(task_events, EV_CONFIG);
						}
					}
					else
					{
						escribir_ln_str(str, LINEA_2, PARPADEO_3 + posicion);
					}
				}
				break;

			case EST_CONT_PAR_C1:
				if (teclado_msg.tecla == TECLA_C2)
				{
					cont = leer_int_eeprom(DIRECC_CONTADOR_2);
					escribir_id(eTXT_PARC_C2, PARPADEO_NO);
					escribir_ln_num(cont, LINEA_2, PARPADEO_NO);
					estado = EST_CONT_PAR_C2;
				}
				break;

			case EST_CONT_PAR_C2:
				if (teclado_msg.tecla == TECLA_C1)
				{
					cont = leer_int_eeprom(DIRECC_CONTADOR_1);
					escribir_id(eTXT_PARC_C1, PARPADEO_NO);
					escribir_ln_num(cont, LINEA_2, PARPADEO_NO);
					estado = EST_CONT_PAR_C1;
				}
				else if (teclado_msg.tecla == TECLA_C2)
				{
					cont = leer_int_eeprom(DIRECC_CONTADOR_1) + leer_int_eeprom(DIRECC_CONTADOR_2) * 2;
					escribir_id(eTXT_SUMA_PARCIALES, PARPADEO_NO);
					escribir_ln_num(cont, LINEA_2, PARPADEO_NO);
					estado = EST_CONT_PAR_SUMA;
				}
				break;

			case EST_CONT_PAR_SUMA:
				if (teclado_msg.tecla == TECLA_C1)
				{
					cont = leer_int_eeprom(DIRECC_CONTADOR_2);
					escribir_id(eTXT_PARC_C2, PARPADEO_NO);
					escribir_ln_num(cont, LINEA_2, PARPADEO_NO);
					estado = EST_CONT_PAR_C2;
				}
				else if (teclado_msg.tecla == TECLA_C2)
				{
					escribir_id(eTXT_BORRAR, PARPADEO_NO);
					estado = EST_CONT_PAR_BORRAR;
				}
				break;

			case EST_CONT_PAR_BORRAR:
				if (teclado_msg.tecla == TECLA_C1)
				{
					cont = leer_int_eeprom(DIRECC_CONTADOR_1) + leer_int_eeprom(DIRECC_CONTADOR_2) * 2;
					escribir_id(eTXT_SUMA_PARCIALES, PARPADEO_NO);
					escribir_ln_num(cont, LINEA_2, PARPADEO_NO);
					estado = EST_CONT_PAR_SUMA;
				}
				else if (teclado_msg.tecla == TECLA_C2)
				{
					escribir_id(eTXT_SALIR, PARPADEO_NO);
					estado = EST_CONT_PAR_SALIR;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					borrar = 0;
					escribir_ln_id(eTXT_SELECT_NO, LINEA_2, PARPADEO_NO);
					estado = EST_CONT_PAR_BORRAR_SEL;
				}
				break;

			case EST_CONT_PAR_BORRAR_SEL:
				if ((teclado_msg.tecla == TECLA_C1) || (teclado_msg.tecla == TECLA_C2))
				{
					if (borrar == 1)
					{
						borrar = 0;
						escribir_ln_id(eTXT_SELECT_NO, LINEA_2, PARPADEO_NO);
					}
					else // borrar == 0
					{
						borrar = 1;
						escribir_ln_id(eTXT_SELECT_SI, LINEA_2, PARPADEO_NO);
					}
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					if (borrar == 1)
					{
						borrar_contadores(PARCIALES);
					}
					cont = leer_int_eeprom(DIRECC_CONTADOR_1);
					escribir_id(eTXT_PARC_C1, PARPADEO_NO);
					escribir_ln_num(cont, LINEA_2, PARPADEO_NO);
					estado = EST_CONT_PAR_C1;
				}
				break;

			case EST_CONT_PAR_SALIR:
				if (teclado_msg.tecla == TECLA_C1)
				{
					escribir_id(eTXT_BORRAR, PARPADEO_NO);
					estado = EST_CONT_PAR_BORRAR;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE)) // Salir
				{
					limpiar_pantalla();
					estado = EST_ENTRADA;
					xEventGroupSetBits(task_events, EV_CONFIG);
				}
				break;

			case EST_SAT_VERSION:
				if (teclado_msg.tecla == TECLA_C2)
				{
					escribir_id(eTXT_CONTADOR_TOTAL, PARPADEO_NO);
					estado = EST_SAT_CONT_TOT;
				}
				break;

			case EST_SAT_CONT_TOT:
				if (teclado_msg.tecla == TECLA_C2)
				{
					escribir_id(eTXT_INDENTIFICACION_PORTA, PARPADEO_NO);
					estado = EST_SAT_IDENTIFICACION;
				}
				else if (teclado_msg.tecla == TECLA_C1)
				{
					escribir_ln_id(eTXT_SW_VERSION, LINEA_1, PARPADEO_NO);
					snprintf(version, sizeof(version), "%c%d.%d", PROGRAM_TYPE, VERSION_MAJOR, VERSION_MINOR);
					escribir_ln_str(version, LINEA_2, PARPADEO_NO);
					estado = EST_SAT_VERSION;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					cont = leer_int_eeprom(DIRECC_CONTADOR_T1);
					escribir_id(eTXT_TOT_C1, PARPADEO_NO);
					escribir_ln_num(cont, LINEA_2, PARPADEO_NO);
					estado = EST_SAT_CONT_TOT_C1;
				}
				break;

			case EST_SAT_CONT_TOT_C1:
				if (teclado_msg.tecla == TECLA_C2)
				{
					cont = leer_int_eeprom(DIRECC_CONTADOR_T2);
					escribir_id(eTXT_TOT_C2, PARPADEO_NO);
					escribir_ln_num(cont, LINEA_2, PARPADEO_NO);
					estado = EST_SAT_CONT_TOT_C2;
				}
				break;

			case EST_SAT_CONT_TOT_C2:
				if (teclado_msg.tecla == TECLA_C1)
				{
					cont = leer_int_eeprom(DIRECC_CONTADOR_T1);
					escribir_id(eTXT_TOT_C1, PARPADEO_NO);
					escribir_ln_num(cont, LINEA_2, PARPADEO_NO);
					estado = EST_SAT_CONT_TOT_C1;
				}
				else if (teclado_msg.tecla == TECLA_C2)
				{
					cont = leer_int_eeprom(DIRECC_CONTADOR_T1) + leer_int_eeprom(DIRECC_CONTADOR_T2) * 2;
					escribir_id(eTXT_SUMA_SERVICIOS, PARPADEO_NO);
					escribir_ln_num(cont, LINEA_2, PARPADEO_NO);
					estado = EST_SAT_CONT_TOT_SUMA;
				}
				break;

			case EST_SAT_CONT_TOT_SUMA:
				if (teclado_msg.tecla == TECLA_C1)
				{
					cont = leer_int_eeprom(DIRECC_CONTADOR_T2);
					escribir_id(eTXT_TOT_C2, PARPADEO_NO);
					escribir_ln_num(cont, LINEA_2, PARPADEO_NO);
					estado = EST_SAT_CONT_TOT_C2;
				}
				else if (teclado_msg.tecla == TECLA_C2)
				{
					escribir_id(eTXT_SALIR, PARPADEO_NO);
					estado = EST_SAT_CONT_TOT_SALIR;
				}
				break;

			case EST_SAT_CONT_TOT_SALIR:
				if (teclado_msg.tecla == TECLA_C1) // Ir a estado EST_CONT_TOT_SUMA
				{
					cont = leer_int_eeprom(DIRECC_CONTADOR_T1) + leer_int_eeprom(DIRECC_CONTADOR_T2) * 2;
					escribir_id(eTXT_SUMA_SERVICIOS, PARPADEO_NO);
					escribir_ln_num(cont, LINEA_2, PARPADEO_NO);
					estado = EST_SAT_CONT_TOT_SUMA;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE)) // Salir
				{
					escribir_id(eTXT_CONTADOR_TOTAL, PARPADEO_NO);
					estado = EST_SAT_CONT_TOT;
				}
				break;

			case EST_SAT_IDENTIFICACION:
				if (teclado_msg.tecla == TECLA_C1)
				{
					escribir_id(eTXT_CONTADOR_TOTAL, PARPADEO_NO);
					estado = EST_SAT_CONT_TOT;
				}
				else if (teclado_msg.tecla == TECLA_C2)
				{
					escribir_id(eTXT_MODO_TRABAJO, PARPADEO_NO);
					estado = EST_SAT_TRABAJO;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					switch (leer_word_eeprom (DIRECC_MODO_IDENTIF)) {
					case TECLADO_C1:
						escribir_id(eTXT_TECLA_C1, PARPADEO_NO);
						estado = EST_SAT_IDENTIFICACION_C1;
						break;
					case TECLADO_C2:
						escribir_id(eTXT_TECLA_C2, PARPADEO_NO);
						estado = EST_SAT_IDENTIFICACION_C2;
						break;
					case TECLADO_LAST:
						escribir_id(eTXT_TECLA_ULTIMA, PARPADEO_NO);
						estado = EST_SAT_IDENTIFICACION_ULTIMA;
						break;
					case DOBLE_CLICK:
						escribir_id(eTXT_DOBLE_PULSACION, PARPADEO_NO);
						estado = EST_SAT_IDENTIFICACION_DOBLE;
						break;
					case SENSOR_MAG:
						escribir_id(eTXT_SENSOR_MAGNETICO, PARPADEO_NO);
						estado = EST_SAT_IDENTIFICACION_MAG;
						break;
					default:
						escribir_id(eTXT_TECLA_C1, PARPADEO_NO);
						estado = EST_SAT_IDENTIFICACION_C1;
						break;
					}
				}
				break;

			case EST_SAT_IDENTIFICACION_C1:
				if (teclado_msg.tecla == TECLA_C1) // Ir a estado EST_SAT_IDENTIFICACION_MAG
				{
					if (check_typeA()){
						escribir_id(eTXT_SENSOR_MAGNETICO, PARPADEO_NO);
						estado = EST_SAT_IDENTIFICACION_MAG;
					} else if (check_typeB()){
						escribir_id(eTXT_DOBLE_PULSACION, PARPADEO_NO);
						estado = EST_SAT_IDENTIFICACION_DOBLE;
					}
				}
				else if (teclado_msg.tecla == TECLA_C2) // Ir a estado EST_SAT_IDENTIFICACION_C2
				{
					escribir_id(eTXT_TECLA_C2, PARPADEO_NO);
					estado = EST_SAT_IDENTIFICACION_C2;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
					// Guardar modo de identificación TECLADO_C1 e ir a estado EST_SAT_IDENTIFICACION
				{
					grabar_word_eeprom(DIRECC_MODO_IDENTIF, TECLADO_C1);
					escribir_id(eTXT_INDENTIFICACION_PORTA, PARPADEO_NO);
					estado = EST_SAT_IDENTIFICACION;
				}
				break;

			case EST_SAT_IDENTIFICACION_C2:
				if (teclado_msg.tecla == TECLA_C1) // Ir a estado EST_SAT_IDENTIFICACION_C1
				{
					escribir_id(eTXT_TECLA_C1, PARPADEO_NO);
					estado = EST_SAT_IDENTIFICACION_C1;
				}
				else if (teclado_msg.tecla == TECLA_C2) // Ir a estado EST_SAT_IDENTIFICACION_ULTIMA
				{
					escribir_id(eTXT_TECLA_ULTIMA, PARPADEO_NO);
					estado = EST_SAT_IDENTIFICACION_ULTIMA;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
					// Guardar modo de identificación TECLADO_C2 e ir a estado EST_SAT_IDENTIFICACION
				{
					grabar_word_eeprom(DIRECC_MODO_IDENTIF, TECLADO_C2);
					escribir_id(eTXT_INDENTIFICACION_PORTA, PARPADEO_NO);
					estado = EST_SAT_IDENTIFICACION;
				}
				break;

			case EST_SAT_IDENTIFICACION_ULTIMA:
				if (teclado_msg.tecla == TECLA_C1) // Ir a estado EST_SAT_IDENTIFICACION_C2
				{
					escribir_id(eTXT_TECLA_C2, PARPADEO_NO);
					estado = EST_SAT_IDENTIFICACION_C2;
				}
				else if (teclado_msg.tecla == TECLA_C2) // Ir a estado EST_SAT_IDENTIFICACION_DOBLE
				{
					escribir_id(eTXT_DOBLE_PULSACION, PARPADEO_NO);
					estado = EST_SAT_IDENTIFICACION_DOBLE;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
					// Guardar modo de identificación TECLADO_LAST e ir a estado EST_SAT_IDENTIFICACION
				{
					grabar_word_eeprom(DIRECC_MODO_IDENTIF, TECLADO_LAST);
					escribir_id(eTXT_INDENTIFICACION_PORTA, PARPADEO_NO);
					estado = EST_SAT_IDENTIFICACION;
				}
				break;

			case EST_SAT_IDENTIFICACION_DOBLE:
				if (teclado_msg.tecla == TECLA_C1) // Ir a estado EST_SAT_IDENTIFICACION_ULTIMA
				{
					escribir_id(eTXT_TECLA_ULTIMA, PARPADEO_NO);
					estado = EST_SAT_IDENTIFICACION_ULTIMA;
				}
				else if (teclado_msg.tecla == TECLA_C2) // Ir a estado EST_SAT_IDENTIFICACION_MAG
				{
					if (check_typeA()){
						escribir_id(eTXT_SENSOR_MAGNETICO, PARPADEO_NO);
						estado = EST_SAT_IDENTIFICACION_MAG;
					} else if (check_typeB()){
						escribir_id(eTXT_TECLA_C1, PARPADEO_NO);
						estado = EST_SAT_IDENTIFICACION_C1;
					}
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
					// Guardar modo de identificación TECLADO_LAST e ir a estado EST_SAT_IDENTIFICACION
				{
					grabar_word_eeprom(DIRECC_MODO_IDENTIF, DOBLE_CLICK);
					escribir_id(eTXT_INDENTIFICACION_PORTA, PARPADEO_NO);
					estado = EST_SAT_IDENTIFICACION;
				}
				break;

			case EST_SAT_IDENTIFICACION_MAG:
				if (teclado_msg.tecla == TECLA_C1) // Ir a estado EST_SAT_IDENTIFICACION_DOBLE
				{
					escribir_id(eTXT_DOBLE_PULSACION, PARPADEO_NO);
					estado = EST_SAT_IDENTIFICACION_DOBLE;
				}
				else if (teclado_msg.tecla == TECLA_C2) // Ir a estado EST_SAT_IDENTIFICACION_C1
				{
					escribir_id(eTXT_TECLA_C1, PARPADEO_NO);
					estado = EST_SAT_IDENTIFICACION_C1;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
					// Guardar modo de identificación SENSOR_MAG e ir a estado EST_SAT_IDENTIFICACION
				{
					grabar_word_eeprom(DIRECC_MODO_IDENTIF, SENSOR_MAG);
					//se apaga porque en task_main, al pasar por comprobar, se enciende y así se resetea
					//  al seleccionar. Si no, en alguna situación (p.ej. dejar porta doble colocado en tecla
					//  c1 puede hacer que la tarjeta MAG vaya al revés. y si se activa con password la tarjeta
					//  no se resetea.
					sensor_mag_off();
					vStopUnivTimer(ID_MAG);
					escribir_id(eTXT_INDENTIFICACION_PORTA, PARPADEO_NO);
					estado = EST_SAT_IDENTIFICACION;
				}
				break;

			case EST_SAT_TRABAJO:
				if (teclado_msg.tecla == TECLA_C1) // Ir a estado EST_SAT_IDENTIFICACION
				{
					escribir_id(eTXT_INDENTIFICACION_PORTA, PARPADEO_NO);
					estado = EST_SAT_IDENTIFICACION;
				}
				else if (teclado_msg.tecla == TECLA_C2) // Ir a estado EST_SAT_LIMPIEZA
				{
					escribir_id(eTXT_VACIADO_DOSIFICADOR, PARPADEO_NO);
					estado = EST_SAT_LIMPIEZA;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
					// Ir a estado EST_SAT_TRABAJO_ONDEMAND
				{
					if (leer_word_eeprom(DIRECC_FASTCHANGE_ON) == 1){
						escribir_id(eTXT_MOLIDO_CR, PARPADEO_NO);
						estado = EST_SAT_TRABAJO_CR;
					} else if (leer_word_eeprom(DIRECC_MODO_TRABAJO) == ONDEMAND){
						escribir_id_lcd(eTXT_MOLIDO_ONDEMAND, PARPADEO_NO);
						estado = EST_SAT_TRABAJO_ONDEMAND;
					} else {
						escribir_id(eTXT_PREMOLIDO, PARPADEO_NO);
						estado = EST_SAT_TRABAJO_PREMOLIDO;
					}
				}
				break;

			case EST_SAT_TRABAJO_ONDEMAND:
				if (teclado_msg.tecla == TECLA_C1)
				// Ir a estado EST_SAT_TRABAJO_PREMOLIDO
				{
					escribir_id(eTXT_PREMOLIDO, PARPADEO_NO);
					estado = EST_SAT_TRABAJO_PREMOLIDO;
				}
				else if (teclado_msg.tecla == TECLA_C2)
				{
					escribir_id(eTXT_MOLIDO_CR, PARPADEO_NO);
					estado = EST_SAT_TRABAJO_CR;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
					// Guardar modo de trabajo ONDEMAND e ir a estado EST_SAT_TRABAJO
				{
					grabar_word_eeprom(DIRECC_MODO_TRABAJO, ONDEMAND);
					escribir_id(eTXT_MODO_TRABAJO, PARPADEO_NO);
					grabar_word_eeprom(DIRECC_FASTCHANGE_ON, 0);
					estado = EST_SAT_TRABAJO;
				}
				break;

			case EST_SAT_TRABAJO_PREMOLIDO:
				if (teclado_msg.tecla == TECLA_C2)
				{
					escribir_id(eTXT_MOLIDO_ONDEMAND, PARPADEO_NO);
					estado = EST_SAT_TRABAJO_ONDEMAND;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				// Guardar modo de trabajo PREMOLIDO e ir a estado EST_SAT_TRABAJO
				{
					// PREMOLIDO (PREMOLIDO): Comprobamos que la variación de C2 está dentro de márgenes
					// fijados por C1 según el criterio:  C1 < C2 < 2*C1
					// 2018-12-11: Modificacion añadida a petición de VILAMATIC (Novell).
					if (leer_word_eeprom(DIRECC_DOSIS_2C) > (leer_word_eeprom(DIRECC_DOSIS_1C)*2))
					{
						grabar_word_eeprom(DIRECC_DOSIS_2C, (leer_word_eeprom(DIRECC_DOSIS_1C)*2));
					}
					grabar_word_eeprom(DIRECC_MODO_TRABAJO, PREMOLIDO);
					grabar_word_eeprom(DIRECC_FASTCHANGE_ON, 0);
					escribir_id(eTXT_MODO_TRABAJO, PARPADEO_NO);
					estado = EST_SAT_TRABAJO;
				}
				break;

			case EST_SAT_TRABAJO_CR:
				if (teclado_msg.tecla == TECLA_C1)
				{
					escribir_id(eTXT_MOLIDO_ONDEMAND, PARPADEO_NO);
					estado = EST_SAT_TRABAJO_ONDEMAND;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					if (leer_word_eeprom(DIRECC_DOSIS_2C) > (leer_word_eeprom(DIRECC_DOSIS_1C)*2))
					{
						grabar_word_eeprom(DIRECC_DOSIS_2C, (leer_word_eeprom(DIRECC_DOSIS_1C)*2));
					}
					grabar_word_eeprom(DIRECC_FASTCHANGE_ON,1);
					escribir_id(eTXT_MODO_TRABAJO, PARPADEO_NO);
					estado = EST_SAT_TRABAJO;
				}
				break;

			case EST_SAT_LIMPIEZA:
				if (teclado_msg.tecla == TECLA_C1) // Ir a estado EST_SAT_TRABAJO
				{
					escribir_id(eTXT_MODO_TRABAJO, PARPADEO_NO);
					estado = EST_SAT_TRABAJO;
				}
				else if (teclado_msg.tecla == TECLA_C2) // Ir a estado EST_SAT_PRUEBA
				{
					escribir_id(eTXT_PRUEBA_INTENSIVA, PARPADEO_NO);
					estado = EST_SAT_PRUEBA;
				}
				else if (((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE)) || directa_limpieza)
				{
					if (directa_limpieza == pdTRUE)
					{
						directa_limpieza = pdFALSE; //se resetea la variable nada más usarla
						mensaje_C1 = pdTRUE;
					}
					escribir_id(eTXT_INSERTAR_PORTA, PARPADEO_NO);
					tiempo_limpieza = 0;
					// Ojo: ahora nos paramos en este bucle sin volver por el bucle for(;;)
					while ( (!micro_porta_pulsado()) && (tiempo_limpieza < TIME_5MIN) ) //tiempo ampliado
					{
						xQueueReceive(handle_queue_teclado, &teclado_msg, 0);// Para no dejar teclas sin validar
						vTaskDelay(TIME_100MSEC);
						tiempo_limpieza += TIME_100MSEC;
						xEventGroupSetBits(wdt_events, EV_WDT_MAIN); // Para no disparar el watchdog
					}
					if (tiempo_limpieza < TIME_5MIN) //relativo al cambio de tiempo anterior
					{  // Se ha pulsado el micro porta seguimos bucle vaciando dosificador
						accion_motor(MOTOR_INFERIOR, ACCION_ABRIR);
						//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY); //Espera que termine el motor
						accion_motor(MOTOR_SUPERIOR, ACCION_ABRIR);
						//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY);
						if (mensaje_C1 == pdTRUE){
							escribir_id(eTXT_TECLA_C1, PARPADEO_NO);
							mensaje_C1 = pdFALSE;
						}
						else
							escribir_id(eTXT_TECLA_INTRO, PARPADEO_NO);

						//otro bucle de 5 min para pulsar la tecla
						tiempo_limpieza = 0; //se usa la misma variable que para el bucle anterior
						teclado_msg.tecla = 0x00; //se le da un valor a la tecla que normalmente no puede tener, para que no se salte el bucle de forma accidental
						while (teclado_msg.tecla != TECLA_C1 && teclado_msg.tecla != TECLA_AVANCE && tiempo_limpieza < TIME_5MIN)
						{
							xQueueReceive(handle_queue_teclado, &teclado_msg, 0);
							vTaskDelay(TIME_100MSEC);
							tiempo_limpieza += TIME_100MSEC;
							xEventGroupSetBits(wdt_events, EV_WDT_MAIN); //Para no disparar el watchdog. antes fallaba por el Watchdog
						}
						if (tiempo_limpieza < TIME_5MIN)
						{
							estado = EST_SAT_LIMPIEZA_VACIO;
							xQueueSend(handle_queue_teclado, &teclado_msg, 0); //se reenvía la tecla recibida para que entre normalmente
																			   //  por la condición principal del bucle
						}
						else // se sale por tiempo
						{
							rellenar_salir();
							limpiar_pantalla();
							estado = EST_ENTRADA;
							xEventGroupSetBits(task_events, EV_CONFIG);
						}
					}
					else
					{  // Salimos por tiempo
						limpiar_pantalla();
						estado = EST_ENTRADA;
						mensaje_C1 = pdFALSE;
						xEventGroupSetBits(task_events, EV_CONFIG);
					}
				}
				break;

			case EST_SAT_LIMPIEZA_VACIO:
				if (teclado_msg.tecla == TECLA_AVANCE)  // Fin
				{
					// Nos aseguramos de que la superior está abierta (si se cierra a mano falla)
					accion_motor(MOTOR_SUPERIOR, ACCION_ABRIR);
					//xEventGroupWaitBits(task_events, EV_MOTOR, pdTRUE, pdFALSE, portMAX_DELAY); //Espera que termine el motor

					rellenar_salir();

					limpiar_pantalla();
					estado = EST_ENTRADA;
					xEventGroupSetBits(task_events, EV_CONFIG);
				}
				else if (teclado_msg.tecla == TECLA_C1)
				{
					escribir_id(eTXT_SECUENCIA_PESADAS, PARPADEO_NO);
					num_ciclos_prueba_temp = 10; // Para entrar en ciclo pesadas
					// Ojo: ahora nos paramos en este bucle sin volver por el bucle for(;;)
					//  Ciclo de 10 dosis molidas con trampilla abierta.
					while (num_ciclos_prueba_temp > 0)
					{
						escribir_ln_num(num_ciclos_prueba_temp, LINEA_2, PARPADEO_NO);
						_motor_ac_on_(); // Moler dosis
						//se fijan las dosis a mano a C1: 2s y C2: 3.5s
						if (num_ciclos_prueba_temp % 2)
							vTaskDelay(PRUEBA_PESADAS_D1);
						else vTaskDelay(PRUEBA_PESADAS_D2);
						_motor_ac_off_();
						vTaskDelay(PRUEBA_PESADAS_PAUSA);
						num_ciclos_prueba_temp--;
						if (xQueueReceive(handle_queue_teclado, &teclado_msg, 0) == pdPASS)
						{ // Salir de prueba por tecla, sigo en menú posventa
							if (teclado_msg.tecla == TECLA_AVANCE) {
								num_ciclos_prueba_temp = 0;
							}
						}
						xEventGroupSetBits(wdt_events, EV_WDT_MAIN); // Watchdog
					}
					rellenar_salir();
					// Salimos del bucle servicio técnico
					limpiar_pantalla();
					estado = EST_ENTRADA;
					xEventGroupSetBits(task_events, EV_CONFIG);
				}
				break;

			case EST_SAT_PRUEBA:
				if (teclado_msg.tecla == TECLA_C1) // Ir a estado EST_SAT_LIMPIEZA
				{
					escribir_id(eTXT_VACIADO_DOSIFICADOR, PARPADEO_NO);
					estado = EST_SAT_LIMPIEZA;
				}
				else if (teclado_msg.tecla == TECLA_C2) // Ir a estado EST_SAT_FABRICA
				{
					escribir_id(eTXT_DATOS_FABRICA, PARPADEO_NO);
					estado = EST_SAT_FABRICA;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					escribir_id(eTXT_CICLOS, PARPADEO_NO);
					escribir_ln_num(num_ciclos_prueba, LINEA_2, PARPADEO_NO);
					estado = EST_SAT_PRUEBA_CONF;
				}
				break;

			case EST_SAT_PRUEBA_CONF:
				if (teclado_msg.tecla == TECLA_C1) // Restar ciclos
				{
					if (num_ciclos_prueba <= 10)
					{
						num_ciclos_prueba = 10;
					}
					else
					{
						num_ciclos_prueba-=10;
					}
					escribir_ln_num(num_ciclos_prueba, LINEA_2, PARPADEO_NO);
				}
				else if (teclado_msg.tecla == TECLA_C2) // Sumar ciclos
				{
					if (num_ciclos_prueba >= NUM_CICLOS_PRUEBA_MAX)
					{
						num_ciclos_prueba = NUM_CICLOS_PRUEBA_MAX;
					}
					else
					{
						num_ciclos_prueba+=10;
					}
					escribir_ln_num(num_ciclos_prueba, LINEA_2, PARPADEO_NO);
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					num_ciclos_prueba_temp = num_ciclos_prueba;
					xEventGroupSetBits(task_events,EV_INTENSIVA_ON);
					while (num_ciclos_prueba_temp > 0)
					{
						if (num_ciclos_prueba_temp % 2)
						{
							moler_dosificar(leer_word_eeprom(DIRECC_MODO_TRABAJO), SIMPLE);
						}
						else
						{
							moler_dosificar(leer_word_eeprom(DIRECC_MODO_TRABAJO), DOBLE);
						}
						uxBits = xEventGroupWaitBits(task_events, EV_FIN_SERV, pdTRUE, pdFALSE, portMAX_DELAY);
						if ((uxBits & EV_FIN_SERV) == EV_FIN_SERV)
						{
							if (num_ciclos_prueba_temp < NUM_CICLOS_PRUEBA_MAX)
							{
								num_ciclos_prueba_temp--;
							}
						}

						envia_sensores(); // Envía datos a la nube
						escribir_ln_num(num_ciclos_prueba_temp, LINEA_2, PARPADEO_NO);

						xEventGroupSetBits(wdt_events, EV_WDT_MAIN);  // Watchdog

						if (xQueueReceive(handle_queue_teclado, &teclado_msg, 0) == pdPASS)
						{
							if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE)) // Ir a estado EST_SAT_PRUEBA
							{
								escribir_id(eTXT_PRUEBA_INTENSIVA, PARPADEO_NO);
								xEventGroupClearBits(task_events,EV_INTENSIVA_ON);
								estado = EST_SAT_PRUEBA;
								break;
							}
						}
						vTaskDelay(TIME_8SEC);  // Parada entre ciclos como en el Quimboa --> 10s
						//vTaskDelay(TIME_6SEC);   // Parada entre ciclos pruebas vida componentes. Otros valores.
					}  //while (num_ciclos_prueba_temp > 0)
					escribir_id(eTXT_PRUEBA_INTENSIVA, PARPADEO_NO);
					xEventGroupClearBits(task_events,EV_INTENSIVA_ON);
					estado = EST_SAT_PRUEBA;
				}
				break;

			case EST_SAT_FABRICA:
				if (teclado_msg.tecla == TECLA_C1) // Ir a estado EST_SAT_PRUEBA
				{
					escribir_id(eTXT_PRUEBA_INTENSIVA, PARPADEO_NO);
					estado = EST_SAT_PRUEBA;
				}
				else if (teclado_msg.tecla == TECLA_C2) // Ir a estado EST_SAT_SERIE
				{
					escribir_id(eTXT_NUMERO_SERIE, PARPADEO_NO);
					estado = EST_SAT_SERIE;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					activar = 0;
					escribir_ln_id(eTXT_SELECT_NO, LINEA_2, PARPADEO_NO);
					estado = EST_SAT_FABRICA_VALIDAR;
				}
				break;

			case EST_SAT_FABRICA_VALIDAR:
				if ((teclado_msg.tecla == TECLA_C1) || (teclado_msg.tecla == TECLA_C2))
				{
					if (activar == 1)
					{
						activar = 0;
						escribir_ln_id(eTXT_SELECT_NO, LINEA_2, PARPADEO_NO);
					}
					else
					{
						activar = 1;
						escribir_ln_id(eTXT_SELECT_SI, LINEA_2, PARPADEO_NO);
					}
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					if (activar == 1)
					{
						borrar_config();
						borrar_aviso();
						sensor_mag_off();
						limpiar_pantalla();
						estado = EST_ENTRADA;
						xEventGroupSetBits(task_events, EV_CONFIG);
					}
					else
					{
						//escribir_ln_id(eTXT_NULL, LINEA_2, PARPADEO_NO);
						escribir_id(eTXT_DATOS_FABRICA, PARPADEO_NO);
						estado = EST_SAT_FABRICA;
					}
				}
				else if (((teclado_msg.tecla == TECLA_C1C2) && (teclado_msg.pulsacion_larga == pdFALSE)) &&
					(activar == 1))
				{
					escribir_id(eTXT_MODELO, PARPADEO_NO);
					modelo = leer_word_eeprom(DIRECC_COD_CLIENTE);
					escribir_ln_id(eTXT_ORHI_STD_A + modelo, LINEA_2, PARPADEO_NO);
					estado = EST_SAT_MODELO_CONF;
				}
				break;

			case EST_SAT_MODELO_CONF:
				if ((teclado_msg.tecla == TECLA_C1) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					if( modelo <= 0) modelo = NUM_MODELOS - 1;
					else modelo--;
					escribir_ln_id(eTXT_ORHI_STD_A + modelo, LINEA_2, PARPADEO_NO);
				}
				else if ((teclado_msg.tecla == TECLA_C2) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					if ( ++modelo >= NUM_MODELOS) modelo = 0;
					escribir_ln_id(eTXT_ORHI_STD_A + modelo, LINEA_2, PARPADEO_NO);
				}
				else if ((teclado_msg. tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					grabar_word_eeprom(DIRECC_COD_CLIENTE, modelo);
					borrar_config();
					borrar_aviso();
					sensor_mag_off();
					limpiar_pantalla();
					estado = EST_ENTRADA;
					xEventGroupSetBits(task_events, EV_CONFIG);
				}
				break;
				
			case EST_SAT_SERIE:
				if (teclado_msg.tecla == TECLA_C1) // Ir a estado EST_SAT_FABRICA
				{
					escribir_id(eTXT_DATOS_FABRICA, PARPADEO_NO);
					estado = EST_SAT_FABRICA;
				}
				else if (teclado_msg.tecla == TECLA_C2) // Ir a estado EST_SAT_PASSWORD
				{
					escribir_id(eTXT_PASSWORD_USUARIO, PARPADEO_NO);
					estado = EST_SAT_PASSWORD_U;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					for (posicion = 0; posicion < POSICIONES_DEVNAME; posicion+=2)
					{
						word = leer_word_eeprom(DIRECC_DEVICENAME + posicion);
						serie[posicion] = word;
						serie[posicion + 1] = word >> 8;
					}
					posicion = 2;
					escribir_id(eTXT_N_SERIE, PARPADEO_NO);
					escribir_ln_str(serie, LINEA_2, PARPADEO_1 + posicion);
					estado = EST_SAT_SERIE_CONF;
				}
				break;

			case EST_SAT_SERIE_CONF:
				if (teclado_msg.tecla == TECLA_C1)
				{
					if (serie[posicion] <= '0')
					{
						serie[posicion] = '9';
					}
					else
					{
						serie[posicion]--;
					}
					escribir_ln_str(serie, LINEA_2, PARPADEO_1 + posicion);
				}
				else if (teclado_msg.tecla == TECLA_C2)
				{
					if (serie[posicion] >= '9')
					{
						serie[posicion] = '0';
					}
					else
					{
						serie[posicion]++;
					}
					escribir_ln_str(serie, LINEA_2, PARPADEO_1 + posicion);
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					posicion++;
					escribir_ln_str(serie, LINEA_2, PARPADEO_1 + posicion);
					if (posicion == POSICIONES_DEVNAME)
					{
						for (posicion = 2; posicion < POSICIONES_DEVNAME; posicion+=2)
						{
							grabar_word_eeprom(DIRECC_DEVICENAME + posicion, ((serie[posicion + 1]<<8) | serie[posicion]));
						}
						escribir_id(eTXT_NUMERO_SERIE, PARPADEO_NO);
						estado = EST_SAT_SERIE;
					}
				}
				break;

			case EST_SAT_PASSWORD_U:
				if (teclado_msg.tecla == TECLA_C1) // Ir a estado EST_SAT_SERIE
				{
					escribir_id(eTXT_NUMERO_SERIE, PARPADEO_NO);
					estado = EST_SAT_SERIE;
				}
				else if (teclado_msg.tecla == TECLA_C2) // Ir a estado EST_SAT_IDIOMA
				{
					escribir_id(eTXT_ELEGIR_IDIOMA, PARPADEO_NO);
					estado = EST_SAT_IDIOMA;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					activar = 0;
					escribir_ln_id(eTXT_SELECT_NO, LINEA_2, PARPADEO_NO);
					estado = EST_SAT_PASSWORD_U_SEL;
				}
				break;

			case EST_SAT_PASSWORD_U_SEL:
				if ((teclado_msg.tecla == TECLA_C1) || (teclado_msg.tecla == TECLA_C2))
				{
					if (activar == 1)
					{
						activar = 0;
						escribir_ln_id(eTXT_SELECT_NO, LINEA_2, PARPADEO_NO);
					}
					else
					{
						activar = 1;
						escribir_ln_id(eTXT_SELECT_SI, LINEA_2, PARPADEO_NO);
					}
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					grabar_word_eeprom(DIRECC_PASS_ACT, activar);

					if (activar == 1)
					{
						for (posicion = 0; posicion < POSICIONES_PASSWORD; posicion+=2)
						{
							word = leer_word_eeprom(DIRECC_PASSWORD_U + posicion);
							pwd[posicion] = word & 0x00FF;
							pwd[posicion + 1] = (word & 0xFF00) >> 8;
						}
						posicion = 0;
						escribir_id(eTXT_PASSWORD, PARPADEO_NO);
						str[0] = ' ';
						str[1] = ' ';
						str[2] = pwd[0];
						str[3] = pwd[1];
						str[4] = pwd[2];
						str[5] = pwd[3];
						str[6] = ' ';
						str[7] = ' ';

						escribir_ln_str(str, LINEA_2, PARPADEO_3 + posicion);
						estado = EST_SAT_PASSWORD_U_CONF;
					}
					else
					{
						escribir_id(eTXT_PASSWORD_USUARIO, PARPADEO_NO);
						estado = EST_SAT_PASSWORD_U;
					}
				}
				break;

			case EST_SAT_PASSWORD_U_CONF:
				if (teclado_msg.tecla == TECLA_C1)
				{
					if (pwd[posicion] <= '0')
					{
						pwd[posicion] = '9';
					}
					else
					{
						pwd[posicion]--;
					}
					str[posicion + 2] = pwd[posicion];
					escribir_ln_str(str, LINEA_2, PARPADEO_3 + posicion);
				}
				else if (teclado_msg.tecla == TECLA_C2)
				{
					if (pwd[posicion] >= '9')
					{
						pwd[posicion] = '0';
					}
					else
					{
						pwd[posicion]++;
					}
					str[posicion + 2] = pwd[posicion];
					escribir_ln_str(str, LINEA_2, PARPADEO_3 + posicion);
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					posicion++;
					str[posicion + 2] = pwd[posicion];
					if (posicion == POSICIONES_PASSWORD)
					{
						for (posicion = 0; posicion < POSICIONES_PASSWORD; posicion+=2)
						{
							grabar_word_eeprom(DIRECC_PASSWORD_U + posicion, ((pwd[posicion + 1] << 8) | pwd[posicion]));
						}
						escribir_id(eTXT_PASSWORD_USUARIO, PARPADEO_NO);
						estado = EST_SAT_PASSWORD_U;
					}
					else
					{
						escribir_ln_str(str, LINEA_2, PARPADEO_3 + posicion);
					}
				}
				break;

			case EST_SAT_IDIOMA:
				if (teclado_msg.tecla == TECLA_C1) // Ir a estado EST_SAT_PASSWORD
				{
					escribir_id(eTXT_PASSWORD_USUARIO, PARPADEO_NO);
					estado = EST_SAT_PASSWORD_U;
				}
				else if (teclado_msg.tecla == TECLA_C2) // Ir a estado EST_SAT_BACKLIGHT
				{
					escribir_id(eTXT_NIVEL_LUZ, PARPADEO_NO);
					estado = EST_SAT_BACKLIGHT;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					escribir_id(eTXT_IDIOMA, PARPADEO_NO);
					idioma = leer_word_eeprom (DIRECC_IDIOMA);
					escribir_ln_id(eTXT_INGLES + idioma, LINEA_2, PARPADEO_NO);
					estado = EST_SAT_IDIOMA_CONF;
				}
				break;

			case EST_SAT_IDIOMA_CONF:
				if (teclado_msg.tecla == TECLA_C1) // Ir a estado EST_SAT_PASSWORD
				{
					if (idioma <= 0)
					{
						idioma = NUM_IDIOMAS - 1;
					}
					else
					{
						idioma--;
					}
					escribir_ln_id(eTXT_INGLES + idioma, LINEA_2, PARPADEO_NO);
				}
				else if (teclado_msg.tecla == TECLA_C2) // Ir a estado EST_SAT_BACKLIGHT
				{
					if (idioma >= NUM_IDIOMAS - 1)
					{
						idioma = 0; // INGLES
					}
					else
					{
						idioma++;
					}
					escribir_ln_id(eTXT_INGLES + idioma, LINEA_2, PARPADEO_NO);
				}          else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					grabar_word_eeprom(DIRECC_IDIOMA, idioma);
					idioma_lcd(leer_word_eeprom(DIRECC_IDIOMA));
					escribir_id(eTXT_ELEGIR_IDIOMA, PARPADEO_NO);
					estado = EST_SAT_IDIOMA;
				}
				break;

			case EST_SAT_BACKLIGHT:
				if (teclado_msg.tecla == TECLA_C1) // Ir a estado EST_SAT_IDIOMA
				{
					escribir_id(eTXT_ELEGIR_IDIOMA, PARPADEO_NO);
					estado = EST_SAT_IDIOMA;
				}
				else if (teclado_msg.tecla == TECLA_C2) // Ir a estado EST_SAT_SALIR
				{
					escribir_id(eTXT_AVFR, PARPADEO_NO);
					estado = EST_SAT_AVFR;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					nivel = leer_word_eeprom(DIRECC_BACKLIGHT);
					escribir_id(eTXT_NIVEL, PARPADEO_NO);
					escribir_ln_num(nivel, LINEA_2, PARPADEO_NO);
					estado = EST_SAT_BACKLIGHT_CONF;
				}
				break;

			case EST_SAT_BACKLIGHT_CONF:
				if (teclado_msg.tecla == TECLA_C1) // Restar nivel de intensida
				{
					if (nivel != 0)
					{
						nivel--;
					}
					backlight_set_duty(nivel*10);
					escribir_id(eTXT_NIVEL, PARPADEO_NO);
					escribir_ln_num(nivel, LINEA_2, PARPADEO_NO);
				}
				else if (teclado_msg.tecla == TECLA_C2) // Sumar nivel de intensidad
				{
					if (nivel < 10)
					{
						nivel++;
					}
					backlight_set_duty(nivel*10);
					escribir_id(eTXT_NIVEL, PARPADEO_NO);
					escribir_ln_num(nivel, LINEA_2, PARPADEO_NO);
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE)) // Ir a estado EST_SAT_BACKLIGHT
				{
					grabar_word_eeprom(DIRECC_BACKLIGHT, nivel);
					escribir_id(eTXT_NIVEL_LUZ, PARPADEO_NO);
					estado = EST_SAT_BACKLIGHT;
				}
				break;

			case EST_SAT_AVFR:
				if (teclado_msg.tecla == TECLA_C1)
				{
					escribir_id(eTXT_NIVEL_LUZ, PARPADEO_NO);
					estado = EST_SAT_BACKLIGHT;
				}
				else if (teclado_msg.tecla == TECLA_C2)
				{
					escribir_id(eTXT_SALIR, PARPADEO_NO);
					estado = EST_SAT_SALIR;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					avfr_activa = leer_word_eeprom(DIRECC_AVFR_ACTIVA);
					if(avfr_activa == 0){
						escribir_ln_id(eTXT_SELECT_NO, LINEA_2, PARPADEO_NO);
					}
					else if (avfr_activa == 1){
						escribir_ln_id(eTXT_SELECT_SI, LINEA_2, PARPADEO_NO);
					}
					estado = EST_SAT_AVFR_CONF;
				}
				break;

			case EST_SAT_AVFR_CONF:
				if (teclado_msg.tecla == TECLA_C1 || teclado_msg.tecla == TECLA_C2)
				{
					avfr_activa = (avfr_activa == 0); //cada vez que das a una de las dos teclas modificas la variable de 0 a 1 o viceversa
					if(avfr_activa == 0){
						escribir_ln_id(eTXT_SELECT_NO, LINEA_2, PARPADEO_NO);
					}
					else if (avfr_activa == 1){
						escribir_ln_id(eTXT_SELECT_SI, LINEA_2, PARPADEO_NO);
					}
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					//guarda el valor de activación en memoria eeprom
					grabar_word_eeprom(DIRECC_AVFR_ACTIVA, avfr_activa);

					if(avfr_activa == 0)
					{
						//salir al menú superior
					    escribir_id(eTXT_AVFR, PARPADEO_NO);
					    estado = EST_SAT_AVFR;
					}
					else if (avfr_activa == 1)
					{
						//pasa al menú de configurar número de cafés
					    ncafes = leer_int_eeprom(DIRECC_AVFR_NCAFES); //se lee el valor 1 vez para mostrarlo cada vez que se entre al bucle en
					        																										//  el siguiente estado
					    escribir_id(eTXT_NCAFES, PARPADEO_NO);
					    escribir_ln_num(ncafes, LINEA_2, PARPADEO_NO);
					    estado = EST_SAT_AVFR_NCAFES;
					}
					avfr_activa = 0; //se resetea variable
				}
				break;

			case EST_SAT_AVFR_NCAFES:
				//menú que avanza de 1000 o 10000 en función de si se mantiene tecla. y cuando se pulsa avance, lo guarda.

				if (teclado_msg.tecla == TECLA_C2 || teclado_msg.tecla == TECLA_C1 )
				{
					incr = ((teclado_msg.tecla == TECLA_C2) - (teclado_msg.tecla == TECLA_C1))*
				    (1000*(teclado_msg.pulsacion_larga == pdFALSE) + 10000*(teclado_msg.pulsacion_larga == pdTRUE))
					- ncafes%1000; //con ncafes%1000 se trunca el valor para que siempre sea múltiplo de 1000
				    ncafes += incr;
				    if (incr < 0 && (ncafes > NCAFES_MAX || ncafes < NCAFES_MIN)) ncafes = NCAFES_MAX;
				    else if (incr > 0 && (ncafes < NCAFES_MIN || ncafes > NCAFES_MAX)) ncafes = NCAFES_MIN;
				    escribir_ln_num(ncafes, LINEA_2, PARPADEO_NO);
				    incr = 0; //se resetea variable
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					grabar_int_eeprom(DIRECC_AVFR_NCAFES,ncafes);

					ncafes = 0; //se resetea variable
					//salimos a menú superior
					escribir_id(eTXT_AVFR, PARPADEO_NO);
					estado = EST_SAT_AVFR;
				}
				break;

			case EST_SAT_SALIR:
				if (teclado_msg.tecla == TECLA_C1) // Ir a estado EST_SAT_IDIOMA
				{
					escribir_id(eTXT_AVFR, PARPADEO_NO);
					estado = EST_SAT_AVFR;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE)) // Salir
				{
					limpiar_pantalla();
					estado = EST_ENTRADA;
					xEventGroupSetBits(task_events, EV_CONFIG);
				}
				break;

			case EST_SALIR:
				if (teclado_msg.tecla == TECLA_C1) // Ir a estado EST_SAT
				{
					escribir_id(eTXT_SERVICIO_TECNICO, PARPADEO_NO);
					estado = EST_SAT;
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					limpiar_pantalla();
					estado = EST_ENTRADA;
					xEventGroupSetBits(task_events, EV_CONFIG);
				}
				break;

			default:
				break;
			} // switch

			xEventGroupSetBits(wdt_events, EV_WDT_MAIN); // Refresco watchdog
		} // if
		else
		{
			limpiar_pantalla();
			estado = EST_ENTRADA;
			xEventGroupSetBits(task_events, EV_CONFIG);
		}
	} // for(;;)
}
