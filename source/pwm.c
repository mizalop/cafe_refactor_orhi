/**
 * \file pwm.c
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

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "peripherals.h"
#include "pwm.h"

/*******************************************************************************
 * Definiciones
 ******************************************************************************/

/* Dirección base del Flextimer */
#define PWM_FTM_BASEADDR            FTM0_PERIPHERAL

/* Clock para el driver FTM */
#define PWM_FTM_SOURCE_CLOCK        FTM0_CLOCK_SOURCE

/* Frecuencia del PWM en Hz*/
#define PWM_FREQ_HZ                 200U

ftm_config_t ftmInfo;
ftm_chnl_pwm_signal_param_t ftmParam[3];
ftm_pwm_level_select_t pwmLevel = kFTM_HighTrue;

/*******************************************************************************
 * Funciones
 ******************************************************************************/

void pwm_init(void)
{
	/* Configurar parámetros del FTM */

	// Motor 1 Open
	ftmParam[0].chnlNumber = PWM_M1_OPEN_FTM_CHANNEL;
	ftmParam[0].level = pwmLevel;
	ftmParam[0].dutyCyclePercent = 0U;
	ftmParam[0].firstEdgeDelayPercent = 0U;

	// Motor 2 Open
	ftmParam[1].chnlNumber = PWM_M2_OPEN_FTM_CHANNEL;
	ftmParam[1].level = pwmLevel;
	ftmParam[1].dutyCyclePercent = 0U;
	ftmParam[1].firstEdgeDelayPercent = 0U;

	// Motores Común Close
	ftmParam[2].chnlNumber = PWM_COM_CLOSE_FTM_CHANNEL;
	ftmParam[2].level = pwmLevel;
	ftmParam[2].dutyCyclePercent = 0U;
	ftmParam[2].firstEdgeDelayPercent = 0U;

	FTM_GetDefaultConfig(&ftmInfo);
	ftmInfo.prescale = kFTM_Prescale_Divide_8;

	/* Initialize FTM module */
	FTM_Init(PWM_FTM_BASEADDR, &ftmInfo);

	FTM_SetupPwm(PWM_FTM_BASEADDR, ftmParam, 3, kFTM_CenterAlignedPwm, PWM_FREQ_HZ, PWM_FTM_SOURCE_CLOCK);
}

void pwm_deinit(void)
{
	FTM_Deinit(PWM_FTM_BASEADDR);
}

void pwm_stop(void)
{
	FTM_StopTimer(PWM_FTM_BASEADDR);
}

void pwm_set_duty(ftm_chnl_t channel, uint8_t duty_cycle)
{
	/* Disable channel output before updating the dutycycle */
	FTM_UpdateChnlEdgeLevelSelect(PWM_FTM_BASEADDR, channel, 0U);

	/* Update PWM duty cycle */
	FTM_UpdatePwmDutycycle(PWM_FTM_BASEADDR, channel, kFTM_CenterAlignedPwm, duty_cycle);

	/* Software trigger to update registers */
	FTM_SetSoftwareTrigger(PWM_FTM_BASEADDR, true);

	/* Start channel output with updated dutycycle */
	FTM_UpdateChnlEdgeLevelSelect(PWM_FTM_BASEADDR, channel, pwmLevel);

	FTM_StartTimer(PWM_FTM_BASEADDR, kFTM_SystemClock);
}
