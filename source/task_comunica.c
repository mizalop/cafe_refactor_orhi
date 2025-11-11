/**
 * \file task_comunica.c
 * \brief Módulo de comunicación con la nube
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


/*******************************************************************************
 * Includes
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "board.h"
#include "pin_mux.h"
#include "fsl_gpio.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#include "main.h"
#include "rtos_time.h"
#include "eeprom.h"
#include "AG5_define.h"
#include "rs232_uart.h"
#include "task_comunica.h"


/*******************************************************************************
 * Definiciones
 ******************************************************************************/

#define COMTX_QUEUE_MAX_ITEMS  5                    // Número máximo de elementos en la cola
#define COMTX_QUEUE_ITEM_SIZE  sizeof (COMTX_MSG_t) // Tamaño del elemento

#define COMRX_QUEUE_MAX_ITEMS  5                    // Número máximo de elementos en la cola
#define COMRX_QUEUE_ITEM_SIZE  sizeof (COMRX_MSG_t) // Tamaño del elemento

#define LONG_PARAMETROS (LONG_BUFFER_ENVIO-10)

#define TIEMPO_ESPERA_RESPUESTA     TIME_10SEC  // Antes era 5s
#define TIEMPO_ENVIO_PERIODICO      TIME_1MIN 	// Antes era 25s

#define _reset_kunak_on_()    GPIO_PinWrite(KUNAK_RST_GPIO, KUNAK_RST_PIN, 1)
#define _reset_kunak_off_()   GPIO_PinWrite(KUNAK_RST_GPIO, KUNAK_RST_PIN, 0)

// Tipos de mensaje O COMADOS que podemos construir
typedef enum
{
	COM_EJECUCION = 0,
	COM_LECTURA,
	COM_ESCRITURA
} COM_SER_t;

// Estados de estado_comunicación
typedef enum
{
	E_COMUNICA_REPOSO = 0,
	E_COMUNICA_ENVIA_AT,
	E_COMUNICA_RECIBE_RESPUESTA_AT,
	E_COMUNICA_COMPRUEBA_DEVNAME_1,
	E_COMUNICA_COMPRUEBA_DEVNAME_2,
	E_COMUNICA_COMPRUEBA_SNAME_0, //5
	E_COMUNICA_COMPRUEBA_SNAME_1,
	E_COMUNICA_COMPRUEBA_SNAME_2,
	E_COMUNICA_COMPRUEBA_SUNIT_1,
	E_COMUNICA_COMPRUEBA_SUNIT_2,
	E_COMUNICA_INICIALIZA_DEVNAME_1,  //10
	E_COMUNICA_INICIALIZA_DEVNAME_2,
	E_COMUNICA_INCIALIZA_SNAME_0,
	E_COMUNICA_INCIALIZA_SNAME_1,
	E_COMUNICA_INCIALIZA_SUNIT_1,
	E_COMUNICA_ENVIA_INFO_0,  //15
	E_COMUNICA_ENVIA_INFO_1,
	E_COMUNICA_ENVIA_INFO_2,
	E_COMUNICA_ENVIA_SENSORES,
	E_COMUNICA_ENVIA_SENSORES_0,
	E_COMUNICA_ESPERA_ENVIOS,   //20
	E_COMUNICA_ENVIA_AT_PER,
	E_COMUNICA_RECIBE_RESPUESTA_AT_PER,
	E_COMUNICA_RECIBIDO_INFO,
	E_COMUNICA_FALLO,
	E_COMUNICA_REINTENTA,  //25
	E_COMUNICA_RECIBIDO_ERROR,
	E_COMUNICA_TRATA_ERROR

} COMUNICA_t;

// Tipos de parametros que vamos a enviar y recibir
typedef enum
{
	PAR_NULO_ = 0,
	PAR_INFO_VERS_FW = 10,
	PAR_INFO_MODELO,
	PAR_INFO_MODO,
	PAR_INFO_IDENTIF,
	PAR_INFO_BLOQUEO,
	PAR_INFO_BLANCO,  //NUM_COMANDOS_INFO
	PAR_SNAME_0 = 20,
	PAR_SNAME_1,
	PAR_SNAME_2,
	PAR_SNAME_3,
	PAR_SNAME_4,
	PAR_SNAME_5,    //NUM_SENSORES
	PAR_SUNIT_0 = 30,
	PAR_SUNIT_1,
	PAR_SUNIT_2,
	PAR_SUNIT_3,
	PAR_SUNIT_4,
	PAR_SUNIT_5,    //NUM_SENSORES
	PAR_DOSIS_C1 = 40,
	PAR_DOSIS_C2,
	PAR_SERV_C1,
	PAR_SERV_C2,
	PAR_UART_4,
	PAR_UART_5,     //NUM_SENSORES
	PAR_DEVICENAME = 100,
	NUMERO_PARAMETROS
} COM_PAR_t;

#define NUM_COMANDOS_INFO 6
#define NUM_SENSORES    6
#define _es_parametro_info(Par)   ((Par>=PAR_INFO_VERS_FW)&&(Par<PAR_SNAME_0))
#define _es_parametro_sname(Par)  ((Par>=PAR_SNAME_0)&&(Par<PAR_SUNIT_0))
#define _es_parametro_sunit(Par)  ((Par>=PAR_SUNIT_0)&&(Par<PAR_DOSIS_C1))
#define _es_parametro_sample(Par) ((Par>=PAR_DOSIS_C1)&&(Par<PAR_DEVICENAME))

