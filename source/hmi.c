/**
 * \file hmi.c
 * \brief Funciones de abstracción para control de HMI Gama Baja y Gama Alta
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

/*
 * Al no tener versión TFT y LCD este archivo es casi un bypass para las funciones
 * relativas al display.
 *
 * En el caso del izaga, desde task_main, task_config_gb, etc. se llama a funciones
 * como por ejemplo "escribir_ln_id" y en hmi.c se redirecciona a la versión tft o
 * a escribir_ln_id_oled.
 *
 * En el caso del Orhi, no se redirecciona, simplemente se llama a la función
 * correspondiente. No obstante, se deja el archivo para tener la estructura
 * si alguna vez hiciera falta.
 *
 * Simplemente se eliminan las funciones como tal y se implementan como x_lcd
 * en hmi.h.
 *
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
/*#include "AG5_define.h"
#include "eeprom.h"
#include "lcd.h"
#include "teclado.h"
#include "task_lcd.h"
#include "task_dosis.h"
#include "task_cafe.h"
#include "text_ids.h"
#include "task_main.h"
#include "main.h"

#include "hmi.h"
*/

/*******************************************************************************
 * Funciones
 ******************************************************************************/
/*
 * INICIALIZACIÓN Y CONFIGURACIÓN
 */

/*
 *
 */

/*
 * ESCRITURA
 */
/* En hmi.h */

/*
 * LIMPIEZA
 */
/* En hmi.h */
