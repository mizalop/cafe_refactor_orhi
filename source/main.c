/**
 * \file main.c
 * \brief Inicialización del HW, del SO y de las tareas.
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
 ******************************************************************************
VERSIONES SACADAS AL MUNDO:

NOTA SOBRE VERSIONES (26/05/2021):
 OLD_2020_09_29 ¿? alguna versión antigua
 ORIGINAL versión recibida de Ulma
 Orhy_MKE14Z...: versión modificada con acceso directo a menús, aviso de fresas y algún
       otro cambio que se había hecho en el Izaga
 Orhi_MKE15Z: misma versión que anterior pero adaptada a microprocesador MKE15Z
 CambiosIzaga: se van añadiendo los nuevos cambios que se han desarrollado en Izaga
    Eliminación de mensaje Auto -> inicialización de timer en task_main
    Unlock de WDOG -> curiosamente en el programa del Izaga estamos teniendo que
      meter primero la palabra más significativa y después la menos significativa
      para desbloquear. En cambio en Orhi, al revés.
    Inicialización de timer -> al exportar el proyecto a MKE15Z un timer no estaba
      inicializado y el programa iba lento
    Inicialización de variable result en task_main (STATUS_UNDEFINED) para que nunca
      haya problemas en inicialización de eeprom.
    Cambio en teclado.c #define LECTURA -> para que cuadre la medida con la máscara
	Modificación en LCD (text_str.c, lcd.c) para sacar correctamente los caracteres
	  especiales (es diferente driver al oled del izaga)
	Modificación en task_main para apagar backlight al apagar molino y encenderlo al
	  encenderlo
	Redirección de HardFault_Handler a ResetISR para que se resetee al instante si
	  hubiera algún hardfault
	Se modifica lcd.c para que el cursor no parpadee al apagar
    Se cambia número de serie G4 a G5
    Se mejoran apagados/encendidos de backlight y se le da más luminosidad por defecto
    Se arregla la falta de mensaje tras el aviso de cambio de fresas en task_main.c
      (se usa una función para ahorrar código)
    Se elimina modelo de pruebas como configuración. se deja una STD1 dummy para tener
      un ejemplo cuando haya que incluir nuevas configuraciones
    Se soluciona problema con printf (sólo afecta a cuando estás ejecutando programa
      conectado al ordenador) modificando las propiedades del proyecto.
    Se hace la función backlight_init para inicializar la frecuencia
      de la pwm del backlight.

	NO SE IMPLEMENTA:
	Retardo en grabación en eeprom: hasta no entender cuál es el
	  problema y por qué el retardo la soluciona.
	Comunicación d'Autore
	Desecho dosis
	Diagrama dosificador
	Dosis 3 cafés

	PENDIENTE:
	Mirar tema heap y stack (o copiarlo de Izaga)
	Watchdog a coroutine

v1.0 Lanzamiento Feria:
	Se lanza y da número de versión cuando las modificaciones anteriores funcionan bien

v1.1 Gráfico Dosificador:
    Se incluyen los gráficos de llenado/vaciado del dosificador.
    Para ello, se simplifica hmi.c/.h, se elimina la función de limpiar_ln (ya no se usa)
    También se reordena task_main y task_cafe para eso.
    Se corrigen errores de escritura en Alemán (ä, ö).
    Probando el cambio rápido se dan las condiciones para que ocurra un error que no suele
      pero sí puede ocurrir de normal. El error es que si, mientras se está moliendo se
      mantiene pulsado intro, entra al menú pero en unas condiciones en las que no actualiza
      el mensaje, por lo que parece que el molino se bloquea (durante 30s que es el timeout,
      aunque si se pulsa cualquier tecla  + - se recorre el menú normal por ejemplo).
      Se dan esas condiciones porque se podría aplicar el cambio rápido manteniendo pulsada
      la tecla intro hasta que se hace el cambio (pulsación larga -> entra en menú, se
      mantiene pulsado sin soltar -> intro a cambio rápido. problema: se sigue manteniendo
      exceso de tiempo y mientras muele (pasando de ondemand a premolido) coge pulsación
      larga tecla intro.
      Solución: al acabar cada tipo de servicio (simple, doble, orhi_iniciar), justo
      antes de poner a 1 flag de fin de servicio se resetea cola de teclado. Se decide
      hacer así porque no tiene razón de ser que se pulsen las teclas mientras está
      haciendo el proceso de moler.
      Nota: se mete antes de poner flag a 1 y no en incrementar_contadores porque hay alguna
      situación en la que no se incrementan contadores aunque se pase por algún servicio de
      task_cafe
      alguna situación pasando a ondemand sigue sin ir del todo bien
      la mejor opción parece mejorar la entrada de task_config_gb, que aunque está funcionando
      cuando no hay cambio rápido cara al usuario realmente hay que pulsar intro largo +
      intro corto para que arranque si se sale desde los estados de salir o de cambio rápido.
      (también se podría mejorar la salida y dejar este task parado en un punto en el que se
      ha pasado la espera de cola.
      se apaña con filtro tipo mutex (no permite mientras el filtro esté activo, que entre en
        task config gb) (ELIMINADO)
      finalmente, se modifica la entrada al menú de cambio rápido para que haya que pulsar dos
        veces, no sólo una pulsación muy larga. para hacer ese cambio, quitar/añadir la condición
        teclado_msg.pulsacion larga == pdFALSE del else if del EST_CAMBIO_MODO. si ésto está
        programado para entrar con una sola tecla, hay que activar el MUTEX. Si no, se puede
        quitar. Aunque se mantienen las limpiezas de teclado mientras se muele.
    nueva modificación: se limpia pantalla cuando se escribe algún mensaje en una sola linea
      como en los menús de ajuste de dosis
    nueva modificación: el mensaje de error power se deja 1 min al volver de reset pero luego
      se deja el molino en funcionamiento
    nueva modificación: se sustituye mensaje Auto y se modifica el reset MAG. se quita
      finalmente el mensaje de MAG ON porque según en qué punto esté el programa puede hacer
      cosas indeseadas. Idea: manejar un flag en task_lcd para saber siempre si está ocupada
      y en el caso del mensaje MAG ON esperar a que esté libre para sacarlo y seguir normal
      (para que task_main se pare hasta que salga ese mensaje).
      Se reduce el tiempo de reset del sensor a 5 segundos. se sigue manteniendo que sólo
      se resetee si detecta durante un minuto el imán.
      otra opción: meter el vTaskDelay en task_lcd tras sacar ese mensaje -> de esta manera el
        mensaje se mantiene durante los 20 segundos pero permite hacer cafés, etc. En cuanto
        salga del delay, resetear la cola de display. además, prohibir de algún modo la entrada
        a config y programación de dosis
      Finalmente se quita timer y se resuelve haciendo un reset de 2 segundos en los que sale
        el mensaje reset MAG. para usuario es lo más simple.
    nueva modificación: como se han mantenido los reset de teclado tras hacer servicios, da un
      problema en prueba intensiva-> no detecta la tecla intro para salirse de la prueba por
      teclado. por tanto, se añade un flag que indica si la prueba intensiva está encendida para
      que los reset de teclado no se hagan en ese caso.

    22/08/2022:
      se corrige el texto "PREG" en txt_str.c

v1.2 se añaden usuarios nacional e internacional para configuración por defecto de orhi
      NAC: idioma: español/cambio rápido desactivado
      INT: idioma: inglés /cambio rápido activo

v1.3 se añaden usuarios A y B

	 inicialización de configuración se pasa a inicializa.c/h como en Izaga. Se retoca, en
	   en base a esto hmi.c/h

	 código para guiaportas universal

	 se cambia la secuencia de salida de vaciado de dosificador

	 se añade menú de inicialización de idioma

	 se cambia mensaje de vaciado de dosificador por la tecla de intro si se entra desde el
	   menú. Si se entra con el acceso directo C1 + intro, sigue saliendo C1. En ambos
	   funcionan las dos teclas

	 mejora en timer_define.c/h para evitar algún error puntual que se podía dar

	 se hacen retoques de código para simplificar (dosis_simple, dosis_doble, rellenar_salir)

	 se borra ucgetTimerID() (no se usa, sólo se usó para pruebas)

v1.4 se reducen los usuarios a A B y siempre se inicializa con cambio rápido activo

     se añade a la versión de programa la letra indicadora del molino (versión o1.4)

     se fijan las dosis, se reduce el tiempo de pausa y se amplía el de relleno de la prueba de pesadas

	 se unifica a izaga el tema de dosis c2 y dosis c1 en cambio rápido. se corrige así el bug mítico de
	 moler indefinidamente. sólo se deja independiente la dosis c2 respecto a c1 si se está en ondemand
	 (puro, no ondemand con cambio rápido) y el dosificador vacío (no en cambio)

	 para deploy, se quita la consola de debug. se redirige printf a nada




*******************************************************************************/


