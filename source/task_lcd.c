/**
 * \file task_lcd.c
 * \brief Funciones para controlar la pantalla LCD
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

#include <stdio.h>
#include <string.h>
#include "new_types.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "AG5_define.h"
#include "lcd.h"
#include "text_ids.h"
#include "rtos_time.h"
#include "task_lcd.h"
#include "main.h"


#define LCD_QUEUE_MAX_ITEMS  2                         // Maximum number of of items on Motor queue
#define LCD_QUEUE_ITEM_SIZE  sizeof (DISPLAY_MSG_t)    // Motor queue item size in bytes


/**
 * @brief Estructura de mensaje para LCD.
 */
typedef struct
{
	ACCION_LCD_e     accion;     /**< Identificador de tipo de acción */
	LINEA_LCD_e      linea;      /**< Línea en la que se queire escribir */
	char            *pTexto;     /**< Puntero del texto */
	PARPADEO_LCD_e   parpadeo;   /**< Parpadeo activado/descativado */
	TickType_t		 tiempo;	 /**< Tiempo dibujando animación box */
} DISPLAY_MSG_t;


static xQueueHandle handle_queue_display = 0;
static DISPLAY_MSG_t display_msg;

static char **  pText_Table;            // text table for the active language


char * Get_Text_Pointer(TEXT_ID_e text_id)
{
	return (text_id ? pText_Table [text_id-1] : "");
}


/**
 * @brief Función que envía mensaje de inicialización LCD a la tarea lcd_task
 */
BaseType_t lcd_init(void)
{
	while (handle_queue_display == 0)
	{
		// Crear cola de mensajes LCD
		handle_queue_display = xQueueCreate(LCD_QUEUE_MAX_ITEMS, LCD_QUEUE_ITEM_SIZE);
		vTaskDelay(TIME_10MSEC);
	}

	display_msg.accion = INICIAR;
	return xQueueSendToBack(handle_queue_display, &display_msg, 0);
}


/**
 * @brief Función que selecciona un idioma
 */
void idioma_lcd(IDIOMA_e idioma)
{
	uint8_t i;

	// count the number of languages
	for (i=0; languages_table[i]; i++) ;
	if (idioma >= i) idioma = 0;

	pText_Table =  (char **) languages_table[idioma];
}


/**
 * @brief Función que envía mensaje de escritura de línea de texto por identificador a la tarea lcd_task
 */
BaseType_t escribir_ln_id_lcd(TEXT_ID_e text_id, LINEA_LCD_e linea, PARPADEO_LCD_e parpadeo)
{
	display_msg.pTexto   = Get_Text_Pointer(text_id);
	display_msg.accion   = ESCRIBIR_LINEA;
	display_msg.linea    = linea;
	display_msg.parpadeo = parpadeo;

	return xQueueSendToBack(handle_queue_display, &display_msg, 0);
}


/**
 * @brief Función que envía mensaje de escritura de texto por identificador a la tarea lcd_task
 */
BaseType_t escribir_id_lcd(TEXT_ID_e text_id, PARPADEO_LCD_e parpadeo)
{
	display_msg.pTexto   = Get_Text_Pointer(text_id);
	display_msg.accion   = ESCRIBIR;
	display_msg.linea    = 1;
	display_msg.parpadeo = parpadeo;

	return xQueueSendToBack(handle_queue_display, &display_msg, 0);
}


/**
 * @brief Función que envía mensaje de escritura de línea de texto libre a la tarea lcd_task
 */
BaseType_t escribir_ln_str_lcd(char *pTexto, LINEA_LCD_e linea, PARPADEO_LCD_e parpadeo)
{
	display_msg.pTexto   = pTexto;
	display_msg.accion   = ESCRIBIR_LINEA;
	display_msg.linea    = linea;
	display_msg.parpadeo = parpadeo;

	return xQueueSendToBack(handle_queue_display, &display_msg, 0);
}


/**
 * @brief Función que envía mensaje de escritura de línea de texto por identificador con información de servicio a la tarea lcd_task
 */
