# Refactorización 2026

Sobre el programa **orhi_v1.4**, se refactoriza *task_cafe* y *task_motores*.

##Objetivo:
1. task_motores 
    * eliminar la tarea y pasarlo a funciones que ejecuten el movimiento de los Mabuccis
2. task_cafe
    * sacar partes de código *"máximo común divisor"* a funciones
    * para diferentes modos de trabajo, sacar estructuras constantes de secuencia
    * tarea: que sea un motor que carga la secuencia correcta y la recorre

##Resultado de la prueba:
1. task_motores
	* implementado completamente
	* sin probar a fondo el funcionamiento, pero en general va bien y reduce algo memoria flash
	* sin probar cómo queda la memoria de FreeRTOS (hay que mirar marcas de agua, etc.), pero es de esperar que haya quedado más memoria libre también (1 tarea y 1 cola menos)
2. task_cafe
    * es un funcionamiento simplificado de la tarea, un ejemplo/plantilla de cómo hacerlo


##Pendiente
1. acabar, basándose en el ejemplo, la implementación
2. verificar memorias y tiempos de ejecución
3. hacer prueba de implementación de nueva secuencia de molido (modo de trabajo, dosis C3...) para ver facilidad de uso
4. detalles y mejoras:
    * los arrays de secuencia, mejor hacerlos const
    * añadir tests y robustecer las funciones
    * limpieza de getVar y similares
    * se pueden meter las secuencias en una tabla a la que se entre con 3 cordenadas [SERV][MODO][TRAMPILLA]
    * se pueden meter los mensajes en una tabla para seleccionar en función de modo de trabajo y servicio
    
5. ojo:
	* ha habido que subir considerablemente el stack de la tarea. ver si la implementación de motor supone mucho incremento de memoria (quizá hubiera que quitar printfs, limpiar la parte de ORHI_INICIAR y recolocarla en otro sitio, etc. para hacerlo a fondo)
	* tal y como está hay mejoras para hacer en las funciones: algún todo, alguna cosa que se puede reordenar, se ve un parpadeo que habría que quitar, etc.
	* se podría sustituir la máquina de estados por unos arrays que contengan directamente las funciones a ejecutar
	* se puede abstraer más
	

##Para que funcione el programa:
Parchear Orhi v1.4 con los archivos de este repositorio.