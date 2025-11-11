/**
 * \file backlight.c
 * \brief Módulo de control de backlight.
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

#include "fsl_ftm.h"
#include "peripherals.h"

#include "eeprom.h"
#include "backlight.h"


/*******************************************************************************
 * Definiciones
 ******************************************************************************/

/* Dirección base del Flextimer */
#define BKL_PWM_FTM_BASEADDR     FTM1_PERIPHERAL
#define BKL_PWM_FTM_CHANNEL      kFTM_Chnl_0

#define BKL_PWM_FREQ 16000U

ftm_chnl_pwm_signal_param_t ftmParam1[1];
/*******************************************************************************
 * Funciones
 ******************************************************************************/
/**
 * @brief Función para inicializar pwm backlight
 */
void backlight_init(){
	ftmParam1[0].chnlNumber            = 0;
	ftmParam1[0].level                 = kFTM_HighTrue;
	ftmParam1[0].dutyCyclePercent      = 0;
	ftmParam1[0].firstEdgeDelayPercent = 0;

	FTM_Init(FTM1_PERIPHERAL,&FTM1_config);
	FTM_SetupPwm(FTM1_PERIPHERAL,ftmParam1,1,kFTM_CenterAlignedPwm,BKL_PWM_FREQ,FTM1_CLOCK_SOURCE);
}

void backlight_set_eeprom(void)
{
	uint16_t nivel = 0;

	// Cargar configuración del backlight
	nivel = leer_word_eeprom(DIRECC_BACKLIGHT);

	// Visualizar
	backlight_set_duty(nivel*10);
}

void backlight_set_duty(uint8_t duty_cycle)
{
	/* Disable channel output before updating the dutycycle */
	FTM_UpdateChnlEdgeLevelSelect(BKL_PWM_FTM_BASEADDR, BKL_PWM_FTM_CHANNEL, 0U);

	/* Update PWM duty cycle */
	FTM_UpdatePwmDutycycle(BKL_PWM_FTM_BASEADDR, BKL_PWM_FTM_CHANNEL, kFTM_CenterAlignedPwm, duty_cycle);

	/* Software trigger to update registers */
	FTM_SetSoftwareTrigger(BKL_PWM_FTM_BASEADDR, true);

	/* Start channel output with updated dutycycle */
	FTM_UpdateChnlEdgeLevelSelect(BKL_PWM_FTM_BASEADDR, BKL_PWM_FTM_CHANNEL, kFTM_HighTrue);

	FTM_StartTimer(BKL_PWM_FTM_BASEADDR, kFTM_SystemClock);
}

void backlight_off(void)
{
	backlight_set_duty(0);
}
