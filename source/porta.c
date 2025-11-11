/**
 * \file porta.c
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


/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "fsl_gpio.h"
#include "pin_mux.h"
#include "porta.h"


/*******************************************************************************
 * Variables Privadas
 ******************************************************************************/



/*******************************************************************************
 * Funciones
 ******************************************************************************/

uint8_t micro_porta_pulsado(void)
{
	return !GPIO_PinRead(PORTA1_GPIO, PORTA1_PIN);
}


void sensor_mag_on(void)
{
	GPIO_PinWrite(MAG_CTRL_GPIO, MAG_CTRL_PIN, 0);
}


void sensor_mag_off(void)
{
	GPIO_PinWrite(MAG_CTRL_GPIO, MAG_CTRL_PIN, 1);
}


uint8_t mag_detectado(void)
{
	return !(GPIO_PinRead(MAG_IN1_GPIO, MAG_IN1_PIN) && GPIO_PinRead(MAG_IN2_GPIO, MAG_IN2_PIN));
}
