/**
 * \file rs232_uart.h
 * \brief Módulo de funciones de bajo nivel del puerto serie
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

#ifndef SOURCE_RS232_UART_H_
#define SOURCE_RS232_UART_H_

/*******************************************************************************
 * Definiciones
 ******************************************************************************/

#define LONG_BUFFER_ENVIO   50
#define LONG_BUFFER_REC     50

// Datos para la UART
#define _CR_                0x0D
#define _LF_                0x0A
#define _END_M_             0x0D,0x0A

/*******************************************************************************
 * Funciones
 ******************************************************************************/

void envio_buffer_serie(char *buffer, size_t size);

#endif /* SOURCE_RS232_UART_H_ */