/**
 * @brief Comandos AT que enviamos a Kunak
 */
typedef enum {
	M_AT_ = 0,
	M_AT_DEVNAME,
	M_AT_INFO,
	M_AT_SNAME,
	M_AT_SUNIT,
	M_AT_SAMPLE,
	TOTAL_MENSAJES_AT
} COMMANDS_t;

/**
 * @brief Comandos que recibimos de Kunak
 */
typedef enum {
	REC_NADA = 0,
	REC_OK,
	REC_OK_COMANDO,
	REC_INFO,
	REC_ERROR,
	REC_RESPUESTA
} REC_t;

/**
 * @brief Valores para la variable pendiente_envio_datos
 */
typedef enum {
	ENVIO_NADA = 0,
	ENVIO_DATOS_SENSORES,
	ENVIO_DATOS_INFO
} ENVIO_t;

/**
 * @brief Estructura de mensaje.
 */
typedef struct
{
	ENVIO_t       envio; /**< Tipo de envío que se quiere hacer */
} COMTX_MSG_t;

/**
 * @brief Estructura de mensaje de lectura.
 */
typedef struct
{
	REC_t             command;                /**< Comando */
	char              data[LONG_BUFFER_REC];  /**< dato */
	uint8_t           size;                   /**< tamaño del dato */
} COMRX_MSG_t;

// --- Constantes en area de código (Flash EPROM) ---
// Lista de nombres de comandos AT: tienen 8 caracteres + NULL
const unsigned char tabla_mensajes_AT[TOTAL_MENSAJES_AT][8] =
{"AT", "DEVNAME", "INFO", "SNAME", "SUNIT", "SAMPLE"};

// LONGITUD TEXTOS INFO =7  (6 + NULL)
// Lista de valores INFO que se pueden enviar a la nube
const unsigned char tabla_txt_info[6][7] =
{"FIRMWA", "MODELO", "MODO", "IDENTI", "BLOCK"," - "};
// Nombre de modelos según el codigo_cliente guardado en e2prom.
const unsigned char tabla_txt_modelo[NUM_MODELOS][7]=
{"STD", "PRUEBAS"};
// Nombre de modos TRABAJO según el modo_trabajo guardado en e2prom.
const unsigned char tabla_txt_modo[2][7] =
{"PREMOL", "INSTAN"};
// Opciones de identificación por orden:
//  ETIQUETA_NEGRA = 0, SENSOR_MAG, DOBLE_CLIK, TECLADO_C1, TECLADO_C2, TECLADO_LAST
const unsigned char tabla_txt_identif[NUM_OPCIONES_IDENTIF][7] =
{"Magnet", "Doble ", "Tec.c1", "Tec.c2", "Tec.UL" };
const unsigned char txt_bloqueo[]= {"Habilitado"};

// Cambiar la version de programa en main.h -->  VERSION_MAJOR, VERSION_MINOR
const char txt_version[]= {"     "};
const char txt_version_fw[]= {"Firmw "};
const unsigned char txt_fixed[] =   {"fixed"};
const unsigned char txt_editable[]= {"editable"};
const unsigned char txt_blanco[]= {"   "};

// Lista de valores de sensores que se pueden enviar a la nube
const unsigned char tabla_txt_sensor[NUM_SENSORES][7] =
{"DOSIS1", "DOSIS2", "C1", "C2", "UART_4", "UART_5"};
// Unidades de cada uno de los sensores enviados a la tarjeta de Kunak.
const unsigned char tabla_txt_unidades[NUM_SENSORES][9] =
{"SEGUNDOS", "SEGUNDOS", "Serv.", "Serv.", "Un", "Un"};


/*******************************************************************************
 * Variables Privadas
 ******************************************************************************/

static uint8_t estado_comunicacion = E_COMUNICA_REPOSO; // Reposo

static xQueueHandle handle_queue_comtx = 0;
static COMTX_MSG_t comtx_msg;

static xQueueHandle handle_queue_comrx = 0;
static COMRX_MSG_t comrx_msg;

static char buffer_a_enviar[LONG_BUFFER_ENVIO];
static char parametros[LONG_PARAMETROS];
static char buffer_size;


/*******************************************************************************
 * Funciones
 ******************************************************************************/

