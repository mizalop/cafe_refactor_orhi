/*---------------------------------------------------------*/
/* text identifiers created automatically from excel sheet */
/*        Copyright ULMA Embedded Solutions - 2014         */
/*---------------------------------------------------------*/
#ifndef __TEXT_IDS_H__
#define __TEXT_IDS_H__


extern const char ** languages_table [];

typedef enum
{
	eTXT_1CAFE                                     = 1,     /* "1 CUP" */
	eTXT_2CAFE                                     = 2,     /* "2 CUPS" */
	eTXT_MOLIENDO                                  = 3,     /* "Grinding" */
	eTXT_PROG_DOSIS                                = 4,     /* "Pr. dose" */
	eTXT_DOSIS_1C                                  = 5,     /* "Dose 1c" */
	eTXT_DOSIS_2C                                  = 6,     /* "Dose 2c" */
	eTXT_CONTADOR_SERVICIO                         = 7,     /* "Service Counter" */
	eTXT_CONTADOR_PARCIAL                          = 8,     /* "Partial Counter" */
	eTXT_CONTADOR_TOTAL                            = 9,     /* "Total Counter" */
	eTXT_BORRAR                                    = 10,    /* "Delete?" */
	eTXT_SERVICIOS_TOTALES                         = 11,    /* "Total Services" */
	eTXT_SALIR                                     = 12,    /* "Exit" */
	eTXT_ATRAS                                     = 13,    /* "Back" */
	eTXT_PARC_C1                                   = 14,    /* "Part. C1" */
	eTXT_PARC_C2                                   = 15,    /* "Part. C2" */
	eTXT_SUMA_PARCIALES                            = 16,    /* "Part.sum" */
	eTXT_TOT_C1                                    = 17,    /* "Total C1" */
	eTXT_TOT_C2                                    = 18,    /* "Total C2" */
	eTXT_SUMA_SERVICIOS                            = 19,    /* "Serv.sum" */
	eTXT_SERVICIO_TECNICO                          = 20,    /* "Tech. Service" */
	eTXT_INDENTIFICACION_PORTA                     = 21,    /* "Porta Identif." */
	eTXT_MODO_TRABAJO                              = 22,    /* "Work Mode" */
	eTXT_TECLA_C1                                  = 23,    /* "C1 Key" */
	eTXT_TECLA_C2                                  = 24,    /* "C2 Key" */
	eTXT_TECLA_ULTIMA                              = 25,    /* "Last Key" */
	eTXT_DOBLE_PULSACION                           = 26,    /* "Double Touch" */
	eTXT_SENSOR_MAGNETICO                          = 27,    /* "Magnetic Sensor" */
	eTXT_MOLIDO_ONDEMAND                           = 28,    /* "OnDemand Grind" */
	eTXT_PREMOLIDO                                 = 29,    /* "PreGrind" */
	eTXT_BARISTA                                   = 30,    /* "Barista" */
	eTXT_VACIADO_DOSIFICADOR                       = 31,    /* "Empty Dosifier" */
	eTXT_INSERTAR_PORTA                            = 32,    /* "Insert Porta" */
	eTXT_TECLA_INTRO                               = 33,    /* "Key to exit" */
	eTXT_PRUEBA_INTENSIVA                          = 34,    /* "Intensiv Test" */
	eTXT_CICLOS                                    = 35,    /* "Cycles" */
	eTXT_DATOS_FABRICA                             = 36,    /* "Factory Data" */
	eTXT_NUMERO_SERIE                              = 37,    /* "Serial Number" */
	eTXT_N_SERIE                                   = 38,    /* "Serial #" */
	eTXT_PASSWORD_USUARIO                          = 39,    /* "User Password" */
	eTXT_PASSWORD                                  = 40,    /* "Password" */
	eTXT_ELEGIR_IDIOMA                             = 41,    /* "Choose Language" */
	eTXT_NIVEL_LUZ                                 = 42,    /* "Light Level" */
	eTXT_NIVEL                                     = 43,    /* "Level" */
	eTXT_CAMBIO                                    = 44,    /* "Change" */
	eTXT_CAMBIO_INSTANT                            = 45,    /* "Change Instant" */
	eTXT_IDIOMA                                    = 46,    /* "Language" */
	eTXT_INGLES                                    = 47,    /* "English" */
	eTXT_ESPANOL                                   = 48,    /* "Spanish" */
	eTXT_ALEMAN                                    = 49,    /* "German" */
	eTXT_FRANCES                                   = 50,    /* "French" */
	eTXT_ITALIANO                                  = 51,    /* "Italian" */
	eTXT_NO                                        = 52,    /* "NO" */
	eTXT_SI                                        = 53,    /* "YES" */
	eTXT_OFF                                       = 54,    /* "Off" */
	eTXT_ON                                        = 55,    /* "On" */
	eTXT_RESETMAG                                  = 56,    /* "Reset MAG" */
	eTXT_SELECT_SI                                 = 57,    /* " NO ~YES" */
	eTXT_SELECT_NO                                 = 58,    /* "~NO  YES" */
	eTXT_RETIRE_PORTA                              = 59,    /* "Remove Holder" */
	eTXT_SECUENCIA_PESADAS						   = 60,	/* "Weight Scale" */
	eTXT_SW_VERSION                                = 61,    /* "Version" */
	eTXT_RECARGAR                                  = 62,    /* "Refill" */
	eTXT_CALIBRAR_LIBRE                            = 63,    /* "Gauge Weight" */
	eTXT_PESO_PATRON_LIBRE                         = 64,    /* "Put Gauge" */
	eTXT_AJUSTE                                    = 65,    /* "Adjust"  */
	eTXT_MEDIDO                                    = 66,    /* "Measure" */
	eTXT_ERROR_POWER                               = 67,    /* "Power Error" */
	eTXT_MODELO									   = 68,    /* "Model" */
	eTXT_ORHI_STD_A  							   = 69,    /* "Orhi - A" */
	eTXT_ORHI_STD_B	    						   = 70,    /* "Orhi - B" */
	eTXT_MODELO1    							   = 71,    /* "Model" */
	eTXT_MODELO2	    						   = 72,    /* "Model" */
	eTXT_MODELO3 								   = 73,    /* "Model" */
	eTXT_MODELO4								   = 74,    /* "Model" */
	eTXT_ONDE									   = 75,	/* "ONDE"*/
	eTXT_PREG									   = 76,	/* "PREG"*/
	eTXT_CHAN1									   = 77,	/* "CHAN"*/
	eTXT_CHAN2									   = 78,	/* "CHAN"*/
	eTXT_MICR_LIBRE								   = 79,	/* " "*/
	eTXT_C1_ONDEM								   = 80,	/* "C1"*/
	eTXT_C2_ONDEM								   = 81,	/* "C2"*/
	eTXT_C1_PREM								   = 82,	/* "C1"*/
	eTXT_C2_PREM								   = 83,	/* "C2"*/
	eTXT_FAST_CHANGE							   = 84,	/* "Fast change"*/
	eTXT_USE_KEYS								   = 85,	/* "Use +/-"*/
	eTXT_EMPTY_BOX								   = 86,	/* ""*/
	eTXT_AVFR						   			   = 87,    /* "Aviso Fresas"*/
	eTXT_NCAFES								   	   = 88,    /* "N Caf\x96s" */
	eTXT_MENSAJECAMBIO							   = 89,    /* "Cambiar fresas!!" */
	eTXT_TIRACAFE_LIBRE							   = 90,	/* "Dose discard"*/
	eTXT_TIRACAFE_TIMEOUT_LIBRE					   = 91,	/* "Timeout"*/
	eTXT_TIRACAFE_MENSAJE_LIBRE					   = 92,	/* "Discard dose!"*/
	eTXT_3DOSIS_ACTIVA							   = 93,	/* ""*/
	eTXT_3CAFE 									   = 94,	/* ""*/
	eTXT_PARC_C3								   = 95,	/* ""*/
	eTXT_TOT_C3									   = 96,	/* ""*/
	eTXT_C3_PREM								   = 97,	/* ""*/
	eTXT_C3_ONDEM								   = 98,	/* ""*/
	eTXT_MOLIDO_CR								   = 99,	/* ""*/
	eTXT_CAMBIO_MODO							   = 100,	/* ""*/
	eTXT_NULL
} TEXT_ID_e;

#endif /* __TEXTS_IDS_H__ */
/*---------------------------------------------------------*/
