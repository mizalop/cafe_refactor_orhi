/*
 * avisoFresas.h
 *
 *  Created on: 19 ene. 2021
 *      Author: mikel
 */

#ifndef AVISOFRESAS_H_
#define AVISOFRESAS_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

/*******************************************************************************
 * Definiciones
 ******************************************************************************/
#define NCAFES_MAX	250000
#define NCAFES_MIN	 50000
#define NCAFES_DEFAULT   0

/*******************************************************************************
 * Funciones
 ******************************************************************************/
/**
 * @brief Función que chequea si hay que mostrar mensaje de cambio de fresas
 *
 * @param
 *
 * @return pdTRUE si se hay que mostrar mensaje, si no pdFALSE
 */

uint8_t check_aviso(void);

/**
 * @brief Función que decrementa los cafés correspondientes del contador del
 * 					aviso de cambio de fresas
 *
 * @param ucCant = nº de cafés molidos
 *
 * @return
 *
 *
 * NOTA: se le llama desde la función incrementar_contadores(...) de task_cafe
 * 			 	cada vez que se muela un café y se sume al contador general se
 * 			 	decrementará del contador del aviso.
 */

void decr_cont(uint8_t ucCant);


/*INICIALIZACIONES*/
/* @brief Función que chequea si los datos guardados en eeprom son congruentes
 *
 * @param
 *
 * @return NO si hay algún dato mal, SI si está bien
 */
uint8_t aviso_ok(void);

/* @brief Función que resetea los datos guardados en eeprom
 *
 * @param
 *
 * @return
 */
void borrar_aviso(void);


/*DEPURACIÓN*/
/*void depu(void);
void progra(void);*/

#endif /* AVISOFRESAS_H_ */
