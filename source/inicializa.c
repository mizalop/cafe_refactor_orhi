/*
 * inicializa.c
 *
 *  Created on: 24 oct. 2022
 *      Author: mikel
 */

#include "teclado.h"
#include "task_main.h"
#include "eeprom.h"
#include "AG5_define.h"
#include "main.h"
#include "text_ids.h"
#include "hmi.h"
#include "rtos_time.h"
#include "avisoFresas.h"
#include "backlight.h"
#include "task_cafe.h"
#include "task_dosis.h"

#include "inicializa.h"

/*******************************************************************************
 * Definiciones
 ******************************************************************************/
#define NUMERO_CICLOS 	6000  // Ciclos de 10ms cada uno --> 60 segundos

/*******************************************************************************
 * Funciones
 ******************************************************************************/

void inicializar_config(void)
{
	if (cont_servicios_ok() == NO)
	{
		borrar_contadores(PARCIALES);
		//activa_fallo(FALLO_CONT_PARCIALES);
	}

	if (cont_totales_ok() == NO)
	{
		borrar_contadores(TOTALES);
		//activa_fallo(FALLO_CONT_TOTALES);
	}

	if (dosis_ok() == NO)
	{
		borrar_dosis();
		//activa_fallo(FALLO_DOSIS);
	}

	if (config_ok() == NO)
	{
		borrar_config();
		//activa_fallo(FALLO_CONFIGURACION);
	}
	if (aviso_ok() == NO)
	{
		borrar_aviso();
	}

	backlight_set_eeprom();

	// Establecer idioma guardado en EEPROM
	idioma_config(leer_word_eeprom(DIRECC_IDIOMA));

	//  if (password_ok() == NO)
	//  {
	//    dummy = borra_password(INICIAR);
	//    activa_fallo(FALLO_CONFIGURACION);
	//  }
}

void inicializar_idioma(void)
{
	uint8_t idioma = CASTELLANO;
	uint16_t contador_vueltas = 0;
	TECLADO_MSG_t teclado_msg;

	if (leer_word_eeprom(DIRECC_INICIALIZADO) != SI)
	{	// Solo se hace esto una vez, al volver de reset tras Datos Fabrica
		idioma_config(CASTELLANO);
		escribir_id(eTXT_ELEGIR_IDIOMA, PARPADEO_NO);
		vTaskDelay(TIME_1SEC);
		contador_vueltas = 0;
		limpiar_pantalla();
		escribir_ln_id(eTXT_USE_KEYS, LINEA_1, PARPADEO_NO);
		escribir_ln_id(eTXT_INGLES + idioma, LINEA_2, PARPADEO_NO);

		while (contador_vueltas < NUMERO_CICLOS)
		{
			// Cada vez esta 10ms esperando a ver si hay mensaje el la cola de teclado
			if (xQueueReceive(handle_queue_teclado, &teclado_msg, TIME_10MSEC) == pdPASS)
			{
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
					contador_vueltas = 0;
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
					contador_vueltas = 0;
					escribir_ln_id(eTXT_INGLES + idioma, LINEA_2, PARPADEO_NO);
				}
				else if ((teclado_msg.tecla == TECLA_AVANCE) && (teclado_msg.pulsacion_larga == pdFALSE))
				{
					contador_vueltas = NUMERO_CICLOS; // Para salir del bucle
				}
			}
			// Cada TIME_10MSEC vuelve por aqui
			contador_vueltas++; // Incremento contador de escape
			if (((contador_vueltas - 80) % 100) == 0)
				limpiar_ln(LINEA_2);
			else if ((contador_vueltas % 100) == 0)
				escribir_ln_id(eTXT_INGLES + idioma, LINEA_2, PARPADEO_NO);

			xEventGroupSetBits(wdt_events, EV_WDT_MAIN); // Para no disparar el watchdog
		}
		grabar_word_eeprom(DIRECC_IDIOMA, idioma);

		grabar_word_eeprom(DIRECC_INICIALIZADO, SI);
	} // Ya estaba inicializado de antes

	// Establecer idioma guardado en EEPROM
	idioma_config(leer_word_eeprom(DIRECC_IDIOMA));
}

