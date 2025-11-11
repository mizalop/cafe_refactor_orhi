/**
 * \file task_lcd.h
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

#ifndef SOURCE_TASK_LCD_H_
#define SOURCE_TASK_LCD_H_

#include "FreeRTOS.h"

#include "lcd.h"
#include "text_ids.h"


/**
 * @brief Enum de acciones para LCD.
 */
typedef enum
{
	INICIAR = 0,      /**< Inicializar LCD */
	ESCRIBIR_LINEA,   /**< Escritura en una línea */
	ESCRIBIR,         /**< Escritura general */
	STATIC_BOX,	  /**< Caja vacía o llena parada */
	FILLING_BOX,	  /**< Animación caja */
	TIMER,			  /**< Contador tiempo moliendo */
	LIMPIAR           /**< Limpiar LCD */
} ACCION_LCD_e;


/**
 * @brief Enum de líneas del LCD.
 */
typedef enum {
	LINEA_1 = 1,      /**< Línea 1 */
	LINEA_2,          /**< Línea 2 */
	LINEA_1_2		  /**< Animación dos lineas */
} LINEA_LCD_e;


/**
 * @brief Tipos de parpadeo del LCD.
 */
typedef enum {
	PARPADEO_NO = 0,  /**< Sin parpadeo */
	PARPADEO_1,       /**< Parpadeo en caracter 1 */
	PARPADEO_2,       /**< Parpadeo en caracter 2 */
	PARPADEO_3,       /**< Parpadeo en caracter 3 */
	PARPADEO_4,       /**< Parpadeo en caracter 4 */
	PARPADEO_5,       /**< Parpadeo en caracter 5 */
	PARPADEO_6,       /**< Parpadeo en caracter 6 */
	PARPADEO_7,       /**< Parpadeo en caracter 7 */
	PARPADEO_8,       /**< Parpadeo en caracter 8 */
	PARPADEO_TODO     /**< Parpadeo de línea completa */
} PARPADEO_LCD_e;


/**
 * @brief Función que envía mensaje de inicialización LCD a la tarea lcd_task
 *
 * @return pdTRUE si se ha ejecutado correctamente, si no errQUEUE_FULL.
 */
BaseType_t lcd_init(void);


/**
 * @brief Función que selecciona un idioma
 *
 * @param idioma [in]
 *           Identificador del idioma.
 */
void idioma_lcd(IDIOMA_e idioma);


/**
 * @brief Función que envía mensaje de escritura de línea de texto por identificador a la tarea lcd_task
 *
 * @param text_id [in]
 *           Identificador del texto.
 * @param linea [in]
 *           Línea del LCD.
 * @param parpadeo [in]
 *           Habilitación del parpadeo.
 *
 * @return pdTRUE si se ha ejecutado correctamente, si no errQUEUE_FULL.
 */
BaseType_t escribir_ln_id_lcd(TEXT_ID_e text_id, LINEA_LCD_e linea, PARPADEO_LCD_e parpadeo);


/**
 * @brief Función que envía mensaje de escritura de texto por identificador a la tarea lcd_task
 *
 * @param text_id [in]
 *           Identificador del texto.
 * @param parpadeo [in]
 *           Habilitación del parpadeo.
 *
 * @return pdTRUE si se ha ejecutado correctamente, si no errQUEUE_FULL.
 */
BaseType_t escribir_id_lcd(TEXT_ID_e text_id, PARPADEO_LCD_e parpadeo);


/**
 * @brief Función que envía mensaje de escritura de línea de texto libre a la tarea lcd_task
 *
 * @param *pTexto [in]
 *           Puntero del texto.
 * @param linea [in]
 *           Línea del LCD.
 * @param parpadeo [in]
 *           Habilitación del parpadeo.
 *
 * @return pdTRUE si se ha ejecutado correctamente, si no errQUEUE_FULL.
 */
BaseType_t escribir_ln_str_lcd(char *pTexto, LINEA_LCD_e linea, PARPADEO_LCD_e parpadeo);


/**
 * @brief Función que envía mensaje de escritura de línea de texto por identificador con información de servicio a la tarea lcd_task
 *
 * @param text_id [in]
 *           Identificador del texto.
 * @param linea [in]
 *           Línea del LCD.
 * @param parpadeo [in]
 *           Habilitación del parpadeo.
 * @param tipo_servicio [in]
 *           Tipo de servicio de café.
 *
 * @return pdTRUE si se ha ejecutado correctamente, si no errQUEUE_FULL.
 */
BaseType_t escribir_ln_id_serv_lcd(TEXT_ID_e text_id, LINEA_LCD_e linea, PARPADEO_LCD_e parpadeo, SERV_e tipo_servicio);


/**
 * @brief Función que envía mensaje de escritura de línea de texto con "dibujo" de caja vacia/llena a la tarea lcd_task
 *
 * @param text_id [in]
 *           Identificador del texto.
 * @param tipo_servicio [in]
 *           Tipo de servicio de café.
 *
 * @return pdTRUE si se ha ejecutado correctamente, si no errQUEUE_FULL.
 */
BaseType_t escribir_ln_id_box_lcd(TEXT_ID_e text_id, SERV_e tipo_servicio);


/**
 * @brief Función que envía mensaje de escritura de línea de texto con "animacion" de caja llenandose a la tarea lcd_task
 *
 * @param text_id [in]
 *           Identificador del texto.
 * @param linea [in]
 *           Línea del OLED.
 * @param tiempo_moler [in]
 *           Tiempo que dura la animacion.
 *
 * @return pdTRUE si se ha ejecutado correctamente, si no errQUEUE_FULL.
 */
BaseType_t escribir_ln_id_llenarbox_lcd(TEXT_ID_e text_id, LINEA_LCD_e linea, uint32_t tiempo_moler);


/**
 * @brief Función que envía mensaje de escritura de línea de texto con contador de tiempo a tarea lcd_task
 *
 * @param text_id [in]
 *           Identificador del texto.
 * @param linea [in]
 *           Línea del OLED.
 * @param tiempo_moler [in]
 *           Tiempo que dura la animacion.
 *
 * @return pdTRUE si se ha ejecutado correctamente, si no errQUEUE_FULL.
 */
BaseType_t escribir_ln_id_timer_lcd(TEXT_ID_e text_id, LINEA_LCD_e linea, uint32_t tiempo_moler);


/**
 * @brief Función que envía mensaje de escritura de número en línea a la tarea lcd_task
 *
 * @param numero [in]
 *          Número.
 * @param linea [in]
 *           Línea del LCD.
 * @param parpadeo [in]
 *           Habilitación del parpadeo.
 *
 * @return pdTRUE si se ha ejecutado correctamente, si no errQUEUE_FULL.
 */
BaseType_t escribir_ln_num_lcd(uint32_t numero, LINEA_LCD_e linea, PARPADEO_LCD_e parpadeo);


/**
 * @brief Función que envía mensaje para limpiar pantalla a la tarea lcd_task
 *
 * @return pdTRUE si se ha ejecutado correctamente, si no errQUEUE_FULL.
 */
BaseType_t limpiar_lcd(void);


/**
 * @brief Función que envía mensaje para limpiar línea a la tarea lcd_task
 *
 * @param linea [in]
 *           Línea del LCD.
 *
 * @return pdTRUE si se ha ejecutado correctamente, si no errQUEUE_FULL.
 */
BaseType_t limpiar_ln_lcd(LINEA_LCD_e linea);


/**
 * @brief Tarea del autómata de visualización de pantalla LCD.
 */
void lcd_task(void *pvParameters);

#endif /* SOURCE_TASK_LCD_H_ */