BaseType_t escribir_ln_id_serv_lcd(TEXT_ID_e text_id, LINEA_LCD_e linea, PARPADEO_LCD_e parpadeo, SERV_e tipo_servicio)
{
	char str[8];

	display_msg.pTexto   = Get_Text_Pointer(text_id);
	display_msg.accion   = ESCRIBIR_LINEA;
	display_msg.linea    = linea;
	display_msg.parpadeo = parpadeo;

	strcpy(str, " ");
	if ((tipo_servicio == SERV_AUTOMATICO_1) || (tipo_servicio == SERV_AUTOMATICO_2))
	{
		lcd_LoadSoftChar(lcd_SOFTCHAR_MAG_ARROW, (uint8_t*) (lcd_SoftCharMAG));
		str[0] = lcd_SOFTCHAR_MAG_ARROW;
	}
	else if ((tipo_servicio == SERV_PROV) || (tipo_servicio == SERV_SIMPLE_PROV) || (tipo_servicio == SERV_DOBLE_PROV))
	{
		lcd_LoadSoftChar(lcd_SOFTCHAR_MAG_ARROW, (uint8_t*)(lcd_SoftCharARROW));
		str[0] = lcd_SOFTCHAR_MAG_ARROW;
	}
	strcat(str, display_msg.pTexto);
	display_msg.pTexto = str;

	while(handle_queue_display == 0)
	{
		vTaskDelay(TIME_10MSEC);
	}
	return xQueueSendToBack(handle_queue_display, &display_msg, 0);
}

/**
 * @brief Función que envía mensaje de escritura de línea de texto con "dibujo" de caja vacia/llena a la tarea lcd_task
 */