uint8_t config_ok(void)
{
	// Preconfiguraciones de Orhi (p. ej. en izaga: KEY, BaristaGroup 1 y 2
	if (leer_word_eeprom(DIRECC_COD_CLIENTE) >= NUM_MODELOS) return 0;
	// Modos de trabajo --> BARISTA, FRESH, FAST
	if (leer_word_eeprom(DIRECC_MODO_TRABAJO) >= NUM_OPCIONES_MODO) return 0;
	// Configuración incorrecta para Orhi B
	if (check_typeB() &&  (leer_word_eeprom(DIRECC_MODO_IDENTIF) == SENSOR_MAG)) return 0;
	// Identificacion portas ->
	// SENSOR_MAG, ETIQUETA_NEGRA, DOBLE_CLIK, TECLADO_C1, TECLADO_C2, TECLADO_LAST
	if (leer_word_eeprom(DIRECC_MODO_IDENTIF) >= NUM_OPCIONES_IDENTIF) return 0;
	// Idiomas de los mensajes --> CASTELLANO, INGLES
	if (leer_word_eeprom(DIRECC_IDIOMA) >= NUM_IDIOMAS) return 0;
	// Numero de serie o DEVNAME --> empieza por "G5"
	if (leer_word_eeprom(DIRECC_DEVICENAME) != 0x3547) return 0;
	// Cambio rápido de modo activado o desactivado (0 o 1)
	// Guardarlo en flag
	if (leer_word_eeprom(DIRECC_FASTCHANGE_ON) > 1) return 0;
	// Inicializado tras datos de fábrica: sí(1) o no(0)
	if (leer_word_eeprom(DIRECC_INICIALIZADO) > SI) return 0;

	return 1; // Correcto
}

void borrar_config(void)
{
	if (leer_word_eeprom(DIRECC_COD_CLIENTE) >= NUM_MODELOS){
		// Si hay código ya programado no se machaca
		grabar_word_eeprom(DIRECC_COD_CLIENTE, ORHI_STD_A);
	}
	borrar_dosis();
	borrar_contadores(PARCIALES);
	grabar_word_eeprom(DIRECC_MODO_TRABAJO, PREMOLIDO);
	grabar_word_eeprom(DIRECC_MODO_TRABAJO_PREV, PREMOLIDO);
	grabar_word_eeprom(DIRECC_ESTADO_TRAMPILLA, T1T2CAFE);
	//  en función de usuario:
	//grabar_word_eeprom(DIRECC_MODO_IDENTIF, SENSOR_MAG);
	grabar_word_eeprom(DIRECC_PASS_ACT, 0);
	grabar_word_eeprom(DIRECC_PASSWORD_U, 0x3030); // "00"
	grabar_word_eeprom(DIRECC_PASSWORD_U + 0x02, 0x3030); // "00"
	grabar_word_eeprom(DIRECC_PASSWORD_SU, 0x3131); // "11"
	grabar_word_eeprom(DIRECC_PASSWORD_SU + 0x02, 0x3131); // "11"
	grabar_word_eeprom(DIRECC_TAG_C1C2, TAG_C2);
	//  en función de usuario:
	//grabar_word_eeprom(DIRECC_IDIOMA, CASTELLANO);
	grabar_word_eeprom(DIRECC_BACKLIGHT, 6);
	if (leer_word_eeprom(DIRECC_DEVICENAME) != 0x3547) // No es "G5"
	{	// Si había un código ya programado (empieza por G5) no lo machaco
		grabar_word_eeprom(DIRECC_DEVICENAME, 0x3547); // "G5"
		grabar_word_eeprom(DIRECC_DEVICENAME + 0x02, 0x3030); // "00"
		grabar_word_eeprom(DIRECC_DEVICENAME + 0x04, 0x3030); // "00"
		grabar_word_eeprom(DIRECC_DEVICENAME + 0x06, 0x3030); // "00"
	}
	//grabar_word_eeprom(DIRECC_TEMPERATURA, 1); //No tiene ventilador
	//  en función de usuario:
	//grabar_word_eeprom(DIRECC_FASTCHANGE_ON, OFF);
	grabar_word_eeprom(DIRECC_INICIALIZADO,NO);
	//El idioma se configura en el menú de configuración de idioma
    //grabar_word_eeprom(DIRECC_IDIOMA, INGLES);
	grabar_word_eeprom(DIRECC_FASTCHANGE_ON, ON);
	if (check_typeA())
		grabar_word_eeprom(DIRECC_MODO_IDENTIF, SENSOR_MAG);
	else if (check_typeB())
		grabar_word_eeprom(DIRECC_MODO_IDENTIF, TECLADO_C1);
}  // borrar_configuración
