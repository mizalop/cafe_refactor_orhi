/**
 * \file watchdog.c
 * \brief Funciones Watchdog y Heartbit.
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

#ifndef SOURCE_WATCHDOG_H_
#define SOURCE_WATCHDOG_H_


/*******************************************************************************
 * Funciones
 ******************************************************************************/

/**
 * @brief Tarea de refresco de watchdog y heartbit.
 */
void watchdog_task(void *pvParameters);

#endif /* SOURCE_WATCHDOG_H_ */
