/**
 * \file eeprom.h
 * \brief Funciones de inicialización, escritura y lectura de EEPROM.
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

#ifndef SOURCE_EEPROM_H_
#define SOURCE_EEPROM_H_


/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "fsl_flash.h"


/*******************************************************************************
 * Definiciones
 ******************************************************************************/

// N° caracteres DEVNAME
#define POSICIONES_DEVNAME    8
// N° caracteres PASSWORD
#define POSICIONES_PASSWORD   4


// DIRECCIONES EN LA EEPROM ENTRE 1 Y 128
// ¡¡¡ SON VALORES DE 2 BYTES !!!
enum { // Grabamos y leemos solo en las posiciones pares de memoria
	DIRECC_DOSIS_1C           = 0x00,                            // Datos de dosis: dos bytes para cada dosis
	DIRECC_DOSIS_2C           = DIRECC_DOSIS_1C          + 0x02,
	DIRECC_MODO_TRABAJO       = DIRECC_DOSIS_2C          + 0x02,
	DIRECC_MODO_TRABAJO_PREV  = DIRECC_MODO_TRABAJO      + 0x02,
	DIRECC_ESTADO_TRAMPILLA   = DIRECC_MODO_TRABAJO_PREV + 0x02,
	DIRECC_MODO_IDENTIF       = DIRECC_ESTADO_TRAMPILLA  + 0x02,
	DIRECC_COD_CLIENTE        = DIRECC_MODO_IDENTIF      + 0x02,
	DIRECC_PASS_ACT           = DIRECC_COD_CLIENTE       + 0x02,
	DIRECC_PASSWORD_U         = DIRECC_PASS_ACT          + 0x02, // 4 bytes
	DIRECC_PASSWORD_SU        = DIRECC_PASSWORD_U        + POSICIONES_PASSWORD, // 4 bytes
	DIRECC_TAG_C1C2           = DIRECC_PASSWORD_SU       + POSICIONES_PASSWORD,
	DIRECC_IDIOMA             = DIRECC_TAG_C1C2          + 0x02,
	DIRECC_BACKLIGHT          = DIRECC_IDIOMA            + 0x02,
	DIRECC_AVFR_ACTIVA		  = DIRECC_BACKLIGHT		 + 0x02,

	// Contabilidades: 4 bytes para cada contador.
	DIRECC_CONTADOR_1         = DIRECC_AVFR_ACTIVA       + 0x02, // Posición del primer byte del contador
	DIRECC_CONTADOR_2         = DIRECC_CONTADOR_1        + 0x04, // 4 bytes para cada contador
	DIRECC_CONTADOR_T1        = DIRECC_CONTADOR_2        + 0x04,
	DIRECC_CONTADOR_T2        = DIRECC_CONTADOR_T1       + 0x04,
	// Byte para controlar la integridad de los contadores totales
	DIRECC_CONTROL_C_T        = DIRECC_CONTADOR_T2       + 0x04,
	// Contador descendente para el aviso de fresas
	DIRECC_AVFR_NCAFES		  = DIRECC_CONTROL_C_T		 + 0x02 + 0x02, //2 bytes + 2 bytes para alinear memoria

	// Posicion para el n° de serie.
	// Ocupamos 8 bytes
	DIRECC_DEVICENAME         = DIRECC_AVFR_NCAFES       + 0x04, //4 bytes para contador descendente

	DIRECC_TEMPERATURA_VACIA  = DIRECC_DEVICENAME        + POSICIONES_DEVNAME, //no tiene ventilador
	DIRECC_FASTCHANGE_ON	  = DIRECC_TEMPERATURA_VACIA + 0x02,
	DIRECC_INICIALIZADO		  = DIRECC_FASTCHANGE_ON	 + 0x02  //2 bytes para true/false
};


/*******************************************************************************
 * Funciones
 ******************************************************************************/

/**
 * @brief Función para inicializar la EEPROM.
 */
status_t inicializar_eeprom(void);


/**
 * @brief Función para grabar un Word en la EEPROM.
 *
 * @param[in] posicion
 *            Dirección del dato.
 * @param[in] data
 *            Dato.
 * @return    Resultado de la operación.
 */
status_t grabar_word_eeprom(uint8_t posicion, uint16_t data);


/**
 * @brief Función para grabar un Integer en la EEPROM.
 *
 * @param[in] posicion
 *            Dirección del dato.
 * @param[in] data
 *            Dato.
 * @return    Resultado de la operación.
 */
status_t grabar_int_eeprom(uint8_t posicion, uint32_t data);


/**
 * @brief Función para leer un Word desde la EEPROM.
 *
 * @param[in] posicion
 *            Dirección del dato.
 * @return    Dato leído.
 */
uint16_t leer_word_eeprom(uint8_t posicion);


/**
 * @brief Función para leer un Integer desde la EEPROM.
 *
 * @param[in] posicion
 *            Dirección del dato.
 * @return    Dato leído.
 */
uint32_t leer_int_eeprom(uint8_t posicion);


#endif /* SOURCE_EEPROM_H_ */
