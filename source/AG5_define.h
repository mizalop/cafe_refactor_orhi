/**
 * \file AG5_define.h
 * \brief Enumeraciones y defines generales
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


#ifndef SOURCE_AG5_DEFINE_H_
#define SOURCE_AG5_DEFINE_H_


/*******************************************************************************
 * Enumeraciones
 ******************************************************************************/

/**
 * @brief ON y OFF
 */
enum
{
	OFF = 0,    /**< OFF */
	ON          /**< ON */
};


/**
 * @brief SI y NO
 */
enum
{
	NO = 0,   /**< NO */
	SI        /**< SI */
};


/**
 * @brief Tipos de servicio de café
 */
typedef enum
{
	SIMPLE = 0,     /**< Servicio simple */
	DOBLE,          /**< Servicio doble */
	ORHI_INICIAR     /**< Iniciar */
} TIPO_SERVICIO_e;


/**
 * @brief Estado de trampillas
 */
typedef enum
{
	NO_CAFE = 0,    /**< Trampilla inferior vacía */
	T1CAFE,         /**< Café en trampilla inferior */
	T1T2CAFE        /**< Café en las dos trampillas */
} ESTADO_TRAMPILLAS_e;


/**
 * @brief Modos de trabajo
 */
typedef enum
{
	PREMOLIDO = 0,      /**< Fast o Premolido */
	ONDEMAND,           /**< Fresh o Instant */
	BARISTA,            /**< Barista */
	NUM_OPCIONES_MODO   /**< Número total de modos de trabajo */
} MODO_TRABAJO_e;


/**
 * @brief Indicador de qué porta lleva el tag (imán)
 */
typedef enum
{
	TAG_C1 = 0,   /**< El tag está en el porta de un café */
	TAG_C2        /**< El tag está en el porta de dos cafés */
} TAG_e;


/**
 * @brief Modos de identificacion del porta
 */
typedef enum
{
	SENSOR_MAG = 0,         /**< Identificación por sensor magnético */
	DOBLE_CLICK,            /**< Identificación por doble click */
	TECLADO_C1,             /**< Identificación por botón C1 */
	TECLADO_C2,             /**< Identificación por botón C2 */
	TECLADO_LAST,           /**< Identificación por botones C1 y C2 */
	NUM_OPCIONES_IDENTIF    /**< Número total de modos de identificación */
} OPCIONES_IDENTIF_e;

/**
 * @brief Modelo configuración previa para clientes con programas personalizados
 */
typedef enum
{
	ORHI_STD_A = 0, /**< Orhi estándar nacional - A*/
	ORHI_STD_B,		/**< Orhi estándar nacional - B*/
	NUM_MODELOS   		/**< Número total de modelos */
} MODELOS_e;

/**
 * @brief Tipos de contadores a visualizar
 */
typedef enum
{
	PARCIALES = 0,    /**< Contadores parciales */
	TOTALES           /**< Contadores totales */
} CONTADORES_e;


/**
 * @brief Idiomas de los mensajes
 */
typedef enum
{
	INGLES = 0,   /**< Inglés */
	CASTELLANO,   /**< Castellano */
	ALEMAN,
	FRANCES,
	ITALIANO,
	NUM_IDIOMAS   /**< Número total de idiomas */
} IDIOMA_e;


/**
 * @brief Tipos de servicio que se seleccionan
 */
typedef enum
{
	SERV_AUTOMATICO_1 = 0,    /**< Servicio simple por sensor magnetico */
	SERV_AUTOMATICO_2,        /**< Servicio doble por sensor magnetico */
	SERV_SIMPLE,              /**< Servicio simple por tecla C2 */
	SERV_DOBLE,               /**< Servicio doble por tecla C2 */
	SERV_PROV,                /**< Posible doble click en el micro porta */
	SERV_SIMPLE_PROV,         /**< Servicio simple por doble click en el micro porta */
	SERV_DOBLE_PROV,          /**< Servicio doble por doble click en el micro porta */
	NUM_SERV                  /**< Número total de tipos de servicio */
} SERV_e;

/**
 * @brief Lista de variables a las que hay que cambiar valor desde otras rutinas
 */
typedef enum
{
	TIPO_SERVICIO_PREV = 0,
	TIPO_SERVICIO,
	ESTADO_AG
} VAR_t;

/*******************************************************************************
 * Definiciones
 ******************************************************************************/

#define EV_MOTOR          (1 << 0)
#define EV_FIN_SERV       (1 << 1)
#define EV_DOSIS          (1 << 2)
#define EV_CONFIG         (1 << 3)
//#define EV_CONFIG_MODO  (1 << 4) -> Este bit se utiliza para TFT
#define EV_LCD			  (1 << 5)
//#define EV_MUTEXCONFIG  (1 << 6) -> sólo para cuando se hace el cambio rápido con 1 tecla
#define EV_ERRORPOWER_OFF (1 << 7) //para apagar mensaje de error power
#define EV_INTENSIVA_ON   (1 << 8) //para no resetear el teclado después de un servicio (si prueba intensiva)

#define EV_WDT_MAIN       (1 << 0)


#endif /* SOURCE_AG5_DEFINE_H_ */
