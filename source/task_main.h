/**
 * @file task_main.h
 *
 * @brief Módulo principal del control de molino automático.
 *
 *
 */

#ifndef SOURCE_TASK_MAIN_H_
#define SOURCE_TASK_MAIN_H_


#define TECLADO_QUEUE_MAX_ITEMS   1                                      // Número máximo de elementos
#define TECLADO_QUEUE_ITEM_SIZE   sizeof (TECLADO_MSG_t)                 // Tamaño del mensaje en bytes

// Estructura mensaje teclado
typedef struct
{
	uint8_t       tecla;
	uint8_t       pulsacion_larga;
} TECLADO_MSG_t;

typedef enum  // Estados del autómata principal
{
	AG_EST_INICIO = 0,
	AG_EST_SOFT_INI,
	AG_EST_COMPROBAR,
	AG_EST_ON,
	AG_EST_PULSADO_C1,
	AG_EST_PULSADO_C2,
	AG_EST_RST_MAG,
	AG_EST_FILTRA_REBOTE,
	AG_EST_ESPERA_C1,
	AG_EST_ESPERA_DOBLE,
	AG_EST_MOLER_1,
	AG_EST_MOLER_2,
	AG_EST_FIN_MOLIDO,
	AG_EST_MICRO_PULSADO,
	AG_EST_PULSADO_OFF,
	AG_EST_MENSAJE_OFF,
	AG_EST_OFF,
	AG_EST_MENSAJE_ON,
	AG_EST_PROGRAMACION,
	AG_EST_CONFIGURACION,
	AG_EST_ERROR_POWER,
	AG_EST_AVISOFRESAS
} AG_EST_e;

extern xQueueHandle  handle_queue_teclado;
//TECLADO_MSG_t teclado_msg;

void enviar_tecla(uint8_t tecla, uint8_t pulsacion_larga);

/*Funciones para obtener o cambiar valores de variables desde otras rutinas*/
uint8_t getVar(uint8_t var);

void setVar(uint8_t var, uint8_t val);

/*Funciones para saber si la configuración del molino es tipo A o tipo B*/
uint8_t check_typeA(void);
uint8_t check_typeB(void);

/**
 * @brief Tarea principal que se ejecuta inicialmente.
 *        Aquí está el autómata de estados principal.
 */
void main_task(void *pvParameters);

#endif /* SOURCE_TASK_MAIN_H_ */
