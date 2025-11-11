/*
 * avisoFresas.c
 *
 *  Created on: 19 ene. 2021
 *      Author: mikel
 */

#include "FreeRTOS.h"
#include "eeprom.h"
#include "AG5_define.h"

#include "avisoFresas.h"

/**
 * @brief Función que devuelve 1 si hay que mostrar mensaje de cambio de fresas y 0 si no.
 */
uint8_t check_aviso(void)
{
	if (leer_word_eeprom(DIRECC_AVFR_ACTIVA) == 1 && leer_int_eeprom(DIRECC_AVFR_NCAFES) <= 0) return pdTRUE;
	return pdFALSE;
}

/**
 * @brief Función que decrementa los cafés correspondientes del contador del
 * 					aviso de cambio de fresas
 */
void decr_cont(uint8_t ucCant)
{
	uint32_t ulNCafes = leer_int_eeprom(DIRECC_AVFR_NCAFES);

	if (ulNCafes >= ucCant) ulNCafes -= ucCant;
	else ulNCafes = 0;

	grabar_int_eeprom(DIRECC_AVFR_NCAFES, ulNCafes);
}

/*INICIALIZACIONES*/
/* @brief Función que chequea si los datos guardados en eeprom son congruentes
 */
uint8_t aviso_ok(void)
{
	if (leer_word_eeprom(DIRECC_AVFR_ACTIVA) != 0 && leer_word_eeprom(DIRECC_AVFR_ACTIVA) != 1) return NO;
	if (leer_int_eeprom(DIRECC_AVFR_NCAFES) > NCAFES_MAX || leer_int_eeprom(DIRECC_AVFR_NCAFES) < 0) return NO;

	return SI;
}

/* @brief Función que resetea los datos guardados en eeprom
 */
void borrar_aviso(void)
{
	grabar_word_eeprom(DIRECC_AVFR_ACTIVA, 0); //desactivar aviso
	grabar_int_eeprom(DIRECC_AVFR_NCAFES, NCAFES_DEFAULT); //poner valor por defecto (0)
}

/*DEPURACIÓN*/
/*Para programar valores de EEPROM durante depuración*/
/*void progra(void)
{
	uint32_t ncaf = 0;
	uint8_t  act  = 0;

	grabar_word_eeprom(DIRECC_AVFR_ACTIVA, act);
	grabar_int_eeprom(DIRECC_AVFR_NCAFES, ncaf);
}*/

/*Para ver valores de EEPROM durante depuración*/
/*void depu(void)
{
	uint32_t auxLong = leer_int_eeprom(DIRECC_AVFR_NCAFES);
	uint16_t auxChar = leer_word_eeprom(DIRECC_AVFR_ACTIVA);
}*/
