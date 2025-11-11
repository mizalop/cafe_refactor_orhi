/**
 * \file porta.h
 * \brief Módulo de funciones relativas al porta-café.
 *
 * \version Markibar_Control_1_0
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

#ifndef SOURCE_PORTA_H_
#define SOURCE_PORTA_H_


/*******************************************************************************
 * Includes
 ******************************************************************************/

#include <stdint.h>


/*******************************************************************************
 * Funciones
 ******************************************************************************/

/**
 * @brief Función para detectar microruptor del porta.
 *
 * @return 1 si se ha pulsado micro, 0 en caso contrario.
 */
uint8_t micro_porta_pulsado(void);


/**
 * @brief Función para encender sensor magnético.
 */
void sensor_mag_on(void);   // Conecto alimentación al sensor


/**
 * @brief Función para apagar sensor magnético.
 */
void sensor_mag_off(void);  // Quito alimentación al sensor


/**
 * @brief Función para detectar imán.
 *
 * @return 0 si se ha detectado imán, 1 en caso contrario.
 */
uint8_t mag_detectado(void);


#endif /* SOURCE_PORTA_H_ */
