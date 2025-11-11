/*---------------------------------------------------------*/
/*   text strings created automatically from excel sheet   */
/*        Copyright ULMA Embedded Solutions - 2014         */
/*---------------------------------------------------------*/

#include <stddef.h>

//	Caracteres especiales según controlador del LCD con códigos (ST7066U, ST7066-0B)
//  Para romper la secuencia de escape iniciada con \x hay dos maneras:
//   - Cuando el siguiente caracter es una letra mayor que "F" o "f"
//	 - Poner dos cadenas seguidas concatenadas, o sea intercalar ""

//	 	á	é   í	ó	ú	É	ñ	Ñ	ö	ä	ü	è	Flecha der. <-
// \x   A0	82	A1  A2	A3	90	9B	9C	94	84	9A	8A	C7			C4
//
//		º	ä	ö	ü
// \x	B2	84	94	81

const char *English [] = 
{
		"1 CUP",			// 1  -  eTXT_1CAFE
		"2 CUPS",			// 2  -  eTXT_2CAFE
		"Grinding",			// 3  -  eTXT_MOLIENDO
		"Pr. dose",			// 4  -  eTXT_PROG_DOSIS
		"Dose 1c",			// 5  -  eTXT_DOSIS_1C
		"Dose 2c",			// 6  -  eTXT_DOSIS_2C
		"Service Counter",	// 7  -  eTXT_CONTADOR_SERVICIO
		"Partial Counter",	// 8  -  eTXT_CONTADOR_PARCIAL
		"Total Counter",	// 9  -  eTXT_CONTADOR_TOTAL
		"Delete?",			// 10  -  eTXT_BORRAR
		"Total Services",	// 11  -  eTXT_SERVICIOS_TOTALES
		"Exit",				// 12  -  eTXT_SALIR
		"Back",				// 13  -  eTXT_ATRAS
		"Part. C1",			// 14  -  eTXT_PARC_C1
		"Part. C2",			// 15  -  eTXT_PARC_C2
		"Part.sum",			// 16  -  eTXT_SUMA_PARCIALES
		"Total C1",			// 17  -  eTXT_TOT_C1
		"Total C2",			// 18  -  eTXT_TOT_C2
		"Serv.sum",			// 19  -  eTXT_SUMA_SERVICIOS
		"Tech. Service",	// 20  -  eTXT_SERVICIO_TECNICO
		"Porta Identif.",	// 21  -  eTXT_INDENTIFICACION_PORTA
		"Work Mode",		// 22  -  eTXT_MODO_TRABAJO
		"C1 Key",			// 23  -  eTXT_TECLA_C1
		"C2 Key",			// 24  -  eTXT_TECLA_C2
		"Last Key",			// 25  -  eTXT_TECLA_ULTIMA
		"Double Touch",		// 26  -  eTXT_DOBLE_PULSACION
		"Magnetic Sensor",	// 27  -  eTXT_SENSOR_MAGNETICO
		"OnDemand Grind",	// 28  -  eTXT_MOLIDO_ONDEMAND
		"PreGrind",			// 29  -  eTXT_PREMOLIDO
		"Barista",			// 30  -  eTXT_BARISTA
		"Empty Dosifier",	// 31  -  eTXT_VACIADO_DOSIFICADOR
		"Insert Porta",		// 32  -  eTXT_INSERTAR_PORTA
		"Press \xC4",		// 33  -  eTXT_TECLA_INTRO
		"Intensiv Test",	// 34  -  eTXT_PRUEBA_INTENSIVA
		"Cycles",			// 35  -  eTXT_CICLOS
		"Factory Data",		// 36  -  eTXT_DATOS_FABRICA
		"Serial Number",	// 37  -  eTXT_NUMERO_SERIE
		"Serial #",			// 38  -  eTXT_N_SERIE
		"User Password",	// 39  -  eTXT_PASSWORD_USUARIO
		"Password",			// 40  -  eTXT_PASSWORD
		"Choose Language",	// 41  -  eTXT_ELEGIR_IDIOMA
		"Light Level",		// 42  -  eTXT_NIVEL_LUZ
		"Level",			// 43  -  eTXT_NIVEL
		"Change",			// 44  -  eTXT_CAMBIO
		"Change Instant",	// 45  -  eTXT_CAMBIO_INSTANT
		"Language",			// 46  -  eTXT_IDIOMA
		"English",			// 47  -  eTXT_INGLES
		"Spanish",			// 48  -  eTXT_ESPANOL
		"German",			// 49  -  eTXT_ALEMAN
		"French",			// 50  -  eTXT_FRANCES
		"Italian",			// 51  -  eTXT_ITALIANO
		"NO",				// 52  -  eTXT_NO
		"YES",				// 53  -  eTXT_SI
		"Off",				// 54  -  eTXT_OFF
		"On",				// 55  -  eTXT_ON
		"Reset MAG",		// 56  -  eTXT_RESETMAG
		" NO \xC7""YES",	// 57  -  eTXT_SELECT_SI
		"\xC7""NO  YES",	// 58  -  eTXT_SELECT_NO
		"Remove Holder",	// 59  -  eTXT_RETIRE_PORTA
		"Weight Scale",		// 60  -  eTXT_SECUENCIA_PESADAS
		"Version",			// 61  -  eTXT_SW_VERSION
		"Refill",			// 62  -  eTXT_RECARGAR
		"Gauge Weight",		// 63  -  eTXT_CALIBRAR
		"Place Gauge",		// 64  -  eTXT_PESO_PATRON
		"Adjust",			// 65  -  eTXT_AJUSTE
		"Measure",			// 66  -  eTXT_MEDIDO
		"Power Error",		// 67  -  eTXT_ERROR_POWER
		"Model",			// 68  -  eTXT_MODELO
		"Orhi - A",			// 69  -  eTXT_ORHI_STD_A
		"Orhi - B",			// 70  -  eTXT_ORHI_STD_B
		"Model",			// 71  -  eTXT_MODELO1
		"Model",			// 72  -  eTXT_MODELO2
		"Model",			// 73  -  eTXT_MODELO3
		"Model",			// 74  -  eTXT_MODELO4
		"ONDE ",			// 75  -  eTXT_ONDE
		"PREG ",			// 76  -  eTXT_PREG
		"CHAN ",			// 77  -  eTXT_CHAN1
		"CHAN ",			// 78  -  eTXT_CHAN2
		"",					// 79  -  eTXT_MICR_LIBRE
		" C1  ",			// 80  -  eTXT_C1_ONDEM
		" C2  ",			// 81  -  eTXT_C2_ONDEM
		" C1  ",			// 82  -  eTXT_C1_PREM
		" C2  ",			// 83  -  eTXT_C2_PREM
		"Fast change",		// 84  -  eTXT_FAST_CHANGE
		"Use +/-",			// 85  -  eTXT_USE_KEYS
		"      ",			// 86  -  eTXT_EMPTY_BOX
		"Burr Alert",		// 87  -  eTXT_AVFR
		"Service n", 		// 88  -  eTXT_NCAFES
		"Change burrs!!", 	// 89  -  eTXT_MENSAJECAMBIO
		"Dose discard",     // 90  -  eTXT_TIRACAFE_LIBRE
		"Timeout",          // 91  -  eTXT_TIRACAFE_TIMEOUT_LIBRE
		"Discard dose!",    // 92  -  eTXT_TIRACAFE_MENSAJE_LIBRE
		"3 coffes",         // 93  -  eTXT_3DOSIS_ACTIVA
		"3 CUPS",           // 94  -  eTXT_3CAFE
		"Part. C3",     	// 95  -  eTXT_PARC_C3
		"Total C3",         // 96  -  eTXT_TOT_C3
		" C3  ",		    // 97  -  eTXT_C3_PREM
		" C3  ",  		    // 98  -  eTXT_C3_ONDEM
		"Fast Change",		// 99  -  eTXT_MOLIDO_CR
		"Mode change",		// 100 -  eTXT_CAMBIO_MODO
		""
};

