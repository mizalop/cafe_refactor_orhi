/**
 * \file task_comunica.h
 * \brief Módulo de comunicación con la nube
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

#ifndef SOURCE_TASK_COMUNICA_H_
#define SOURCE_TASK_COMUNICA_H_

/*******************************************************************************
 * Funciones
 ******************************************************************************/

void envia_sensores(void);

void envia_info(void);

void ini_comunica(void);

void parseo_mensaje(char *buffer, uint8_t size);

void comunicacion_task(void *pvParameters);

#endif /* SOURCE_TASK_COMUNICA_H_ */
