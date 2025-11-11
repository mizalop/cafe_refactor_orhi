/**
 * \file pwm.h
 * \brief Módulo de funciones relativas al estado de la fuente de alimentación.
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

#ifndef POWER_H_
#define POWER_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "fsl_adc12.h"

/*******************************************************************************
 * Definiciones
 ******************************************************************************/

/*******************************************************************************
 * Funciones
 ******************************************************************************/

/**
 * @brief Función para comprobar que la alimentación es correcta.
 *
 * @return 1 si el resultado es correcto, 0 en caso contrario.
 */
uint8_t power_ok(void);

#endif /* POWER_H_ */