void construye_mensaje(unsigned char mensaje, unsigned char tipo_com)
{
	unsigned char i = 0;
	unsigned char j = 0;

	if (tipo_com == COM_EJECUCION) { //Envía un comando pelado
		i=0;
		while ( (tabla_mensajes_AT[mensaje][i] != 0 ) && (i<LONG_BUFFER_ENVIO)) {
			buffer_a_enviar[i] = tabla_mensajes_AT[mensaje][i];
			i++;
		}
	}
	else if (tipo_com == COM_LECTURA){ // AT+Comando?
		i=0;
		buffer_a_enviar[i] = 'A';
		i++;
		buffer_a_enviar[i] = 'T';
		i++;
		buffer_a_enviar[i] = '+';
		i++;
		j=0;
		while ( (tabla_mensajes_AT[mensaje][j] != 0 ) && (j<(LONG_BUFFER_ENVIO-6)) ) {
			buffer_a_enviar[i+j] = tabla_mensajes_AT[mensaje][j];
			j++;
		}
		i+=j;
		buffer_a_enviar[i] = '?';
		i++;
	}
	else if (tipo_com == COM_ESCRITURA){ // AT+Comando=
		i=0;
		buffer_a_enviar[i] = 'A';
		i++;
		buffer_a_enviar[i] = 'T';
		i++;
		buffer_a_enviar[i] = '+';
		i++;
		j=0;
		while ( (tabla_mensajes_AT[mensaje][j] != 0 ) && (i<(LONG_BUFFER_ENVIO-6)) ) {
			buffer_a_enviar[i+j] = tabla_mensajes_AT[mensaje][j];
			j++;
		}
		i+=j;
		buffer_a_enviar[i] = '=';
		i++;
		// Vuelvo a buscar el caracter NULL para cerrar el string de parametros.
		j = 0;
		while ( (parametros[j] != 0 ) && (j<LONG_PARAMETROS) ) {
			buffer_a_enviar[i+j] = parametros[j];
			j++;
		}
		i+=j;
	}
	buffer_a_enviar[i] = '\r';
	i++;
	buffer_a_enviar[i] = '\n';
	buffer_size = i + 1;
}


void prepara_parametros(unsigned char tipo_parametro)
{
	unsigned int dato_int;
	unsigned char i = 0;
	unsigned char j = 0;
	char fw_version[11];
	char version[11];
	uint16_t word;

	if (tipo_parametro == PAR_DEVICENAME)
	{
		for (i = 0; i < POSICIONES_DEVNAME; i+=2)
		{
			word = leer_word_eeprom(DIRECC_DEVICENAME + i);
			parametros[i] = word;
			parametros[i + 1] = word >> 8;
		}
	}

	// OJO: El tipo_parametro va de 10 en 10 y para el primer
	// dígito lo recorto a un valor de 0 a 9.
	//  PAR_INFO_   10 --> 19
	//  PAR_SNAME_  20 --> 29
	//  PAR_SUNIT_  30 --> 39
	//  PAR_SAMPLE_ 40 --> 49
	else if (_es_parametro_info(tipo_parametro)) { // Parametros INFO
		i=0;
		// Recortamos tipo_parametro
		parametros[i] = ('0' + (tipo_parametro % 10));
		i++;
		parametros[i] = ',';
		i++;
		j=0;
		while (tabla_txt_info[(tipo_parametro % 10)][j] != 0) {
			parametros[i+j]= tabla_txt_info[(tipo_parametro % 10)][j];
			j++;
		}
		i+=j;
		parametros[i]=',';
		i++;
		j=0;
		switch (tipo_parametro)
		{
		case PAR_INFO_VERS_FW:
			strcpy(fw_version, txt_version_fw);
			snprintf(version, sizeof(version), "%d.%d", VERSION_MAJOR, VERSION_MINOR);
			strcat(fw_version, version);
			// Pos. version firmware del molino
			while (fw_version[j] != 0) {
				parametros[i+j]= fw_version[j];
				j++;
			}
			break;
		case PAR_INFO_MODELO:
			//Pos. fija codigo cliente
			while (tabla_txt_modelo[leer_word_eeprom(DIRECC_COD_CLIENTE)][j] != 0) {
				parametros[i+j]= tabla_txt_modelo[leer_word_eeprom(DIRECC_COD_CLIENTE)][j];
				j++;
			}
			break;
		case PAR_INFO_MODO:
			//Pos. fija modo trabajo
			while (tabla_txt_modo[leer_word_eeprom(DIRECC_MODO_TRABAJO)][j] != 0) {
				parametros[i+j]= tabla_txt_modo[leer_word_eeprom(DIRECC_MODO_TRABAJO)][j];
				j++;
			}
			break;
		case PAR_INFO_IDENTIF:
			//Pos. fija modo identificación
			while (tabla_txt_identif[leer_word_eeprom(DIRECC_MODO_IDENTIF)][j] != 0) {
				parametros[i+j]= tabla_txt_identif[leer_word_eeprom(DIRECC_MODO_IDENTIF)][j];
				j++;
			}
			break;
		case PAR_INFO_BLOQUEO:
			//Pos. fija bloqueo
			while (txt_bloqueo[j] != 0) {
				parametros[i+j]= txt_bloqueo[j];
				j++;
			}
			break;
		default: // PAR_INFO_BLANCO:
			//Pos. fija bloqueo
			while (txt_blanco[j] != 0) {
				parametros[i+j]= txt_blanco[j];
				j++;
			}
			break;
		} // switch (tipo_parametro)
		i+=j;
		parametros[i]=',';
		i++;
		j=0;
		// Aqui Puedo poner TODOS los parametros "fixed" o "editable"
		// Para hacerlos de 1 en uno debería poner esto en el switch de arriba.
		while (txt_editable[j] != 0) {  //txt_editable <-> txt_fixed
			parametros[i+j]= txt_fixed[j];
			j++;
		}
		i+=j;
	}
	else if (_es_parametro_sname(tipo_parametro)) { // Parametros SNAME
		tipo_parametro = (tipo_parametro % 10); // Recortamos
		i=0;
		parametros[i] = ('0' + tipo_parametro);
		i++;
		parametros[i] = ',';
		i++;
		j=0;
		while (tabla_txt_sensor[tipo_parametro][j] != 0) {
			parametros[i+j]= tabla_txt_sensor[tipo_parametro][j];
			j++;
		}
		i+=j;
		parametros[i] = 0;
	}
	else if (_es_parametro_sunit(tipo_parametro)) { // Parametros SUNIT
		tipo_parametro = (tipo_parametro % 10); // Recortamos
		i=0;
		parametros[i] = ('0' + tipo_parametro);
		i++;
		parametros[i] = ',';
		i++;
		j=0;
		while (tabla_txt_unidades[tipo_parametro][j] != 0) {
			parametros[i+j]= tabla_txt_unidades[tipo_parametro][j];
			j++;
		}
		i+=j;
	}
	else if (_es_parametro_sample(tipo_parametro)) { // Parametros SAMPLE sensores
		i=0;
		parametros[i] = ('0' + (tipo_parametro % 10));
		i++;
		parametros[i] = ',';
		i++;
		if (tipo_parametro <= PAR_DOSIS_C2) {
			// Parametros PAR_DOSIS_C1 y C2
			// Datos dosis en posiciones fijas de e2prom
			if (tipo_parametro <= PAR_DOSIS_C1)
				dato_int = leer_word_eeprom(DIRECC_DOSIS_1C);
			else if (tipo_parametro <= PAR_DOSIS_C2)
				dato_int = leer_word_eeprom(DIRECC_DOSIS_2C);
			dato_int = dato_int / 10;
			parametros[i] = dato_int / 10000 + '0';
			i++;
			dato_int = dato_int % 10000;
			parametros[i] = dato_int / 1000 + '0';
			i++;
			dato_int = dato_int % 1000;
			parametros[i] = dato_int / 100 + '0';
			i++;
			// El dato de dosis está guardado en centésismas los dos
			// últimos dígitos van después del punto decimal.
			parametros[i] = '.';
			i++;
			dato_int = dato_int % 100;
			parametros[i] = dato_int / 10 + '0';
			i++;
			dato_int = dato_int % 10;
			parametros[i] = dato_int + '0';
		}
		else if (tipo_parametro <= PAR_SERV_C2){
			// Parametros n° servicios PAR_SERV_C1 y C2. Contadores totales.
			if (tipo_parametro == PAR_SERV_C1)
				dato_int = leer_int_eeprom(DIRECC_CONTADOR_T1);
			else  dato_int = leer_int_eeprom(DIRECC_CONTADOR_T2);
			sprintf(&parametros[i], "%d", dato_int);
			i = 8;
		}
		else { // Parametros de sensores no definidos UART_4 y UART_5
			j=0;
			while (txt_blanco[j] != 0) {
				parametros[i+j] = txt_blanco[j];
				j++;
			}
			i+=j;
		}
	} // Parametros SAMPLE
	// Importante: el valor NULL (cod. ascii 0) debe cerrar el string
	parametros[i] = 0;
}


