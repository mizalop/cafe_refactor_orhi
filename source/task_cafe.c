/*
 * task_cafe2.c
 *
 *  Created on: 11 nov 2025
 *      Author: mikel
 */


/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#include "fsl_debug_console.h"

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

typedef enum{
	MOLIDO_INF,
	MOLIDO_SUP,
	VACIADO_INF,
	VACIADO_SUP,
	FINAL,
	SALIR
}Accion_Molido_e;

//Definición de secuencias:
Accion_Molido_e seq_doble_ondemand_nocafe[4] = {MOLIDO_INF,VACIADO_INF,FINAL,SALIR};
Accion_Molido_e seq_doble_ondemand_t1[4] = {MOLIDO_INF,VACIADO_INF,FINAL,SALIR};
Accion_Molido_e seq_doble_ondemand_t1t2[4] = {VACIADO_SUP,VACIADO_INF,FINAL,SALIR};
Accion_Molido_e seq_doble_premolido[6] = {VACIADO_SUP,VACIADO_INF,MOLIDO_INF,MOLIDO_SUP,FINAL,SALIR};
Accion_Molido_e seq_simple_ondemand_nocafe[4] = {MOLIDO_INF,VACIADO_INF,FINAL,SALIR};
Accion_Molido_e seq_simple_ondemand_t1[2] = {VACIADO_INF,SALIR};
Accion_Molido_e seq_simple_ondemand_t1t2[5] = {VACIADO_INF,VACIADO_SUP,MOLIDO_INF,FINAL,SALIR};
Accion_Molido_e seq_simple_premolido[6] = {VACIADO_INF,VACIADO_SUP,MOLIDO_INF,MOLIDO_SUP,FINAL,SALIR};

/*******************************************************************************
 * Variables Privadas
 ******************************************************************************/

static uint32_t dosis_1c = DOSIS_1C_FAB;      // Tiempo de dosificación en milisegundos
static uint32_t dosis_2c = DOSIS_2C_FAB;
static uint32_t tiempo_moliendo = 0;

static xQueueHandle handle_queue_orhi = 0;
static ORHI_MSG_t orhi_msg;

TickType_t calcular_tiempo_moliendo_inf(MODO_TRABAJO_e modo, TIPO_SERVICIO_e serv, ESTADO_TRAMPILLAS_e tramp);
void mensaje_pantalla_molido_inf(MODO_TRABAJO_e modo, TIPO_SERVICIO_e serv, ESTADO_TRAMPILLAS_e tramp, const TickType_t *pt_moliendo);
void vaciado_superior();
void vaciado_inferior(MODO_TRABAJO_e modo, TIPO_SERVICIO_e serv);
void molido_inferior(MODO_TRABAJO_e modo, TIPO_SERVICIO_e serv, ESTADO_TRAMPILLAS_e tramp);
void molido_superior(TIPO_SERVICIO_e serv);
void tareas_finales(MODO_TRABAJO_e modo, TIPO_SERVICIO_e serv, ESTADO_TRAMPILLAS_e tramp);