BaseType_t escribir_ln_id_box_lcd(TEXT_ID_e text_id, SERV_e tipo_servicio)
{
	char_t str[9] = "        ";

	configASSERT(text_id == eTXT_CHAN1 || text_id == eTXT_CHAN2 \
			|| text_id == eTXT_PREG || text_id == eTXT_ONDE || text_id == eTXT_EMPTY_BOX);

	display_msg.linea    = LINEA_1;
	display_msg.parpadeo = PARPADEO_NO;
	display_msg.accion   = STATIC_BOX;
	display_msg.tiempo   = 0;

	// LINEA DE ARRIBA: texto "C1:" o "C2:" o "C3:" y caja vacia o llena
	if ((tipo_servicio == SERV_DOBLE_PROV) || (tipo_servicio == SERV_AUTOMATICO_2) || (tipo_servicio == SERV_DOBLE))
	{
		strcpy(str, " C2     "); //Justo 8 caracteres para que el noveno sea un fin de cadena
	}
	/*else if (tipo_servicio == SERV_TRIPLE) //Por si se implementa la dosis triple
	{
		strcpy(str, " C3     ");
	}*/
	else
	{
		strcpy(str, " C1     ");
	}
	// Simbolo de "wifi" o flecha cuando procede
	if ((tipo_servicio == SERV_AUTOMATICO_1) || (tipo_servicio == SERV_AUTOMATICO_2))
	{
		lcd_LoadSoftChar(lcd_SOFTCHAR_MAG_ARROW, (uint8_t*) (lcd_SoftCharMAG));
		str[0] = lcd_SOFTCHAR_MAG_ARROW;
	}
	else if ((tipo_servicio == SERV_PROV) || (tipo_servicio == SERV_SIMPLE_PROV) || (tipo_servicio == SERV_DOBLE_PROV))
	{
		lcd_LoadSoftChar(lcd_SOFTCHAR_MAG_ARROW, (uint8_t*) (lcd_SoftCharARROW));
		str[0] = lcd_SOFTCHAR_MAG_ARROW;
	}

	// Dibujamos la caja
	if ( (text_id == eTXT_ONDE) || (text_id == eTXT_CHAN2)  || (text_id == eTXT_EMPTY_BOX) )
	{
		// Cargamos los tres caracteres con el valor de la caja vacia
		lcd_LoadSoftChar(lcd_SOFTCHAR_LEFT_UP_BOX, (uint8_t*) (lcd_SoftCharLEFTBOXUP));
		lcd_LoadSoftChar(lcd_SOFTCHAR_CENTER_UP_BOX, (uint8_t*)(lcd_SoftCharCENTERBOXUP));
		lcd_LoadSoftChar(lcd_SOFTCHAR_RIGHT_UP_BOX, (uint8_t*) (lcd_SoftCharRIGHTBOXUP));
		str[5] = lcd_SOFTCHAR_LEFT_UP_BOX;
		str[6] = lcd_SOFTCHAR_CENTER_UP_BOX;
		str[7] = lcd_SOFTCHAR_RIGHT_UP_BOX;
	}
	else if (text_id == eTXT_CHAN1)
	{
		// Cargamos los tres caracteres con el valor de la caja llena "especial"
		lcd_LoadSoftChar(lcd_SOFTCHAR_LEFT_UP_BOX, (uint8_t*) (lcd_SoftCharLEFTBOXUP + 56));
		lcd_LoadSoftChar(lcd_SOFTCHAR_CENTER_UP_BOX, (uint8_t*) lcd_SoftCharARROW);
		lcd_LoadSoftChar(lcd_SOFTCHAR_RIGHT_UP_BOX, (uint8_t*) (lcd_SoftCharRIGHTBOXUP + 56));
		str[5] = lcd_SOFTCHAR_LEFT_UP_BOX;
		str[6] = lcd_SOFTCHAR_CENTER_UP_BOX;
		str[7] = lcd_SOFTCHAR_RIGHT_UP_BOX;
	}
	else // Premolido
	{
		// Cargamos los tres caracteres con el valor de la caja llena
		lcd_LoadSoftChar(lcd_SOFTCHAR_LEFT_UP_BOX, (uint8_t*) (lcd_SoftCharLEFTBOXUP + 56));
		lcd_LoadSoftChar(lcd_SOFTCHAR_CENTER_UP_BOX, (uint8_t*) (lcd_SoftCharCENTERBOXUP + 56));
		lcd_LoadSoftChar(lcd_SOFTCHAR_RIGHT_UP_BOX, (uint8_t*) (lcd_SoftCharRIGHTBOXUP + 56));
		str[5] = lcd_SOFTCHAR_LEFT_UP_BOX;
		str[6] = lcd_SOFTCHAR_CENTER_UP_BOX; // Negro
		str[7] = lcd_SOFTCHAR_RIGHT_UP_BOX;
	}

	display_msg.pTexto   = str;

	// Ponemos en el buzon el mensaje para que oled_task lo visualice
	xQueueSendToBack(handle_queue_display, &display_msg, 0);

	// Esperamos que la tarea  Oled este libre
	xEventGroupWaitBits(task_events, EV_LCD, pdTRUE, pdFALSE, portMAX_DELAY);

	// LINEA DE ABAJO: texto "PREM" o "ONDE" y caja vacia o llena
	display_msg.linea    = LINEA_2;
	display_msg.parpadeo = PARPADEO_NO;
	display_msg.accion   = STATIC_BOX;
	display_msg.tiempo   = 0;

	strcpy(str, Get_Text_Pointer(text_id)); //Texto

	/*	if (text_id == eTXT_MICR)
		{
			// Cargamos los tres caracteres con el valor de la caja vacia y microdosis
			lcd_LoadSoftChar(lcd_SOFTCHAR_LEFT_DOWN_BOX, (uint8_t*) (lcd_SoftCharLEFTBOXDOWN));
			lcd_LoadSoftChar(lcd_SOFTCHAR_CENTER_DOWN_BOX, (uint8_t*) (lcd_SoftCharMICRODOSE));
			lcd_LoadSoftChar(lcd_SOFTCHAR_RIGHT_DOWN_BOX, (uint8_t*) (lcd_SoftCharRIGHTBOXDOWN));
			display_msg.parpadeo = PARPADEO_7;
			str[5] = lcd_SOFTCHAR_LEFT_DOWN_BOX;
			str[6] = lcd_SOFTCHAR_CENTER_DOWN_BOX;
			str[7] = lcd_SOFTCHAR_RIGHT_DOWN_BOX;
		}
		else*/ /*por si se implementa microdosis*/
	if ( (text_id == eTXT_ONDE) || (text_id == eTXT_EMPTY_BOX)  )
	{
		// Cargamos los tres caracteres con el valor de la caja vacia
		lcd_LoadSoftChar(lcd_SOFTCHAR_LEFT_DOWN_BOX, (uint8_t*) (lcd_SoftCharLEFTBOXDOWN));
		lcd_LoadSoftChar(lcd_SOFTCHAR_CENTER_DOWN_BOX, (uint8_t*) (lcd_SoftCharCENTERBOXDOWN));
		lcd_LoadSoftChar(lcd_SOFTCHAR_RIGHT_DOWN_BOX, (uint8_t*) (lcd_SoftCharRIGHTBOXDOWN));
		str[5] = lcd_SOFTCHAR_LEFT_DOWN_BOX;
		str[6] = lcd_SOFTCHAR_CENTER_DOWN_BOX;
		str[7] = lcd_SOFTCHAR_RIGHT_DOWN_BOX;
	}
	else if (text_id == eTXT_PREG)
	{
		// Cargamos los tres caracteres con el valor de la caja llena
		lcd_LoadSoftChar(lcd_SOFTCHAR_LEFT_DOWN_BOX, (uint8_t*) (lcd_SoftCharLEFTBOXDOWN + 56));
		lcd_LoadSoftChar(lcd_SOFTCHAR_CENTER_DOWN_BOX, (uint8_t*) (lcd_SoftCharCENTERBOXDOWN + 56));
		lcd_LoadSoftChar(lcd_SOFTCHAR_RIGHT_DOWN_BOX, (uint8_t*) (lcd_SoftCharRIGHTBOXDOWN + 56));
		str[5] = lcd_SOFTCHAR_LEFT_DOWN_BOX; // Negro
		str[6] = lcd_SOFTCHAR_CENTER_DOWN_BOX;
		str[7] = lcd_SOFTCHAR_RIGHT_DOWN_BOX;
	}
	else if ( (text_id == eTXT_CHAN1) || (text_id == eTXT_CHAN2) )
	{
		// Cargamos los tres caracteres con el valor de la caja media
		lcd_LoadSoftChar(lcd_SOFTCHAR_LEFT_DOWN_BOX, (uint8_t*) (lcd_SoftCharLEFTBOXDOWN + 40));
		lcd_LoadSoftChar(lcd_SOFTCHAR_CENTER_DOWN_BOX, (uint8_t*) lcd_SoftCharARROW);
		lcd_LoadSoftChar(lcd_SOFTCHAR_RIGHT_DOWN_BOX, (uint8_t*) (lcd_SoftCharRIGHTBOXDOWN + 40));
		str[5] =  lcd_SOFTCHAR_LEFT_DOWN_BOX;
		str[6] =  lcd_SOFTCHAR_CENTER_DOWN_BOX;
		str[7] =  lcd_SOFTCHAR_RIGHT_DOWN_BOX;
	}
	else // Cualquier otra cosa no prevista
	{
		str[5] =  0x20; // Blanco
		str[6] =  0x21; // !!!!!!!!!!!!!!!!
		str[7] =  0x20; // Blanco
	}

	display_msg.pTexto   = str;

	// Ponemos en el buzon el mensaje para que oled_task lo visualice
	 xQueueSendToBack(handle_queue_display, &display_msg, 0);

	// Esperamos que la tarea  Oled este libre
	xEventGroupWaitBits(task_events, EV_LCD, pdTRUE, pdFALSE, portMAX_DELAY);

	return 0;
}