unsigned char ok_mensaje_enviado(char* mensaje1, char* mensaje2)
{
	unsigned char i = 0;
	for (i=0;
			(  (*(mensaje1 + i) != '?')  // Comando lectura
					&& (*(mensaje1 + i) != '=')  // Comando escritura
					&& (*(mensaje1 + i) != ':')  // Respuesta a comando lectura
					&& (i < 12)  );              // Limite de longitud por seguridad
			i++) {
		if(*(mensaje1 + i) != *(mensaje2 + i)) return 0;
	}
	return 1; // Todos los caracteres coinciden
}


unsigned char ok_parametros_respuesta(char* mensaje1)
{
	unsigned char i = 0;
	unsigned char j = 0;
	// OJO: en una respuesta de tipo "OK:SNAME=0,..." hay dos
	// carateres de separación, ":" y "=" empiezo a comparar en la
	// posición 3 para no tropezarme el ":"
	i=3;
	while (  (*(mensaje1 + i) != '=')
			&& (*(mensaje1 + i) != ':')
			&& (i<12) ) {
		i++; //Localizamos el caracter '=' o el ":"
	}
	i++; // El siguiente es ya el dato
	for (j=0;( (*(mensaje1 + i + j) != '\r') && (j<LONG_PARAMETROS) );j++) {
		if(parametros[j] != *(mensaje1 + i + j)) return 0;
	}
	return 1;
}


unsigned char envia_comando(unsigned char parametros_sensor)
{
	COMRX_MSG_t     rx_msg;

	if (parametros_sensor != PAR_NULO_)
	{
		prepara_parametros(parametros_sensor);
		if (_es_parametro_info(parametros_sensor))
			construye_mensaje(M_AT_INFO, COM_ESCRITURA);
		else if (_es_parametro_sname(parametros_sensor))
			construye_mensaje(M_AT_SNAME, COM_ESCRITURA);
		else if (_es_parametro_sunit(parametros_sensor))
			construye_mensaje(M_AT_SUNIT, COM_ESCRITURA);
		else if (_es_parametro_sample(parametros_sensor))
			construye_mensaje(M_AT_SAMPLE, COM_ESCRITURA);

		envio_buffer_serie(buffer_a_enviar, buffer_size);

		if (xQueueReceive(handle_queue_comrx, &rx_msg, TIEMPO_ESPERA_RESPUESTA) == pdPASS)
		{
			if (rx_msg.command == REC_OK_COMANDO)
			{
				// Desplazo el puntero de buffer_a_enviar otros 3 (quito "AT+")
				if (ok_mensaje_enviado((rx_msg.data), (buffer_a_enviar + 3)) == 0)
				{
					//activa_fallo(FALLO_GENERICO);
					return 0;
				}
				else // Mensaje correcto: compruebo parametro dosis
				{
					// prepara_parametros() ya está hecho antes
					if (ok_parametros_respuesta(rx_msg.data) == 0) // Mal
					{
						//activa_fallo(FALLO_GENERICO);
						return 0;
					}
					return 1;
				}
			}
		}
	}
	return 0;
}