const char *Spanish [] = 
{
		"1 CAFE",					// 1  -  eTXT_1CAFE
		"2 CAFES",					// 2  -  eTXT_2CAFE
		"Moliendo",					// 3  -  eTXT_MOLIENDO
		"Pr.dosis",					// 4  -  eTXT_PROG_DOSIS
		"Dosis 1c",					// 5  -  eTXT_DOSIS_1C
		"Dosis 2c",					// 6  -  eTXT_DOSIS_2C
		"Contador Servicio",		// 7  -  eTXT_CONTADOR_SERVICIO
		"Contador Parcial",			// 8  -  eTXT_CONTADOR_PARCIAL
		"Contador Total",			// 9  -  eTXT_CONTADOR_TOTAL
		"Borrar?",					// 10  -  eTXT_BORRAR
		"Servicios Total",			// 11  -  eTXT_SERVICIOS_TOTALES
		"Salir",					// 12  -  eTXT_SALIR
		"Atr\xA0""s",				// 13  -  eTXT_ATRAS
		"Parc. C1",					// 14  -  eTXT_PARC_C1
		"Parc. C2",					// 15  -  eTXT_PARC_C2
		"Suma par",					// 16  -  eTXT_SUMA_PARCIALES
		"Total C1",					// 17  -  eTXT_TOT_C1
		"Total C2",					// 18  -  eTXT_TOT_C2
		"Suma se.",			 		// 19  -  eTXT_SUMA_SERVICIOS
		"Servicio T\x82""cnico",	// 20  -  eTXT_SERVICIO_TECNICO
		"Identif. Porta",			// 21  -  eTXT_INDENTIFICACION_PORTA
		"Modo Trabajo",				// 22  -  eTXT_MODO_TRABAJO
		"Tecla C1",					// 23  -  eTXT_TECLA_C1
		"Tecla C2",					// 24  -  eTXT_TECLA_C2
		"Tecla \xA3""ltima",		// 25  -  eTXT_TECLA_ULTIMA
		"Doble Pulsa.",				// 26  -  eTXT_DOBLE_PULSACION
		"Sensor Magn\x82""tic",		// 27  -  eTXT_SENSOR_MAGNETICO
		"Molido OnDemand",			// 28  -  eTXT_MOLIDO_ONDEMAND
		"Premolid",					// 29  -  eTXT_PREMOLIDO
		"Barista",					// 30  -  eTXT_BARISTA
		"Vaciado Dosific.",			// 31  -  eTXT_VACIADO_DOSIFICADOR
		"Insertar Porta",			// 32  -  eTXT_INSERTAR_PORTA
		"Pulse \xC4",				// 33  -  eTXT_TECLA_INTRO
		"Prueba Intensiv",			// 34  -  eTXT_PRUEBA_INTENSIVA
		"Ciclos",					// 35  -  eTXT_CICLOS
		"Datos F\xA0""brica",		// 36  -  eTXT_DATOS_FABRICA
		"N\xA3""mero Serie",		// 37  -  eTXT_NUMERO_SERIE
		"N\xB2 Serie",				// 38  -  eTXT_N_SERIE
		"Password Usuario",			// 39  -  eTXT_PASSWORD_USUARIO
		"Password",					// 40  -  eTXT_PASSWORD
		"Elegir Idioma",			// 41  -  eTXT_ELEGIR_IDIOMA
		"Nivel Luz",				// 42  -  eTXT_NIVEL_LUZ
		"Nivel",					// 43  -  eTXT_NIVEL
		"Cambio",					// 44  -  eTXT_CAMBIO
		"Cambio Instant",			// 45  -  eTXT_CAMBIO_INSTANT
		"Idioma",					// 46  -  eTXT_IDIOMA
		"Ingl\x82""s",				// 47  -  eTXT_INGLES
		"Espa\x9B""ol",				// 48  -  eTXT_ESPANOL
		"Alem\xA0""n",				// 49  -  eTXT_ALEMAN
		"Franc\x82""s",				// 50  -  eTXT_FRANCES
		"Italiano",					// 51  -  eTXT_ITALIANO
		"NO",						// 52  -  eTXT_NO
		"SI",						// 53  -  eTXT_SI
		"Off",						// 54  -  eTXT_OFF
		"On",						// 55  -  eTXT_ON
		"Reset MAG",				// 56  -  eTXT_RESETMAG
		" NO \xC7""SI",				// 57  -  eTXT_SELECT_SI
		"\xC7""NO  SI",				// 58  -  eTXT_SELECT_NO
		"Retire porta",				// 59  -  eTXT_RETIRE_PORTA
		"Peso b\xA0""scula",		// 60  -  eTXT_SECUENCIA_PESADAS
		"Versi\xA2""n",				// 61  -  eTXT_SW_VERSION
		"Recargando",				// 62  -  eTXT_RECARGAR
		"Calibrar peso",			// 63  -  eTXT_CALIBRAR
		"Poner pesa",				// 64  -  eTXT_PESO_PATRON
		"Ajustar",					// 65  -  eTXT_AJUSTE
		"Medido",					// 66  -  eTXT_MEDIDO
		"Error Power",				// 67  -  eTXT_ERROR_POWER
		"Modelo",					// 68  -  eTXT_MODELO
		"Orhi - A",					// 69  -  eTXT_ORHI_STD_A
		"Orhi - B",					// 70  -  eTXT_ORHI_STD_B
		"Modelo",					// 71  -  eTXT_MODELO1
		"Modelo",					// 72  -  eTXT_MODELO2
		"Modelo",					// 73  -  eTXT_MODELO3
		"Modelo",					// 74  -  eTXT_MODELO4
		"ONDE ",					// 75  -  eTXT_ONDE
		"PREM ",					// 76  -  eTXT_PREG
		"CAMB ",					// 77  -  eTXT_CHAN1
		"CAMB ",					// 78  -  eTXT_CHAN2
		"",							// 79  -  eTXT_MICR_LIBRE
		" C1  ",					// 80  -  eTXT_C1_ONDEM
		" C2  ",					// 81  -  eTXT_C2_ONDEM
		" C1  ",					// 82  -  eTXT_C1_PREM
		" C2  ",					// 83  -  eTXT_C2_PREM
		"Cambio r\xA0""pido",		// 84  -  eTXT_FAST_CHANGE
		"Use +/-",					// 85  -  eTXT_USE_KEYS
		"      ",					// 86  -  eTXT_EMPTY_BOX
		"Aviso Fresas",				// 87  -  eTXT_AVFR
		"N Caf\x82""s",	 			// 88  -  eTXT_NCAFES
		"Cambiar fresas!!", 		// 89  -  eTXT_MENSAJECAMBIO
		"Desecho dosis",     		// 90  -  eTXT_TIRACAFE_LIBRE
		"Tiempo",		            // 91  -  eTXT_TIRACAFE_TIMEOUT_LIBRE
		"Desechar dosis!",		    // 92  -  eTXT_TIRACAFE_MENSAJE_LIBRE
		"3 caf\x82""s",         	// 93  -  eTXT_3DOSIS_ACTIVA
		"3 CAFES",           		// 94  -  eTXT_3CAFE
		"Parc. C3",     			// 95  -  eTXT_PARC_C3
		"Total C3",         		// 96  -  eTXT_TOT_C3
		" C3  ",		    		// 97  -  eTXT_C3_PREM
		" C3  ",  		    		// 98  -  eTXT_C3_ONDEM
		"Cambio R\xA0pido",			// 99  -  eTXT_MOLIDO_CR
		"Cambio modo",				// 100 -  eTXT_CAMBIO_MODO
		""
};

