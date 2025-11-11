/**
 * \file backlight.h
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

#ifndef SOURCE_BACKLIGHT_H_
#define SOURCE_BACKLIGHT_H_


/*******************************************************************************
 * Funciones
 ******************************************************************************/
/**
 * @brief Función inline para inicializar pwm backlight
 */
void backlight_init(void);


/**
 * @brief Función para establecer en el backlight el nivel de brillo guardado en la EEPROM.
 */
void backlight_set_eeprom(void);


/**
 * @brief Función para establecer el nivel de brillo del backlight.
 *
 * @param[in] duty_cycle
 *            Nivel (0..100).
 */
void backlight_set_duty(uint8_t duty_cycle);

/**
 * @brief Función para apagar backlight.
 *
 * @param[in] void
 */
void backlight_off(void);

#endif /* SOURCE_BACKLIGHT_H_ */