void ini_comunica(void)
{
	uint8_t i;
	uint8_t serie[POSICIONES_DEVNAME];
	uint8_t error = 1;
	uint16_t word;

	// Valor por defecto para DEVNAME "G5000000" --> no arranca comunic.
	// Valores programables validos 0-->9
	for (i = 0; i < POSICIONES_DEVNAME; i+=2)
	{
		word = leer_word_eeprom(DIRECC_DEVICENAME + i);
		serie[i] = word;
		serie[i + 1] = word >> 8;
	}

	for (i = 2; i < POSICIONES_DEVNAME; i++)
	{
		if ((serie[i] > '0') && (serie[i] <= '9'))
		{
			error = 0;
		}
	}

	if ((serie[0] != 'G') || (serie[1] != '5'))
	{
		error = 1;
	}

	// Devname distinto de "G5000000"
	if (error == 0)
	{
		estado_comunicacion = E_COMUNICA_ENVIA_AT;
	}
} // ini_comunica


void envia_sensores(void)
{
	comtx_msg.envio = ENVIO_DATOS_SENSORES;
	xQueueSendToBack(handle_queue_comtx, &comtx_msg, 0);
}


void envia_info(void)
{
	comtx_msg.envio = ENVIO_DATOS_INFO;
	xQueueSendToBack(handle_queue_comtx, &comtx_msg, 0);
}

/*  Tipos de mensajes recibidos:
REC_NADA = 0,
REC_OK,
REC_OK_COMANDO,
REC_INFO,
REC_ERROR,
REC_RESPUESTA
 */
void parseo_mensaje(char *buffer, uint8_t size)
{
	if (strncmp(buffer, "OK\r\n", 4) == 0)
	{
		comrx_msg.command = REC_OK;
		//comrx_msg.data = "";
		comrx_msg.size = 0;
		xQueueSendToBackFromISR(handle_queue_comrx, &comrx_msg, pdFALSE);
	}
	else if (strncmp(buffer, "OK:", 3) == 0)
	{
		comrx_msg.command = REC_OK_COMANDO;
		comrx_msg.size = size - 3;
		strncpy(comrx_msg.data, buffer + 3, comrx_msg.size);
		xQueueSendToBackFromISR(handle_queue_comrx, &comrx_msg, pdFALSE);
	}
	else if (strncmp(buffer, "ERR:", 4) == 0)
	{
		comrx_msg.command = REC_ERROR;
		comrx_msg.size = size - 4;
		strncpy(comrx_msg.data, buffer + 4, comrx_msg.size);
		xQueueSendToBackFromISR(handle_queue_comrx, &comrx_msg, pdFALSE);
	}
	else
	{
		comrx_msg.command = REC_RESPUESTA;
		comrx_msg.size = size;
		strncpy(comrx_msg.data, buffer, comrx_msg.size);
		xQueueSendToBackFromISR(handle_queue_comrx, &comrx_msg, pdFALSE);
	}
}