/**
 * @brief Función que envía mensaje de escritura de línea de texto con "animacion" de caja llenandose a la tarea lcd_task
 */
BaseType_t escribir_ln_id_llenarbox_lcd(TEXT_ID_e text_id, LINEA_LCD_e linea, uint32_t tiempo_moler)
{
	display_msg.linea    = linea;
	display_msg.parpadeo = PARPADEO_NO;
	display_msg.accion   = FILLING_BOX;
	display_msg.tiempo   = tiempo_moler;
	display_msg.pTexto   = Get_Text_Pointer(text_id);

	return xQueueSendToBack(handle_queue_display, &display_msg, 0);
}

/**
 * @brief Función que envía mensaje de escritura de línea de texto con contador de tiempo a tarea lcd_task
 */
BaseType_t escribir_ln_id_timer_lcd(TEXT_ID_e text_id, LINEA_LCD_e linea, uint32_t tiempo_moler)
{
	display_msg.linea    = linea;
	display_msg.parpadeo = PARPADEO_NO;
	display_msg.accion   = TIMER;
	display_msg.tiempo   = tiempo_moler;
	display_msg.pTexto   = Get_Text_Pointer(text_id);

	return xQueueSendToBack(handle_queue_display, &display_msg, 0);
}


/**
 * @brief Función que envía mensaje de escritura de número en línea a la tarea lcd_task
 */
