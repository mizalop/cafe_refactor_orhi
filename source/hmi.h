/**
 * \file hmi.h
 * \brief Funciones de abstracción para control de HMI Gama Baja y Gama Alta
 *
 * \version 1.0
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

#ifndef SOURCE_HMI_H_
#define SOURCE_HMI_H_


/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "text_ids.h"
#include "task_lcd.h"


/*******************************************************************************
 * Funciones
 ******************************************************************************/
/*
 * INICIALIZACIÓN Y CONFIGURACIÓN
 */

#define idioma_config(x1)					idioma_lcd(x1)

/*
 * ESCRITURA
 */
/* Se redirigen las llamadas de escritura a las de lcd directamente.
 * Se hace así para mantener la estructura de programa.
 * Si en un futuro, por ejemplo, se introdujera
 * un display diferente, habría que definir la función hmi y que ahí se hiciera la llamada
 * a la función correspondiente de lcd o del nuevo display.
 * */
#define escribir_ln_id_serv(x1,x2,x3,x4)    escribir_ln_id_serv_lcd(x1,x2,x3,x4)
#define escribir_ln_id(x1,x2,x3)		    escribir_ln_id_lcd(x1,x2,x3)
#define escribir_id(x1,x2)				    escribir_id_lcd(x1,x2)
#define escribir_ln_str(x1,x2,x3)		    escribir_ln_str_lcd(x1,x2,x3)
#define escribir_ln_id_box(x1,x2)		    escribir_ln_id_box_lcd(x1,x2)
#define escribir_ln_id_llenarbox(x1,x2,x3)  escribir_ln_id_llenarbox_lcd(x1,x2,x3)
#define escribir_ln_id_timer(x1,x2,x3)		escribir_ln_id_timer_lcd(x1,x2,x3)
#define escribir_ln_num(x1,x2,x3)			escribir_ln_num_lcd(x1,x2,x3)

/*
 * LIMPIEZA
 */

/**
 * @brief Función para limpiar pantalla.
 */
#define limpiar_pantalla()					limpiar_lcd()
#define limpiar_ln(x1)						limpiar_ln_lcd(x1)


#endif /* SOURCE_HMI_H_ */