void comunicacion_task(void *pvParameters)
{
	static uint8_t repite_comando;
	static uint8_t inicializar_kunak = 0;
	static uint8_t resetear_kunak = 0;
	static TickType_t tiempo_envio;

	COMTX_MSG_t  tx_msg;
	COMRX_MSG_t  rx_msg;

	while (handle_queue_comtx == 0)
	{
		// Crear cola de mensajes de envío
		handle_queue_comtx = xQueueCreate(COMTX_QUEUE_MAX_ITEMS, COMTX_QUEUE_ITEM_SIZE);
		vTaskDelay(TIME_10MSEC);
	}
	while (handle_queue_comrx == 0)
	{
		// Crear cola de mensajes de recepción
		handle_queue_comrx = xQueueCreate(COMRX_QUEUE_MAX_ITEMS, COMRX_QUEUE_ITEM_SIZE);
		vTaskDelay(TIME_10MSEC);
	}

	for(;;)
	{
		switch (estado_comunicacion)
		{
		case E_COMUNICA_REPOSO: // Esperando
			break;

		case E_COMUNICA_ENVIA_AT:
			construye_mensaje(M_AT_, COM_EJECUCION);
			envio_buffer_serie(buffer_a_enviar, buffer_size);
			estado_comunicacion = E_COMUNICA_RECIBE_RESPUESTA_AT;
			break;

		case E_COMUNICA_RECIBE_RESPUESTA_AT:
			if (xQueueReceive(handle_queue_comrx, &rx_msg, TIEMPO_ESPERA_RESPUESTA) == pdPASS)
			{
				if (rx_msg.command == REC_OK)
				{
					construye_mensaje(M_AT_DEVNAME, COM_LECTURA);
					envio_buffer_serie(buffer_a_enviar, buffer_size);
					estado_comunicacion = E_COMUNICA_COMPRUEBA_DEVNAME_1;
				}
				else
				{
					estado_comunicacion = E_COMUNICA_FALLO;
				}
			}
			else
			{
				estado_comunicacion = E_COMUNICA_FALLO;
			}
			break;

		case E_COMUNICA_COMPRUEBA_DEVNAME_1:
			if (xQueueReceive(handle_queue_comrx, &rx_msg, TIEMPO_ESPERA_RESPUESTA) == pdPASS)
			{
				if (rx_msg.command == REC_RESPUESTA)
				{
					if (ok_mensaje_enviado(rx_msg.data, (buffer_a_enviar + 3)) == 0)
					{
						estado_comunicacion = E_COMUNICA_FALLO;
					}
					else // Mensaje correcto: compruebo parametro n° serie
					{
						prepara_parametros(PAR_DEVICENAME); // Para comparar resultado
						// Si los parametros de este mensaje (o sea el número de serie)
						// no conciden con los recibidos de Kunak hay que escribir el
						// n° serie y resto de datos nuevos.
						if (ok_parametros_respuesta(rx_msg.data) == 0)
						{
							inicializar_kunak = 1; // Flag para reescribir
						}
						estado_comunicacion = E_COMUNICA_COMPRUEBA_DEVNAME_2;
					}
				}
				else
				{
					estado_comunicacion = E_COMUNICA_FALLO;
				}
			}
			else
			{
				estado_comunicacion = E_COMUNICA_FALLO;
			}
			break;

		case E_COMUNICA_COMPRUEBA_DEVNAME_2:
			if (xQueueReceive(handle_queue_comrx, &rx_msg, TIEMPO_ESPERA_RESPUESTA) == pdPASS)
			{
				if (rx_msg.command == REC_OK_COMANDO)
				{
					// Desplazo el puntero de buffer_a_enviar otros 3 (quito "AT+")
					if (ok_mensaje_enviado((rx_msg.data), (buffer_a_enviar + 3)) == 0)
					{
						estado_comunicacion = E_COMUNICA_FALLO;
					}
					else
					{
						if (inicializar_kunak == 0)
						{
							estado_comunicacion = E_COMUNICA_COMPRUEBA_SNAME_0;
						}
						else
						{
							estado_comunicacion = E_COMUNICA_INICIALIZA_DEVNAME_1;
						}
					}
				}
				else
				{
					estado_comunicacion = E_COMUNICA_FALLO;
				}
			}
			else
			{
				estado_comunicacion = E_COMUNICA_FALLO;
			}
			break;

		case E_COMUNICA_COMPRUEBA_SNAME_0:
			inicializar_kunak = 0;
			repite_comando = 0;
			//Comado lectura: no necesita prepara_parametros()
			construye_mensaje(M_AT_SNAME, COM_LECTURA);
			envio_buffer_serie(buffer_a_enviar, buffer_size);
			estado_comunicacion = E_COMUNICA_COMPRUEBA_SNAME_1;
			break;

		case E_COMUNICA_COMPRUEBA_SNAME_1:
			// Espera a recibir la respuesta con los datos pedidos
			if (xQueueReceive(handle_queue_comrx, &rx_msg, TIEMPO_ESPERA_RESPUESTA) == pdPASS)
			{
				if (rx_msg.command == REC_RESPUESTA)
				{
					// Desplazo el de buffer_a_enviar +3 para quitar el "AT+"
					if (ok_mensaje_enviado(rx_msg.data, (buffer_a_enviar + 3)) == 0)
					{
						estado_comunicacion = E_COMUNICA_FALLO;
					}
					else // Mensaje correcto: compruebo parametro SNAME
					{
						prepara_parametros(PAR_SNAME_0 + repite_comando); //Para comparar
						// Si los parametros de este mensaje (o sea el nombre sensor) no
						// conciden con los recibidos de Kunak hay que escribirlos.
						if (ok_parametros_respuesta(rx_msg.data) == 0)
							inicializar_kunak = 1; // Flag para reescribir
						repite_comando++;
						if (repite_comando >= NUM_SENSORES)
							estado_comunicacion = E_COMUNICA_COMPRUEBA_SNAME_2;
					}
				}
				else
				{
					estado_comunicacion = E_COMUNICA_FALLO;
				}
			}
			else
			{
				estado_comunicacion = E_COMUNICA_FALLO;
			}
			break;

		case E_COMUNICA_COMPRUEBA_SNAME_2:
			// Espera al eco del comando lectura que devuelve Kunak
			if (xQueueReceive(handle_queue_comrx, &rx_msg, TIEMPO_ESPERA_RESPUESTA) == pdPASS)
			{
				if (rx_msg.command == REC_OK_COMANDO)
				{
					// Desplazo el puntero de buffer_a_enviar otros 3 (quito "AT+")
					if (ok_mensaje_enviado((rx_msg.data), (buffer_a_enviar + 3)) == 0)
						estado_comunicacion = E_COMUNICA_FALLO;
					else // Mensaje correcto: AHORA NO HAY PARAMETRO
					{
						if (inicializar_kunak == 0)
						{
							repite_comando = 0;
							//Comando lectura: no necesita prepara_parametros()
							construye_mensaje(M_AT_SUNIT, COM_LECTURA);
							envio_buffer_serie(buffer_a_enviar, buffer_size);
							estado_comunicacion = E_COMUNICA_COMPRUEBA_SUNIT_1;
						}
						else
						{
							estado_comunicacion = E_COMUNICA_INCIALIZA_SNAME_0;
						}
					}
				}
				else
				{
					estado_comunicacion = E_COMUNICA_FALLO;
				}
			}
			else
			{
				estado_comunicacion = E_COMUNICA_FALLO;
			}
			break;

		case E_COMUNICA_COMPRUEBA_SUNIT_1:
			if (xQueueReceive(handle_queue_comrx, &rx_msg, TIEMPO_ESPERA_RESPUESTA) == pdPASS)
			{
				if (rx_msg.command == REC_RESPUESTA)
				{
					// Si desplazo el de buffer_a_enviar +3 para quitar el "AT+"
					if (ok_mensaje_enviado(rx_msg.data, (buffer_a_enviar + 3)) == 0)
						estado_comunicacion = E_COMUNICA_FALLO;
					else { // Mensaje correcto: compruebo parametro SUNIT
						prepara_parametros(PAR_SUNIT_0 + repite_comando); //Para comparar
						// Si los parametros de este mensaje (o sea el nombre sensor) no
						// conciden con los recibidos de Kunak hay que escribirlos.
						if (ok_parametros_respuesta(rx_msg.data) == 0)
							inicializar_kunak = 1; // Flag para reescribir
						repite_comando++;
						if (repite_comando >= NUM_SENSORES)
							estado_comunicacion = E_COMUNICA_COMPRUEBA_SUNIT_2;
					}
				}
				else
				{
					estado_comunicacion = E_COMUNICA_FALLO;
				}
			}
			else
			{
				estado_comunicacion = E_COMUNICA_FALLO;
			}
			break;

		case E_COMUNICA_COMPRUEBA_SUNIT_2:
			if (xQueueReceive(handle_queue_comrx, &rx_msg, TIEMPO_ESPERA_RESPUESTA) == pdPASS)
			{
				if (rx_msg.command == REC_OK_COMANDO)
				{
					// Desplazo el puntero de buffer_a_enviar otros 3 (quito "AT+")
					if (ok_mensaje_enviado((rx_msg.data), (buffer_a_enviar + 3)) == 0)
						estado_comunicacion = E_COMUNICA_FALLO;
					else { // Mensaje correcto: AHORA NO HAY PARAMETRO
						if (inicializar_kunak == 0) {
							estado_comunicacion = E_COMUNICA_ENVIA_INFO_0;
						}
						else estado_comunicacion = E_COMUNICA_INCIALIZA_SNAME_0;
					}
				}
				else
				{
					estado_comunicacion = E_COMUNICA_FALLO;
				}
			}
			else
			{
				estado_comunicacion = E_COMUNICA_FALLO;
			}
			break;

		case E_COMUNICA_INICIALIZA_DEVNAME_1:
			prepara_parametros(PAR_DEVICENAME);
			construye_mensaje(M_AT_DEVNAME, COM_ESCRITURA);
			envio_buffer_serie(buffer_a_enviar, buffer_size);
			estado_comunicacion = E_COMUNICA_INICIALIZA_DEVNAME_2;
			break;

		case E_COMUNICA_INICIALIZA_DEVNAME_2:
			if (xQueueReceive(handle_queue_comrx, &rx_msg, TIEMPO_ESPERA_RESPUESTA) == pdPASS)
			{
				if (rx_msg.command == REC_OK_COMANDO)
				{
					// Desplazo el puntero de buffer_a_enviar otros 3 (quito "AT+")
					if (ok_mensaje_enviado((rx_msg.data), (buffer_a_enviar + 3)) == 0)
						estado_comunicacion = E_COMUNICA_FALLO;
					else { // Mensaje correcto: compruebo parametro n° serie
						// prepara_parametros() ya está hecho antes
						if (ok_parametros_respuesta(rx_msg.data) == 0) // Mal otra vez
							estado_comunicacion = E_COMUNICA_FALLO;
						else { // N° serie reescrito correctamente.
							// Hay que configurar toda la tarjeta de Kunak
							repite_comando = 0;
							envia_comando(PAR_SNAME_0);
							estado_comunicacion = E_COMUNICA_INCIALIZA_SNAME_1;
						}
					}
				}
				else
				{
					estado_comunicacion = E_COMUNICA_FALLO;
				}
			}
			else
			{
				estado_comunicacion = E_COMUNICA_FALLO;
			}
			break;

		case E_COMUNICA_INCIALIZA_SNAME_0:
			repite_comando = 0;
			envia_comando(PAR_SNAME_0);
			estado_comunicacion = E_COMUNICA_INCIALIZA_SNAME_1;
			break;

		case E_COMUNICA_INCIALIZA_SNAME_1:
			repite_comando++;
			if (repite_comando < NUM_SENSORES)  // Hacemos 6 SNAME
				envia_comando(PAR_SNAME_0 + repite_comando);
			else {
				repite_comando = 0;
				envia_comando(PAR_SUNIT_0);
				estado_comunicacion = E_COMUNICA_INCIALIZA_SUNIT_1;
			}
			break;

		case E_COMUNICA_INCIALIZA_SUNIT_1:
			repite_comando++;
			if (repite_comando < NUM_SENSORES)  // Hacemos 6 SUNIT
				envia_comando(PAR_SUNIT_0 + repite_comando);
			else {
				estado_comunicacion = E_COMUNICA_ENVIA_INFO_0;
			}
			break;

			// Aquí hacemos un envío de datos INFO.
		case E_COMUNICA_ENVIA_INFO_0:
			repite_comando = 0;
			envia_comando(PAR_INFO_VERS_FW);
			estado_comunicacion = E_COMUNICA_ENVIA_INFO_1;
			break;

		case E_COMUNICA_ENVIA_INFO_1:
			repite_comando++;
			if (repite_comando < NUM_COMANDOS_INFO) // NUM_COMANDOS_INFO vale 5
				envia_comando(PAR_INFO_VERS_FW + repite_comando);
			else
			{
				estado_comunicacion = E_COMUNICA_ENVIA_SENSORES_0; //Antes iba a E_COMUNICA_ESPERA_ENVIOS
			}
			break;

			// Hay que llamar a envia_sensores() y nos trae aquí para
			// hacer un envío de estado de servicios y dosis.
		case E_COMUNICA_ENVIA_SENSORES_0:
			repite_comando = 0;
			envia_comando(PAR_DOSIS_C1);
			estado_comunicacion = E_COMUNICA_ENVIA_SENSORES;
			break;

		case E_COMUNICA_ENVIA_SENSORES:
			repite_comando++;
			// Hacemos 4 SENSORES aunque hay 6
			if (repite_comando < 4) // NUM_SENSORES vale 6
				envia_comando(PAR_DOSIS_C1 + repite_comando);
			else
			{
				resetear_kunak = 0; // Contador de fallos
				estado_comunicacion = E_COMUNICA_ESPERA_ENVIOS;
			}
			break;

			// Estado base en el que está cuando no hacemos nada
			// ***** ESTADO DE REPOSO DE COMUNICACIONES *****
			// Si no hay nada que enviar ni recibir está aquí quieto.
			// Periodicamente envía un "AT" para comprobar que recibimos
			// un "OK" y seguimos conectados.
		case E_COMUNICA_ESPERA_ENVIOS:
			if (xQueueReceive(handle_queue_comtx, &tx_msg, 0) == pdPASS)
			{
				if (tx_msg.envio == ENVIO_DATOS_SENSORES)
					estado_comunicacion =  E_COMUNICA_ENVIA_SENSORES_0;
				if (tx_msg.envio == ENVIO_DATOS_INFO)
					estado_comunicacion = E_COMUNICA_ENVIA_INFO_0;
			}

			if (xQueueReceive(handle_queue_comrx, &rx_msg, 0) == pdPASS)
			{
				if (rx_msg.command == REC_INFO) // Mensaje asincrono
				{
					estado_comunicacion = E_COMUNICA_RECIBIDO_INFO;
				}
				else if (rx_msg.command == REC_ERROR)
				{
					estado_comunicacion = E_COMUNICA_RECIBIDO_ERROR;
				}
			}

			if ((xTaskGetTickCount() - tiempo_envio) >= TIEMPO_ENVIO_PERIODICO)
			{
				tiempo_envio = xTaskGetTickCount();
				estado_comunicacion = E_COMUNICA_ENVIA_AT_PER;
			}
			break;

		case E_COMUNICA_ENVIA_AT_PER:
			construye_mensaje(M_AT_, COM_EJECUCION);
			envio_buffer_serie(buffer_a_enviar, buffer_size);
			estado_comunicacion = E_COMUNICA_RECIBE_RESPUESTA_AT_PER;
			break;

		case E_COMUNICA_RECIBE_RESPUESTA_AT_PER:
			if (xQueueReceive(handle_queue_comrx, &rx_msg, TIEMPO_ESPERA_RESPUESTA) == pdPASS)
			{
				if (rx_msg.command == REC_OK)
				{
					estado_comunicacion = E_COMUNICA_ESPERA_ENVIOS;
				}
				else
				{
					estado_comunicacion = E_COMUNICA_FALLO;
				}
			}
			else
			{
				estado_comunicacion = E_COMUNICA_FALLO;
			}
			break;

			// Comandos asincronos que vienen de la nube.
		case E_COMUNICA_RECIBIDO_INFO:
			break;

		case E_COMUNICA_FALLO:
			if (rx_msg.command == REC_ERROR) {
				estado_comunicacion = E_COMUNICA_RECIBIDO_ERROR;
			}
			else {
				xQueueReset(handle_queue_comrx);
				estado_comunicacion = E_COMUNICA_REINTENTA;
			}
			break;

		case E_COMUNICA_REINTENTA:
			resetear_kunak++;
			if (resetear_kunak > 20)
			{
				estado_comunicacion = E_COMUNICA_RECIBIDO_ERROR;
			}
			else estado_comunicacion = E_COMUNICA_ENVIA_AT;
			break;

		case E_COMUNICA_RECIBIDO_ERROR:
			resetear_kunak++;
			if (resetear_kunak > 5)
			{
				_reset_kunak_on_();
				resetear_kunak = 0;
				estado_comunicacion = E_COMUNICA_TRATA_ERROR;
			}
			else estado_comunicacion = E_COMUNICA_ENVIA_AT;
			break;

		case E_COMUNICA_TRATA_ERROR:
			resetear_kunak++;
			if (resetear_kunak > 20) { // Espero varios ciclos de 100ms
				_reset_kunak_off_();
			vTaskDelay(TIME_5MIN);
			resetear_kunak = 0;
			estado_comunicacion = E_COMUNICA_ENVIA_AT;
			}
			break;

		default:
			break;
		}
		vTaskDelay(TIME_100MSEC);
	}
}