BaseType_t escribir_ln_num_lcd(uint32_t numero, LINEA_LCD_e linea, PARPADEO_LCD_e parpadeo)
{
	char str[8];

	sprintf(str, "%lu", numero);

	display_msg.pTexto   = str;
	display_msg.accion   = ESCRIBIR_LINEA;
	display_msg.linea    = linea;
	display_msg.parpadeo = parpadeo;

	return xQueueSendToBack(handle_queue_display, &display_msg, 0);
}


/**
 * @brief Función que envía mensaje para limpiar pantalla a la tarea lcd_task
 */
BaseType_t limpiar_lcd(void)
{
	display_msg.accion   = LIMPIAR;

	return xQueueSendToBack(handle_queue_display, &display_msg, 0);
}


/**
 * @brief Función que envía mensaje para limpiar línea a la tarea lcd_task
 */
BaseType_t limpiar_ln_lcd(LINEA_LCD_e linea)
{
	display_msg.pTexto   = "";
	display_msg.accion   = ESCRIBIR_LINEA;
	display_msg.linea    = linea;
	display_msg.parpadeo = PARPADEO_NO;

	return xQueueSendToBack(handle_queue_display, &display_msg, 0);
}

/**
 * @brief Tarea del autómata de visualización de pantalla LCD.
 */