/*******************************************************************************
 * Includes
 ******************************************************************************/

#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKE14Z7.h"
#include "fsl_debug_console.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "event_groups.h"

#include "fsl_gpio.h"

#include "AG5_define.h"
#include "task_main.h"
#include "motores.h"
#include "task_cafe.h"
#include "task_comunica.h"
#include "watchdog.h"

#include "main.h"


/*******************************************************************************
 * Variables Globales
 ******************************************************************************/

EventGroupHandle_t task_events;
EventGroupHandle_t wdt_events;


/*******************************************************************************
 * Funciones
 ******************************************************************************/

/*
 * @brief Punto de entrada de la aplicación.
 */
int main(void)
{
	// Inicializar hardware
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
	/* Init FSL debug console. */
	BOARD_InitDebugConsole();

	task_events = xEventGroupCreate();
	wdt_events = xEventGroupCreate();

	// Crear tareas del RTOS
	xTaskCreate(main_task,            "Tarea Principal",    configMINIMAL_STACK_SIZE + 96, NULL, main_task_PRIORITY,     NULL);
	xTaskCreate(moler_dosificar_task, "Tarea QBA",          configMINIMAL_STACK_SIZE,      NULL, cafe_task_PRIORITY,      NULL);
	xTaskCreate(watchdog_task,        "Tarea Watchdog",     configMINIMAL_STACK_SIZE,      NULL, watchdog_task_PRIORITY, NULL);
	xTaskCreate(comunicacion_task,    "Tarea Comunicacion", configMINIMAL_STACK_SIZE + 96, NULL, com_task_PRIORITY,      NULL);

	// Iniciar el scheduler
	vTaskStartScheduler();

	for (;;)
	{ 	/* Infinite loop to avoid leaving the main function */
		__asm("NOP");
		/* something to use as a breakpoint stop while looping */
	}
}


/*
 * @brief Hook para situaciones de StackOverflow.
 */
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed char *pcTaskName)
{
	(void) pxTask;
	(void) pcTaskName;

	for (;;)
	{
	}
}
