/**
 * \file task_cafe.h
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

#ifndef SOURCE_TASK_CAFE_H_
#define SOURCE_TASK_CAFE_H_


/*******************************************************************************
 * Funciones
 ******************************************************************************/

void borrar_contadores(CONTADORES_e tipo_cont);
uint8_t cont_servicios_ok(void);
uint8_t cont_totales_ok(void);

uint8_t moler_dosificar(MODO_TRABAJO_e modo, TIPO_SERVICIO_e servicio);

void moler_dosificar_task(void *pvParameters);

#endif /* SOURCE_TASK_CAFE_H_ */
