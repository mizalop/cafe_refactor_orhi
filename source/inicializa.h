/*
 * inicializa.h
 *
 *  Created on: 24 oct. 2022
 *      Author: mikel
 */

#ifndef INICIALIZA_H_
#define INICIALIZA_H_


/**
 * @brief Función para inicializar/comprobar la configuración del molino.
 */
void inicializar_config(void);

void inicializar_idioma(void);

uint8_t config_ok(void);

void borrar_config(void);


#endif /* INICIALIZA_H_ */