const char *Francais [] = 
{
		"1 CAFE",				// 1  -  eTXT_1CAFE
		"2 CAFES",				// 2  -  eTXT_2CAFE
		"Moudre",				// 3  -  eTXT_MOLIENDO
		"Pr. dose",				// 4  -  eTXT_PROG_DOSIS
		"Dose 1c",				// 5  -  eTXT_DOSIS_1C
		"Dose 2c",				// 6  -  eTXT_DOSIS_2C
		"Compteur Service",		// 7  -  eTXT_CONTADOR_SERVICIO
		"Compteur Partiel",		// 8  -  eTXT_CONTADOR_PARCIAL
		"Compteur Total",		// 9  -  eTXT_CONTADOR_TOTAL
		"Effacer?",				// 10  -  eTXT_BORRAR
		"Total Services",		// 11  -  eTXT_SERVICIOS_TOTALES
		"Sortir",				// 12  -  eTXT_SALIR
		"En_arri\x8A""re",		// 13  -  eTXT_ATRAS
		"Part. C1",				// 14  -  eTXT_PARC_C1
		"Part. C2",				// 15  -  eTXT_PARC_C2
		"Somme part.",			// 16  -  eTXT_SUMA_PARCIALES
		"Total C1",				// 17  -  eTXT_TOT_C1
		"Total C2",				// 18  -  eTXT_TOT_C2
		"Somme serv.",			// 19  -  eTXT_SUMA_SERVICIOS
		"Service tech.",		// 20  -  eTXT_SERVICIO_TECNICO
		"Identif. Porte-F.",	// 21  -  eTXT_INDENTIFICACION_PORTA
		"Mode Travail",			// 22  -  eTXT_MODO_TRABAJO
		"Touche C1",			// 23  -  eTXT_TECLA_C1
		"Touche C2",			// 24  -  eTXT_TECLA_C2
		"Derni\x8A""re Touche",	// 25  -  eTXT_TECLA_ULTIMA
		"Double Impuls.",		// 26  -  eTXT_DOBLE_PULSACION
		"Capteur magn\x82""t.",	// 27  -  eTXT_SENSOR_MAGNETICO
		"Moulu OnDem.",			// 28  -  eTXT_MOLIDO_ONDEMAND
		"Pr\x82""moulu",		// 29  -  eTXT_PREMOLIDO
		"Barista",				// 30  -  eTXT_BARISTA
		"Vidage Doseur",		// 31  -  eTXT_VACIADO_DOSIFICADOR
		"Ins\x82""rer Porta",	// 32  -  eTXT_INSERTAR_PORTA
		"Touche \xC4",			// 33  -  eTXT_TECLA_INTRO
		"Test intensif",		// 34  -  eTXT_PRUEBA_INTENSIVA
		"Cycles",				// 35  -  eTXT_CICLOS
		"Donn\x82""es Usine",	// 36  -  eTXT_DATOS_FABRICA
		"Num\x82""ro S\x82""rie",// 37  -  eTXT_NUMERO_SERIE
		"N\xB2"" S\x82""rie",	// 38  -  eTXT_N_SERIE
		"Password Usager",		// 39  -  eTXT_PASSWORD_USUARIO
		"Password  ",			// 40  -  eTXT_PASSWORD
		"Choisir Langue",		// 41  -  eTXT_ELEGIR_IDIOMA
		"Niveau lumi\x8A""re",	// 42  -  eTXT_NIVEL_LUZ
		"Niveau  ",				// 43  -  eTXT_NIVEL
		"Chang.",				// 44  -  eTXT_CAMBIO
		"Chang. instant.",		// 45  -  eTXT_CAMBIO_INSTANT
		"Langue",				// 46  -  eTXT_IDIOMA
		"Anglais",				// 47  -  eTXT_INGLES
		"Espagnol",				// 48  -  eTXT_ESPANOL
		"Allemand",				// 49  -  eTXT_ALEMAN
		"Francais",				// 50  -  eTXT_FRANCES
		"Italien",				// 51  -  eTXT_ITALIANO
		"NON",					// 52  -  eTXT_NO
		"OUI",					// 53  -  eTXT_SI
		"Off",					// 54  -  eTXT_OFF
		"On ",					// 55  -  eTXT_ON
		"Reset MAG",			// 56  -  eTXT_RESETMAG
		"NON \xC7""OUI",		// 57  -  eTXT_SELECT_SI
		"\xC7""NON OUI",		// 58  -  eTXT_SELECT_NO
		"Retirer Porte-F.",		// 59  -  eTXT_RETIRE_PORTA
		"Poids balance",		// 60  -  eTXT_SECUENCIA_PESADAS
		"Version",				// 61  -  eTXT_SW_VERSION
		"Recharge",			    // 62  -  eTXT_RECARGAR
		"Calibrer balance",		// 63  -  eTXT_CALIBRAR
		"Pacer poids",			// 64  -  eTXT_PESO_PATRON
		"R\x82""glage",			// 65  -  eTXT_AJUSTE
		"Mesur\x82""",			// 66  -  eTXT_MEDIDO
		"Power Error",			// 67  -  eTXT_ERROR_POWER
		"Mod\x8A""le",			// 68  -  eTXT_MODELO
		"Orhi - A",				// 69  -  eTXT_ORHI_STD_A
		"Orhi - B",				// 70  -  eTXT_ORHI_STD_B
		"Mod\x8A""le",			// 71  -  eTXT_MODELO1
		"Mod\x8A""le",			// 72  -  eTXT_MODELO2
		"Mod\x8A""le",			// 73  -  eTXT_MODELO3
		"Mod\x8A""le",			// 74  -  eTXT_MODELO4
		"ONDE ",				// 75  -  eTXT_ONDE
		"PREG ",				// 76  -  eTXT_PREG
		"CHAN ",				// 77  -  eTXT_CHAN1
		"CHAN ",				// 78  -  eTXT_CHAN2
		"",						// 79  -  eTXT_MICR_LIBRE
		" C1  ",				// 80  -  eTXT_C1_ONDEM
		" C2  ",				// 81  -  eTXT_C2_ONDEM
		" C1  ",				// 82  -  eTXT_C1_PREM
		" C2  ",				// 83  -  eTXT_C2_PREM
		"Change rapide",		// 84  -  eTXT_FAST_CHANGE
		"Use +/-",				// 85  -  eTXT_USE_KEYS
		"      ",				// 86  -  eTXT_EMPTY_BOX
		"Meule changement",		// 87  -  eTXT_AVFR
		"Compteur d'alerte",	// 88  -  eTXT_NCAFES
		"Changer meules!!",		// 89  -  eTXT_MENSAJECAMBIO
		"Dose jetement",    	// 90  -  eTXT_TIRACAFE_LIBRE
		"Temps",	        	// 91  -  eTXT_TIRACAFE_TIMEOUT_LIBRE
		"Jetez dose!",      	// 92  -  eTXT_TIRACAFE_MENSAJE_LIBRE
		"3 caf\x82""s",     	// 93  -  eTXT_3DOSIS_ACTIVA
		"3 CAFES",          	// 94  -  eTXT_3CAFE
		"Part. C3",     		// 95  -  eTXT_PARC_C3
		"Total C3",         	// 96  -  eTXT_TOT_C3
		" C3  ",		    	// 97  -  eTXT_C3_PREM
		" C3  ",  		    	// 98  -  eTXT_C3_ONDEM
		"Change rapide",		// 99  -  eTXT_MOLIDO_CR
		"Mode change",			// 100 -  eTXT_CAMBIO_MODO
		""
};

