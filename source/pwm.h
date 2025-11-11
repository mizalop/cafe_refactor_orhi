/**
 * \file pwm.h
 * \brief Módulo de funciones relativas a los periféricos PWM.
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

#ifndef SOURCE_PWM_H_
#define SOURCE_PWM_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "fsl_ftm.h"

/*******************************************************************************
 * Definiciones
 ******************************************************************************/

#define PWM_M1_OPEN_FTM_CHANNEL       kFTM_Chnl_6
#define PWM_M2_OPEN_FTM_CHANNEL       kFTM_Chnl_2
#define PWM_COM_CLOSE_FTM_CHANNEL     kFTM_Chnl_4

/*******************************************************************************
 * Funciones
 ******************************************************************************/

/**
 * @brief Función para inicializar los canales PWM.
 */
void pwm_init(void);

/**
 * @brief Función para de-inicializar los canales PWM.
 */
void pwm_deinit(void);

/**
 * @brief Función para detener los canales PWM.
 */
void pwm_stop(void);

/**
 * @brief Función para establecer un duty-cycle en un canal PWM.
 *
 * @param[in] channel
 *            Canal PWM.
 * @param[in] duty_cycle
 *            Duty-Cycle.
 */
void pwm_set_duty(ftm_chnl_t channel, uint8_t duty_cycle);

#endif /* SOURCE_PWM_H_ */