void lcd_task(void *pvParameters)
{
	uint32_t tiempo1, tiempo_escalon, tiempo_fila2 = 0, tiempo_fin_animacion;
	char_t i;
	char_t str1[8], str2[8];

	for(;;)
	{
		// Leer mensajes en la cola (esperar indefinidamente)
		if (xQueueReceive(handle_queue_display, &display_msg, portMAX_DELAY) == pdPASS)
		{
			switch (display_msg.accion)
			{
			case INICIAR:
				taskENTER_CRITICAL();
				lcd_Init();
				lcd_Clear();
				lcd_Home();
				taskEXIT_CRITICAL();
				break;

			case ESCRIBIR_LINEA:
				lcd_WriteLineStr(display_msg.linea, display_msg.pTexto);
				if (display_msg.parpadeo > PARPADEO_NO)
				{
					lcd_GotoXY(display_msg.linea, display_msg.parpadeo);
					lcd_BlinkingOn();
				}
				break;

			case ESCRIBIR:
				lcd_WriteFullStr(display_msg.pTexto);
				break;

			case STATIC_BOX:
				lcd_WriteLineStr(display_msg.linea, display_msg.pTexto);
				if (display_msg.parpadeo > PARPADEO_NO)
				{
					lcd_GotoXY(display_msg.linea, display_msg.parpadeo);
					lcd_BlinkingOn();
				}
				xEventGroupSetBits(task_events, EV_LCD); //Flag fin de tarea
				break;

			case FILLING_BOX:
				strcpy(str2, "00.00   ");
				strcpy(str1, display_msg.pTexto);

				// Linea de arriba con "caja"
				lcd_LoadSoftChar(lcd_SOFTCHAR_LEFT_UP_BOX, (uint8_t*) (lcd_SoftCharLEFTBOXUP));
				lcd_LoadSoftChar(lcd_SOFTCHAR_CENTER_UP_BOX, (uint8_t*) (lcd_SoftCharCENTERBOXUP));
				lcd_LoadSoftChar(lcd_SOFTCHAR_RIGHT_UP_BOX, (uint8_t*) (lcd_SoftCharRIGHTBOXUP));
				str1[5] = lcd_SOFTCHAR_LEFT_UP_BOX;
				str1[6] = lcd_SOFTCHAR_CENTER_UP_BOX;
				str1[7] = lcd_SOFTCHAR_RIGHT_UP_BOX;

				// Primero calculamos los tiempos de refresco de cada linea
				if (display_msg.linea == LINEA_1)
				{
					tiempo1 = tiempo_fila2; // Inicio a la mitad de la grafica
					i = 8;
					tiempo_escalon = display_msg.tiempo /8;
					tiempo_fin_animacion = display_msg.tiempo + tiempo_fila2;
				}
				else if (display_msg.linea == LINEA_2)
				{
					tiempo1 = 0;
					i = 0;
					tiempo_fila2 = 0;
					tiempo_escalon = display_msg.tiempo /7;
					tiempo_fin_animacion = display_msg.tiempo;
				}
				else if (display_msg.linea == LINEA_1_2)
				{
					tiempo1 = 0;
					i = 0;
					tiempo_fila2 = 0;
					tiempo_escalon = display_msg.tiempo /15;
					tiempo_fin_animacion = display_msg.tiempo;
				}

				// Ojo: ahora nos paramos en este bucle sin volver por el bucle for(;;)
				while (tiempo1 <= tiempo_fin_animacion)
				{
					// Texto con el tiempo en números en linea 2
					if (tiempo1 > 10000) //Dos digitos antes del "." y dos despues
					{
						str2[0] = 0x30 + (tiempo1/10000); // 0 + valor
						str2[1] = 0x30 + ((tiempo1/1000)%10); // 0 + valor
						str2[2] = 0x2E; // Caracter "."
						str2[3] = 0x30 + (tiempo1%1000)/100;
						str2[4] = 0x20; // Blanco
						// Segundo decimal se escribe al final para no mover centesimas de segundo
						// cambia de posicion según la parte entera tiene 1 o 2 digitos
						if (tiempo1 >= (tiempo_fin_animacion - 50)) str2[4] = 0x30 + (tiempo1%100)/10;
					}
					else // Un digito antes del "." y dos despues
					{
						str2[0] = 0x30 + (tiempo1/1000); // 0 + valor
						str2[1] = 0x2E; // Caracter "."
						str2[2] = 0x30 + (tiempo1%1000)/100;
						str2[3] = 0x20; // Blanco
						str2[4] = 0x20; // Blanco
						if (tiempo1 >= (tiempo_fin_animacion - 50)) str2[3] = 0x30 + (tiempo1%100)/10;
					}
					// Animacion por escalones
					// Es un lio. Cuando es solo la linea de arriba (linea_1) sumamos el tiempo almacenado
					// de la anterior vez, en la que se hizo la linea de abajo (linea_2) y empezamos en i=8
					if (display_msg.linea == LINEA_2)
					{
						if (tiempo1 >= (tiempo_escalon * i))
						{
							// Cargamos los tres caracteres con el valor de la "caja" mas o menos llena
							lcd_LoadSoftChar(lcd_SOFTCHAR_LEFT_DOWN_BOX, (uint8_t*) (lcd_SoftCharLEFTBOXDOWN + 8*i));
							lcd_LoadSoftChar(lcd_SOFTCHAR_CENTER_DOWN_BOX, (uint8_t*) (lcd_SoftCharCENTERBOXDOWN + 8*i));
							lcd_LoadSoftChar(lcd_SOFTCHAR_RIGHT_DOWN_BOX, (uint8_t*) (lcd_SoftCharRIGHTBOXDOWN + 8*i));
							str2[5] = lcd_SOFTCHAR_LEFT_DOWN_BOX;
							str2[6] = lcd_SOFTCHAR_CENTER_DOWN_BOX;
							str2[7] = lcd_SOFTCHAR_RIGHT_DOWN_BOX;
							if (i < 8) i++;
						}
					}
					if (display_msg.linea == LINEA_1)  // Linea 1: arriba
					{
						if (tiempo1 >= (tiempo_escalon * (i-8) + tiempo_fila2))
						{
							//while (tiempo1 > (tiempo_escalon*(i-8) + tiempo_fila2 + 50) ) i++; // Si los escalones son mas de 50ms saltamos algunos
							// Linea de abajo toda llena
							lcd_LoadSoftChar(lcd_SOFTCHAR_LEFT_DOWN_BOX, (uint8_t*) (lcd_SoftCharLEFTBOXDOWN + 56));
							lcd_LoadSoftChar(lcd_SOFTCHAR_CENTER_DOWN_BOX, (uint8_t*) (lcd_SoftCharCENTERBOXDOWN + 56));
							lcd_LoadSoftChar(lcd_SOFTCHAR_RIGHT_DOWN_BOX, (uint8_t*) (lcd_SoftCharRIGHTBOXDOWN + 56));
							str2[5] = lcd_SOFTCHAR_LEFT_DOWN_BOX;// Linea de abajo toda llena
							str2[6] = lcd_SOFTCHAR_CENTER_DOWN_BOX;
							str2[7] = lcd_SOFTCHAR_RIGHT_DOWN_BOX;
							// Linea 1: arriba. Cargamos los tres caracteres con el valor de la "caja" mas o menos llena
							lcd_LoadSoftChar(lcd_SOFTCHAR_LEFT_UP_BOX, (uint8_t*) (lcd_SoftCharLEFTBOXUP + 8*(i-8)));
							lcd_LoadSoftChar(lcd_SOFTCHAR_CENTER_UP_BOX, (uint8_t*) (lcd_SoftCharCENTERBOXUP + 8*(i-8)));
							lcd_LoadSoftChar(lcd_SOFTCHAR_RIGHT_UP_BOX, (uint8_t*) (lcd_SoftCharRIGHTBOXUP + 8*(i-8)));
							str1[5] = lcd_SOFTCHAR_LEFT_UP_BOX;
							str1[6] = lcd_SOFTCHAR_CENTER_UP_BOX;
							str1[7] = lcd_SOFTCHAR_RIGHT_UP_BOX;
							if (i < 15) i++;
						}
					}
					if (display_msg.linea == LINEA_1_2)
					{
						if (tiempo1 >= (tiempo_escalon * i))
						{
							if (i < 8)   // Linea 2: abajo
							{
								// Cargamos los tres caracteres con el valor de la "caja" mas o menos llena
								lcd_LoadSoftChar(lcd_SOFTCHAR_LEFT_DOWN_BOX, (uint8_t*) (lcd_SoftCharLEFTBOXDOWN + 8*i));
								lcd_LoadSoftChar(lcd_SOFTCHAR_CENTER_DOWN_BOX, (uint8_t*) (lcd_SoftCharCENTERBOXDOWN + 8*i));
								lcd_LoadSoftChar(lcd_SOFTCHAR_RIGHT_DOWN_BOX, (uint8_t*) (lcd_SoftCharRIGHTBOXDOWN + 8*i));
								str2[5] = lcd_SOFTCHAR_LEFT_DOWN_BOX;
								str2[6] = lcd_SOFTCHAR_CENTER_DOWN_BOX;
								str2[7] = lcd_SOFTCHAR_RIGHT_DOWN_BOX;
							}
							else // Linea 1: arriba
							{
								//while (tiempo1 > (tiempo_escalon*(i-8) + tiempo_fila2 + 50) ) i++; // Si los escalones son mas de 50ms saltamos algunos
								// Linea de abajo toda llena
								lcd_LoadSoftChar(lcd_SOFTCHAR_LEFT_DOWN_BOX, (uint8_t*) (lcd_SoftCharLEFTBOXDOWN + 56));
								lcd_LoadSoftChar(lcd_SOFTCHAR_CENTER_DOWN_BOX, (uint8_t*) (lcd_SoftCharCENTERBOXDOWN + 56));
								lcd_LoadSoftChar(lcd_SOFTCHAR_RIGHT_DOWN_BOX, (uint8_t*) (lcd_SoftCharRIGHTBOXDOWN + 56));
								str2[5] = lcd_SOFTCHAR_LEFT_DOWN_BOX;// Linea de abajo toda llena
								str2[6] = lcd_SOFTCHAR_CENTER_DOWN_BOX;
								str2[7] = lcd_SOFTCHAR_RIGHT_DOWN_BOX;
								// Linea 1: arriba. Cargamos los tres caracteres con el valor de la "caja" mas o menos llena
								lcd_LoadSoftChar(lcd_SOFTCHAR_LEFT_UP_BOX, (uint8_t*) (lcd_SoftCharLEFTBOXUP + 8*(i-8)));
								lcd_LoadSoftChar(lcd_SOFTCHAR_CENTER_UP_BOX, (uint8_t*) (lcd_SoftCharCENTERBOXUP + 8*(i-8)));
								lcd_LoadSoftChar(lcd_SOFTCHAR_RIGHT_UP_BOX, (uint8_t*) (lcd_SoftCharRIGHTBOXUP + 8*(i-8)));
								str1[5] = lcd_SOFTCHAR_LEFT_UP_BOX;
								str1[6] = lcd_SOFTCHAR_CENTER_UP_BOX;
								str1[7] = lcd_SOFTCHAR_RIGHT_UP_BOX;
							}
							if (i < 15) i++;
						}   //tiempo
					}
					lcd_WriteLineStr(LINEA_1, str1);
					lcd_WriteLineStr(LINEA_2, str2);

					vTaskDelay(TIME_50MSEC);
					tiempo1 += TIME_50MSEC;  // Hacemos un bucle cada 50 ms

					xEventGroupSetBits(wdt_events, EV_WDT_MAIN); // Para no disparar el watchdog, aunque no dura tanto tiempo
				}  // while (tiempo1 <tiempo_fin_animacion)
				tiempo_fila2 = display_msg.tiempo; // Para la siguiente vez

				xEventGroupSetBits(task_events, EV_LCD); // Flag de fin de tarea
				break;

			case TIMER:
				strcpy(str2, "  00.00 ");
				strcpy(str1, "  ");
				strcat(str1, display_msg.pTexto);
				//strcpy(str1, display_msg.pTexto);

				// Primero calculamos los tiempos de refresco de cada linea
				if (display_msg.linea == LINEA_1)
				{
					tiempo1 = tiempo_fila2; // Inicio a la mitad de la grafica
					tiempo_fin_animacion = display_msg.tiempo + tiempo_fila2;
				}
				else if (display_msg.linea == LINEA_2)
				{
					tiempo1 = 0;
					tiempo_fila2 = 0;
					tiempo_fin_animacion = display_msg.tiempo;
				}
				else if (display_msg.linea == LINEA_1_2)
				{
					tiempo1 = 0;
					tiempo_fila2 = 0;
					tiempo_fin_animacion = display_msg.tiempo;
				}
				// Ojo: ahora nos paramos en este bucle sin volver por el bucle for(;;)
				while (tiempo1 <= tiempo_fin_animacion)
				{
					// Texto con el tiempo en números en linea 2
					if (tiempo1 > 10000) //Dos digitos antes del "." y dos despues
					{
						str2[2] = 0x30 + (tiempo1/10000); // 0 + valor
						str2[3] = 0x30 + ((tiempo1/1000)%10); // 0 + valor
						str2[4] = 0x2E; // Caracter "."
						str2[5] = 0x30 + (tiempo1%1000)/100;
						str2[6] = 0x20; // Blanco
						// Segundo decimal se escribe al final para no mover centesimas de segundo
						// cambia de posicion según la parte entera tiene 1 o 2 digitos
						if (tiempo1 >= (tiempo_fin_animacion - 50)) str2[6] = 0x30 + (tiempo1%100)/10;
					}
					else // Un digito antes del "." y dos despues
					{
						str2[2] = 0x30 + (tiempo1/1000); // 0 + valor
						str2[3] = 0x2E; // Caracter "."
						str2[4] = 0x30 + (tiempo1%1000)/100;
						str2[5] = 0x20; // Blanco
						str2[6] = 0x20; // Blanco
						if (tiempo1 >= (tiempo_fin_animacion - 50)) str2[5] = 0x30 + (tiempo1%100)/10;
					}

					lcd_WriteLineStr(LINEA_1, str1);
					lcd_WriteLineStr(LINEA_2, str2);

					vTaskDelay(TIME_50MSEC);
					tiempo1 += TIME_50MSEC;  // Hacemos un bucle cada 50 ms

					xEventGroupSetBits(wdt_events, EV_WDT_MAIN); // Para no disparar el watchdog, aunque no dura tanto tiempo
				}  // while (tiempo1 <tiempo_fin_animacion)
				tiempo_fila2 = display_msg.tiempo; // Para la siguiente vez

			    xEventGroupSetBits(task_events, EV_LCD); // Flag de fin de tarea
				break;

			case LIMPIAR:
				lcd_Clear();
				break;

			default:
				break;
			}
		}
	}
}