const char *Deutsch [] = 
{
		"1 KAFFEE",				// 1  -  eTXT_1CAFE
		"2 KAFFEES",			// 2  -  eTXT_2CAFE
		"Mahlvorg",				// 3  -  eTXT_MOLIENDO
		"Prog. Portion",		// 4  -  eTXT_PROG_DOSIS
		"Portion 1K",			// 5  -  eTXT_DOSIS_1C
		"Portion 2K",			// 6  -  eTXT_DOSIS_2C
		"Ausgabe z\x84""hler",	// 7  -  eTXT_CONTADOR_SERVICIO
		"Teil z\x84""hler",		// 8  -  eTXT_CONTADOR_PARCIAL
		"Gesamt z\x84""hler",	// 9  -  eTXT_CONTADOR_TOTAL
		"L\x81""schen?",		// 10  -  eTXT_BORRAR
		"Gesamt ausgaben",		// 11  -  eTXT_SERVICIOS_TOTALES
		"Beenden",				// 12  -  eTXT_SALIR
		"Zur\x81""ck",			// 13  -  eTXT_ATRAS
		"Teilz. K1",			// 14  -  eTXT_PARC_C1
		"Teilz. K2",			// 15  -  eTXT_PARC_C2
		"Summe Teilz.",			// 16  -  eTXT_SUMA_PARCIALES
		"Gesamt K1",			// 17  -  eTXT_TOT_C1
		"Gesamt K2",			// 18  -  eTXT_TOT_C2
		"Summe Ausg.",			// 19  -  eTXT_SUMA_SERVICIOS
		"Techn. Service",		// 20  -  eTXT_SERVICIO_TECNICO
		"Ident. Siebtr\x84""g",	// 21  -  eTXT_INDENTIFICACION_PORTA
		"Betriebs modus",		// 22  -  eTXT_MODO_TRABAJO
		"Taste K1",				// 23  -  eTXT_TECLA_C1
		"Taste K2",				// 24  -  eTXT_TECLA_C2
		"Letzte Taste",			// 25  -  eTXT_TECLA_ULTIMA
		"2x Tastendr",			// 26  -  eTXT_DOBLE_PULSACION
		"Magnet sensor",		// 27  -  eTXT_SENSOR_MAGNETICO
		"Mahl. OnDemand",		// 28  -  eTXT_MOLIDO_ONDEMAND
		"Vormahle",				// 29  -  eTXT_PREMOLIDO
		"Barista",				// 30  -  eTXT_BARISTA
		"Dosierer leeren",		// 31  -  eTXT_VACIADO_DOSIFICADOR
		"Siebtr\x84""g einsetze",// 32  -  eTXT_INSERTAR_PORTA
		"Taste \xC4",			// 33  -  eTXT_TECLA_INTRO
		"Intensiv test",		// 34  -  eTXT_PRUEBA_INTENSIVA
		"Zyklen",				// 35  -  eTXT_CICLOS
		"Werks daten",			// 36  -  eTXT_DATOS_FABRICA
		"Serien nummer",		// 37  -  eTXT_NUMERO_SERIE
		"Seriennr",				// 38  -  eTXT_N_SERIE
		"Benutzer Password",	// 39  -  eTXT_PASSWORD_USUARIO
		"Password",				// 40  -  eTXT_PASSWORD
		"Sprache w\x84""hlen",	// 41  -  eTXT_ELEGIR_IDIOMA
		"Licht niveau",			// 42  -  eTXT_NIVEL_LUZ
		"Niveau",				// 43  -  eTXT_NIVEL
		"Wechsel",				// 44  -  eTXT_CAMBIO
		"Wechs. OnDemand",		// 45  -  eTXT_CAMBIO_INSTANT
		"Sprache",				// 46  -  eTXT_IDIOMA
		"Englisch",				// 47  -  eTXT_INGLES
		"Spanisch",				// 48  -  eTXT_ESPANOL
		"Deutsch",				// 49  -  eTXT_ALEMAN
		"Franz\x94""si",		// 50  -  eTXT_FRANCES
		"Italieni",				// 51  -  eTXT_ITALIANO
		"NEIN",					// 52  -  eTXT_NO
		"JA",					// 53  -  eTXT_SI
		"Off",					// 54  -  eTXT_OFF
		"On",					// 55  -  eTXT_ON
		"Reset MAG",			// 56  -  eTXT_RESETMAG
		"NEIN \xC7""JA",		// 57  -  eTXT_SELECT_SI
		"\xC7""NEIN JA",		// 58  -  eTXT_SELECT_NO
		"Siebtr. entferne",		// 59  -  eTXT_RETIRE_PORTA
		"Gewicht Waage",		// 60  -  eTXT_SECUENCIA_PESADAS
		"Version",				// 61  -  eTXT_SW_VERSION
		"Nachf\x81""ll",		// 62  -  eTXT_RECARGAR
		"Gewicht kalibrie",		// 63  -  eTXT_CALIBRAR
		"Gewicht platzier",		// 64  -  eTXT_PESO_PATRON
		"Einstell",				// 65  -  eTXT_AJUSTE
		"Messwert",				// 66  -  eTXT_MEDIDO
		"Power Error",			// 67  -  eTXT_ERROR_POWER
		"Modell",				// 68  -  eTXT_MODELO
		"Orhi - A",				// 69  -  eTXT_ORHI_STD_A
		"Orhi - B",				// 70  -  eTXT_ORHI_STD_B
		"Modell",				// 71  -  eTXT_MODELO1
		"Modell",				// 72  -  eTXT_MODELO2
		"Modell",				// 73  -  eTXT_MODELO3
		"Modell",				// 74  -  eTXT_MODELO4
		"ONDE ",				// 75  -  eTXT_ONDE
		"PREG ",				// 76  -  eTXT_PREG
		"CHAN ",				// 77  -  eTXT_CHAN1
		"CHAN ",				// 78  -  eTXT_CHAN2
		"",						// 79  -  eTXT_MICR_LIBRE
		" C1  ",				// 80  -  eTXT_C1_ONDEM
		" C2  ",				// 81  -  eTXT_C2_ONDEM
		" C1  ",				// 82  -  eTXT_C1_PREM
		" C2  ",				// 83  -  eTXT_C2_PREM
		"Schnell wechsel",		// 84  -  eTXT_FAST_CHANGE
		"Use +/-",				// 85  -  eTXT_USE_KEYS
		"      ",				// 86  -  eTXT_EMPTY_BOX
		"Abnutzung",			// 87  -  eTXT_AVFR
		"Z\x84""hler",			// 88  -  eTXT_NCAFES
		"Wechseln!!",			// 89  -  eTXT_MENSAJECAMBIO
		"Abfalldosis",     		// 90  -  eTXT_TIRACAFE_LIBRE
		"Zeit ",          		// 91  -  eTXT_TIRACAFE_TIMEOUT_LIBRE
		"Dosis wegwerfen",  	// 92  -  eTXT_TIRACAFE_MENSAJE_LIBRE
		"3 kaffees",        	// 93  -  eTXT_3DOSIS_ACTIVA
		"3 KAFFEES",        	// 94  -  eTXT_3CAFE
		"Teilz. C3",     		// 95  -  eTXT_PARC_C3
		"Gesamt C3",        	// 96  -  eTXT_TOT_C3
		" C3  ",		    	// 97  -  eTXT_C3_PREM
		" C3  ",  		    	// 98  -  eTXT_C3_ONDEM
		"Schnell wechsel",		// 99  -  eTXT_MOLIDO_CR
		"Modus wechsel",		// 100 -  eTXT_CAMBIO_MODO
		""
};

