/**
 * \file main.c
 * \brief Inicialización del HW, del SO y de las tareas.
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


#ifndef SOURCE_MAIN_H_
#define SOURCE_MAIN_H_


/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "event_groups.h"


/*******************************************************************************
 * Definiciones
 ******************************************************************************/

#define PROGRAM_TYPE           'o'    // o - Orhi (propuesta de notación: i - izaga key, t - izaga tft?, mon - orhi monedero, w - izaga weight, c - izaga connect (ojo: ag6 también tendrá connect))
#define VERSION_MAJOR           255     // Max 255
#define VERSION_MINOR           255     // Max 255
//valor irreal para que se note que no es programa de producción

/* Task priorities. */
#define main_task_PRIORITY      (configMAX_PRIORITIES - 2)
#define teclado_task_PRIORITY   (configMAX_PRIORITIES - 1)
#define pesaje_task_PRIORITY    (configMAX_PRIORITIES - 3)
#define modbus_task_PRIORITY    (configMAX_PRIORITIES - 1)
#define cafe_task_PRIORITY      (configMAX_PRIORITIES - 3)
#define dosis_task_PRIORITY     (configMAX_PRIORITIES - 3)
#define config_task_PRIORITY    (configMAX_PRIORITIES - 3)
#define lcd_task_PRIORITY       (configMAX_PRIORITIES - 1)
#define com_task_PRIORITY       (configMAX_PRIORITIES - 3)
#define watchdog_task_PRIORITY  (configMAX_PRIORITIES - 4)
#define ventil_task_PRIORITY    (configMAX_PRIORITIES - 4)


/*******************************************************************************
 * Variables Globales
 ******************************************************************************/

extern EventGroupHandle_t task_events;
extern EventGroupHandle_t wdt_events;

#endif /* SOURCE_MAIN_H_ */
