/**
 * \file pwm.c
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

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_gpio.h"
#include "pin_mux.h"
#include "peripherals.h"
#include "power.h"

/*******************************************************************************
 * Definiciones
 ******************************************************************************/

/* Grupo del canal ADC */
#define ADC_POWER_CHANNEL_GROUP	0U

/* Valor mínimo y máximo permitidos para la entrada analógica de 32VDC.
 * El ADC es de 12 bits 0-5V (valor máximo 4095).
 * El cálculo para obtener el valor ADC respecto al valor de entrada de alimentación
 * teniendo en cuenta el divisor de tensión es: adc=Vs*3900/(27000+3900)*(4095/5)
 * */
#define ADC_32VDC_MIN			3000	// 29V
#define ADC_32VDC_MAX			3600	// 35V

/*******************************************************************************
 * Funciones
 ******************************************************************************/

uint8_t power_ok(void)
{
	uint32_t adc_value;

	// Llamar a la función "ADC12_SetChannelConfig()" para ejecutar una lectura del ADC.
	ADC12_SetChannelConfig(ADC_POWER_PERIPHERAL, ADC_POWER_CHANNEL_GROUP, ADC_POWER_channelsConfig);
	while (0U == (kADC12_ChannelConversionCompletedFlag &
				  ADC12_GetChannelStatusFlags(ADC_POWER_PERIPHERAL, ADC_POWER_CHANNEL_GROUP)))
	{
	}
	adc_value = ADC12_GetChannelConversionValue(ADC_POWER_PERIPHERAL, ADC_POWER_CHANNEL_GROUP);

	if ((adc_value > ADC_32VDC_MIN) && (adc_value < ADC_32VDC_MAX) && (GPIO_PinRead(PGOOD_DC5V_GPIO, PGOOD_DC5V_PIN) == 1))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