const char *Italiano [] = 
{
		"1 CAFFE",				// 1  -  eTXT_1CAFE
		"2 CAFFE",				// 2  -  eTXT_2CAFE
		"Macinazione",			// 3  -  eTXT_MOLIENDO
		"Pr. dose",				// 4  -  eTXT_PROG_DOSIS
		"Dose 1c",				// 5  -  eTXT_DOSIS_1C
		"Dose 2c",				// 6  -  eTXT_DOSIS_2C
		"Contator Servizio",	// 7  -  eTXT_CONTADOR_SERVICIO
		"Contator Parziale",	// 8  -  eTXT_CONTADOR_PARCIAL
		"Contator Totale",		// 9  -  eTXT_CONTADOR_TOTAL
		"Elimina?",				// 10  -  eTXT_BORRAR
		"Servizi Totali",		// 11  -  eTXT_SERVICIOS_TOTALES
		"Esci",					// 12  -  eTXT_SALIR
		"Indietro",				// 13  -  eTXT_ATRAS
		"Parz. C1",				// 14  -  eTXT_PARC_C1
		"Parz. C2",				// 15  -  eTXT_PARC_C2
		"Somma par.",			// 16  -  eTXT_SUMA_PARCIALES
		"Totale C1",			// 17  -  eTXT_TOT_C1
		"Totale C2",			// 18  -  eTXT_TOT_C2
		"Somma serv.",			// 19  -  eTXT_SUMA_SERVICIOS
		"Servizio Tecnico",		// 20  -  eTXT_SERVICIO_TECNICO
		"Identif. Portaf.",		// 21  -  eTXT_INDENTIFICACION_PORTA
		"Modo Lavoro",			// 22  -  eTXT_MODO_TRABAJO
		"Tasto C1",				// 23  -  eTXT_TECLA_C1
		"Tasto C2",				// 24  -  eTXT_TECLA_C2
		"Ultimo Tasto",			// 25  -  eTXT_TECLA_ULTIMA
		"Doppio Tocco",			// 26  -  eTXT_DOBLE_PULSACION
		"Sensore Magnetic",		// 27  -  eTXT_SENSOR_MAGNETICO
		"Macinaz. OnDemand",	// 28  -  eTXT_MOLIDO_ONDEMAND
		"Premaci- nazione",		// 29  -  eTXT_PREMOLIDO
		"Barista",				// 30  -  eTXT_BARISTA
		"Svuotam. Dosat.",		// 31  -  eTXT_VACIADO_DOSIFICADOR
		"Inserire Portaf.",		// 32  -  eTXT_INSERTAR_PORTA
		"Tasto \xC4",			// 33  -  eTXT_TECLA_INTRO
		"Prova Intensiv",		// 34  -  eTXT_PRUEBA_INTENSIVA
		"Cicli",				// 35  -  eTXT_CICLOS
		"Dati Azienda",			// 36  -  eTXT_DATOS_FABRICA
		"Numero Serie",			// 37  -  eTXT_NUMERO_SERIE
		"N. Serie",				// 38  -  eTXT_N_SERIE
		"Password Utente",		// 39  -  eTXT_PASSWORD_USUARIO
		"Password",				// 40  -  eTXT_PASSWORD
		"Selezion Lingua",		// 41  -  eTXT_ELEGIR_IDIOMA
		"Livello Luce",			// 42  -  eTXT_NIVEL_LUZ
		"Livello",				// 43  -  eTXT_NIVEL
		"Cambio",				// 44  -  eTXT_CAMBIO
		"Cambio Istant",		// 45  -  eTXT_CAMBIO_INSTANT
		"Lingua",				// 46  -  eTXT_IDIOMA
		"Inglese",				// 47  -  eTXT_INGLES
		"Spagnolo",				// 48  -  eTXT_ESPANOL
		"Tedesco",				// 49  -  eTXT_ALEMAN
		"Francese",				// 50  -  eTXT_FRANCES
		"Italiano",				// 51  -  eTXT_ITALIANO
		"NO",					// 52  -  eTXT_NO
		"SI",					// 53  -  eTXT_SI
		"Off",					// 54  -  eTXT_OFF
		"On",					// 55  -  eTXT_ON
		"Reset MAG",			// 56  -  eTXT_RESETMAG
		" NO \xC7""SI",			// 57  -  eTXT_SELECT_SI
		"\xC7""NO  SI",			// 58  -  eTXT_SELECT_NO
		"Rimuover Portaf.",		// 59  -  eTXT_RETIRE_PORTA
		"Peso Bilancia",		// 60  -  eTXT_SECUENCIA_PESADAS
		"Versione",				// 61  -  eTXT_SW_VERSION
		"Ricarica",				// 62  -  eTXT_RECARGAR
		"Calibrar peso",		// 63  -  eTXT_CALIBRAR
		"Posizion  peso",		// 64  -  eTXT_PESO_PATRON
		"Regolare",				// 65  -  eTXT_AJUSTE
		"Misurato",				// 66  -  eTXT_MEDIDO
		"Error Power",			// 67  -  eTXT_ERROR_POWER
		"Modello",				// 68  -  eTXT_MODELO
		"Orhi - A",				// 69  -  eTXT_ORHI_STD_A
		"Orhi - B",				// 70  -  eTXT_ORHI_STD_B
		"Modello",				// 71  -  eTXT_MODELO1
		"Modello",				// 72  -  eTXT_MODELO2
		"Modello",				// 73  -  eTXT_MODELO3
		"Modello",				// 74  -  eTXT_MODELO4
		"ONDE ",				// 75  -  eTXT_ONDE
		"PREG ",				// 76  -  eTXT_PREG
		"CHAN ",				// 77  -  eTXT_CHAN1
		"CHAN ",				// 78  -  eTXT_CHAN2
		"",						// 79  -  eTXT_MICR_LIBRE
		" C1  ",				// 80  -  eTXT_C1_ONDEM
		" C2  ",				// 81  -  eTXT_C2_ONDEM
		" C1  ",				// 82  -  eTXT_C1_PREM
		" C2  ",				// 83  -  eTXT_C2_PREM
		"Cambio rapido",		// 84  -  eTXT_FAST_CHANGE
		"Use +/-",				// 85  -  eTXT_USE_KEYS
		"      ",				// 86  -  eTXT_EMPTY_BOX
		"Alerta macine",		// 87  -  eTXT_AVFR
		"Numero caff\x8A""", 	// 88  -  eTXT_NCAFES
		"Cambiare macine!!",	// 89  -  eTXT_MENSAJECAMBIO
		"Dose scarto",     		// 90  -  eTXT_TIRACAFE_LIBRE
		"Tempo",          		// 91  -  eTXT_TIRACAFE_TIMEOUT_LIBRE
		"Scartare dose!",    	// 92  -  eTXT_TIRACAFE_MENSAJE_LIBRE
		"3 caffe",        	 	// 93  -  eTXT_3DOSIS_ACTIVA
		"3 CAFFE",           	// 94  -  eTXT_3CAFE
		"Parz. C3",     		// 95  -  eTXT_PARC_C3
		"Totale C3",         	// 96  -  eTXT_TOT_C3
		" C3  ",		    	// 97  -  eTXT_C3_PREM
		" C3  ",  		    	// 98  -  eTXT_C3_ONDEM
		"Cambio rapido",		// 99  -  eTXT_MOLIDO_CR
		"Cambio modo",			// 100 -  eTXT_CAMBIO_MODO
		""
};

const char ** languages_table [] = {
		English,
		Spanish,
		Deutsch,
		Francais,
		Italiano,
		NULL };