void seleccionar_seq_molido(Accion_Molido_e **seq, TIPO_SERVICIO_e serv, MODO_TRABAJO_e modo, ESTADO_TRAMPILLAS_e tramp)
{
	if ((serv != SIMPLE) && (serv != DOBLE)) return;
	if ((modo != PREMOLIDO) && (modo != ONDEMAND)) return;
	if ((tramp != NO_CAFE) && (tramp != T1CAFE) && (tramp != T1T2CAFE)) return;

	if (modo == PREMOLIDO)
	{
		if (serv == SIMPLE) *seq = seq_simple_premolido;
		else                *seq = seq_doble_premolido;
	}
	else //modo = ONDEMAND
	{
		switch(tramp)
		{
		case NO_CAFE:
			if (serv == SIMPLE) *seq = seq_simple_ondemand_nocafe;
			else                *seq = seq_doble_ondemand_nocafe;
			break;
		case T1CAFE:
			if (serv == SIMPLE) *seq = seq_simple_ondemand_t1;
			else				*seq = seq_doble_ondemand_t1;
			break;
		case T1T2CAFE:
			if (serv == SIMPLE) *seq = seq_simple_ondemand_t1t2;
			else				*seq = seq_doble_ondemand_t1t2;
			break;
		}
	}
}

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
			if (orhi_msg.servicio == ORHI_INICIAR)
			{
				dosis_1c = leer_word_eeprom(DIRECC_DOSIS_1C);
				dosis_2c = leer_word_eeprom(DIRECC_DOSIS_2C);
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
				}
				// Fin servicio, reset de teclado, flag fin secuencia
				if ((xEventGroupGetBits(task_events) & EV_INTENSIVA_ON) == 0 ) //si estamos en prueba intensiva no resetea teclado
					xQueueReset(handle_queue_teclado);						   //  para detectar cuando se quiere parar la prueba
				xEventGroupSetBits(task_events, EV_FIN_SERV);
			}
			else
			{
				Accion_Molido_e *seq_molido = NULL;
				uint8_t idx = 0;
				MODO_TRABAJO_e modo = orhi_msg.modo;
				TIPO_SERVICIO_e serv = orhi_msg.servicio;
				ESTADO_TRAMPILLAS_e tramp = leer_word_eeprom(DIRECC_ESTADO_TRAMPILLA);

				PRINTF("\n");
				seleccionar_seq_molido(&seq_molido, serv, modo, tramp);

				while (seq_molido[idx] != SALIR)
				{
					switch(seq_molido[idx])
					{
					case MOLIDO_INF:
						molido_inferior(modo,serv,tramp);
						PRINTF("Molido inferior\n");
						break;
					case MOLIDO_SUP:
						molido_superior(serv);
						PRINTF("Molido superior\n");
						break;
					case VACIADO_INF:
						vaciado_inferior(modo,serv);
						PRINTF("Vaciado inferior\n");
						break;
					case VACIADO_SUP:
						vaciado_superior();
						PRINTF("Vaciado superior\n");
						break;
					case FINAL:
						tareas_finales(modo,serv,tramp);
						PRINTF("Ejecutando tareas finales - contadores, lcd...\n");
						break;
					case SALIR:
						PRINTF("Salir del bucle\n");
						break;
					default:
						break;
					}
					vTaskDelay(TIME_5MSEC);
					idx++;
				}
			}
		} // if (xQueueReceive(handle_queue_orhi, &orhi_msg, portMAX_DELAY) == pdPASS)
	}
}

TickType_t calcular_tiempo_moliendo_inf(MODO_TRABAJO_e modo, TIPO_SERVICIO_e serv, ESTADO_TRAMPILLAS_e tramp)
{
	uint16_t dosis_1c = leer_word_eeprom(DIRECC_DOSIS_1C);
	uint16_t dosis_2c = leer_word_eeprom(DIRECC_DOSIS_2C);

	switch(modo)
	{
	case PREMOLIDO:
		if (serv == SIMPLE) return(TickType_t)(dosis_1c*2 - dosis_2c);
		else                return(TickType_t)(dosis_1c);
		break;
	case ONDEMAND:
		if (serv == SIMPLE)
		{
			switch(tramp)
			{
			case NO_CAFE:
				return (TickType_t)(dosis_1c);
				break;
			case T1CAFE:
				return (TickType_t)0;
				break;
			case T1T2CAFE:
				return (TickType_t)(dosis_1c*2 - dosis_2c);
				break;
			}
		}
		else
		{
			switch(tramp)
			{
			case NO_CAFE:
				return (TickType_t)(dosis_2c);
				break;
			case T1CAFE:
				return (TickType_t)(dosis_2c - dosis_1c);
				break;
			case T1T2CAFE:
				return (TickType_t)0;
				break;
			}
		}
		break;
	default:
		return -1;
		break;
	}
	return -1;
}

void mensaje_pantalla_molido_inf(MODO_TRABAJO_e modo, TIPO_SERVICIO_e serv, ESTADO_TRAMPILLAS_e tramp, const TickType_t *pt_moliendo)
{
	TEXT_ID_e text;
	LINEA_LCD_e line;

	if (modo == PREMOLIDO)
	{
		line = LINEA_2;
		if (serv == SIMPLE)
		{
			text = eTXT_C1_PREM;
		}
		else
		{
			text = eTXT_C2_PREM;
		}
	}
	else
	{
		if (serv == SIMPLE)
		{
			text = eTXT_C1_ONDEM;
			line = LINEA_2;
		}
		else
		{
			text = eTXT_C2_ONDEM;
			if (tramp == NO_CAFE)
			{
				line = LINEA_1_2;
			}
			else
			{
				line = LINEA_1;
			}
		}
	}

	escribir_ln_id_llenarbox(text,line,*pt_moliendo);
}

void vaciado_superior()
{
	accion_motor(MOTOR_SUPERIOR, ACCION_ABRIR);
}

void vaciado_inferior(MODO_TRABAJO_e modo, TIPO_SERVICIO_e serv)
{
	/*todo: Revisar esta función porque al unificar se meten cambios respecto al código original*/

	accion_motor(MOTOR_INFERIOR, ACCION_ABRIR);

	escribir_ln_id_box(eTXT_EMPTY_BOX, serv);
	vTaskDelay(TIEMPO_DESCARGA);

	//revisar qué hace esto y por qué no está para premolido! ondemand doble
	if (modo == ONDEMAND) escribir_ln_id_box(eTXT_ONDE, serv);

	accion_motor(MOTOR_INFERIOR, ACCION_CERRAR);
}

void molido_inferior(MODO_TRABAJO_e modo, TIPO_SERVICIO_e serv, ESTADO_TRAMPILLAS_e tramp)
{
	//aquí sí interesa saber el modo de trabajo, servicio y estado de trampillas para saber el tiempo de molido
	TickType_t tiempo_moliendo = calcular_tiempo_moliendo_inf(modo, serv, tramp);

	// Encender Motor AC
	if (tiempo_moliendo != 0) _motor_ac_on_();

	mensaje_pantalla_molido_inf(modo,serv,tramp,&tiempo_moliendo);
	vTaskDelay(tiempo_moliendo);

	// Apagar Motor AC
	_motor_ac_off_();

	// Esperamos que la tarea Lcd este libre, podría estar acabando la animacion
	// habría que asegurarse de que hay lcd si se metiera un display diferente:
	// if(lcd()) xEventGroup... o sustituir EV_LCD por EV_FINANIMACION
	xEventGroupWaitBits(task_events, EV_LCD, pdTRUE, pdFALSE, portMAX_DELAY);

	//vTaskDelay(TIEMPO_DESCARGA); //?no estaba en casi ninguno
}

void molido_superior(TIPO_SERVICIO_e serv)
{
	TEXT_ID_e text;
	TickType_t tiempo_moliendo = (TickType_t)(leer_word_eeprom(DIRECC_DOSIS_2C) - leer_word_eeprom(DIRECC_DOSIS_1C));

	//doble premolido
	// Cerrar motor superior y esperar a que termine
	accion_motor(MOTOR_SUPERIOR, ACCION_CERRAR);

	// Encender Motor AC si el tiempo restante es superior a 0
	if (tiempo_moliendo != 0)
	{
		_motor_ac_on_();
	}

	if (serv == SIMPLE)
	{
		text = eTXT_C1_PREM;
	}
	else
	{
		text = eTXT_C2_PREM;
	}
	escribir_ln_id_llenarbox(text, LINEA_1, tiempo_moliendo);
	// Esperar "dosis_2c - dosis_1c" milisegundos
	vTaskDelay(tiempo_moliendo);

	// Apagar Motor AC
	_motor_ac_off_();

	vTaskDelay(TIEMPO_VISUALIZACION); //Para que de tiempo a ver el valor molido
	// Esperamos que la tarea  Oled este libre, podría estar acabando la animacion
	// habría que asegurarse de que hay lcd si se metiera un display diferente:
	// if(lcd()) xEventGroup... o sustituir EV_LCD por EV_FINANIMACION
	xEventGroupWaitBits(task_events, EV_LCD, pdTRUE, pdFALSE, portMAX_DELAY);
}

void tareas_finales(MODO_TRABAJO_e modo, TIPO_SERVICIO_e serv, ESTADO_TRAMPILLAS_e tramp)
{
	if (tramp == NO_CAFE)
		incrementar_contadores(serv);
	if (modo == PREMOLIDO)
	{
		//todo: el mensaje en ondemand se muestra en vaciado de inferior. quizá se pueda reordenar
		//y traer a tareas finales para unificar el flujo
		escribir_ln_id_box(eTXT_PREG, serv);
	}
	else if (serv == DOBLE)
	{
		if (tramp != NO_CAFE) grabar_word_eeprom(DIRECC_ESTADO_TRAMPILLA, NO_CAFE);
	}
	else if (serv == SIMPLE)
	{
		if (tramp == T1CAFE) grabar_word_eeprom(DIRECC_ESTADO_TRAMPILLA, NO_CAFE);
		else if (tramp == T1T2CAFE)
		{
			grabar_word_eeprom(DIRECC_ESTADO_TRAMPILLA, T1CAFE);
			escribir_ln_id_box(eTXT_CHAN2, serv);
		}
	}

	// Fin servicio, reset de teclado, flag fin secuencia
	if ((xEventGroupGetBits(task_events) & EV_INTENSIVA_ON) == 0 ) //si estamos en prueba intensiva no resetea teclado
		xQueueReset(handle_queue_teclado);						   //  para detectar cuando se quiere parar la prueba
	xEventGroupSetBits(task_events, EV_FIN_SERV);
}
