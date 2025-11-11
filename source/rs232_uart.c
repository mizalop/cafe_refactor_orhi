/**
 * \file rs232_uart.c
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

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "board.h"
#include "peripherals.h"
#include "fsl_lpuart.h"

#include "task_comunica.h"
#include "rs232_uart.h"

/*******************************************************************************
 * Variables Privadas
 ******************************************************************************/

static char buffer_recepcion[LONG_BUFFER_REC];
static unsigned char pos_buffer_rec = 0;

/*******************************************************************************
 * Funciones
 ******************************************************************************/

void COMUNICA_LPUART_IRQHandler(void)
{
	if ((kLPUART_RxDataRegFullFlag)
			& LPUART_GetEnabledInterrupts(COMUNICA_LPUART_PERIPHERAL))
	{
		/* If new data arrived. */
		if ((kLPUART_RxDataRegFullFlag) & LPUART_GetStatusFlags(COMUNICA_LPUART_PERIPHERAL))
		{
			if (pos_buffer_rec < LONG_BUFFER_REC)
			{
				buffer_recepcion[pos_buffer_rec] = LPUART_ReadByte(COMUNICA_LPUART_PERIPHERAL);
				pos_buffer_rec++;

				if ((buffer_recepcion[pos_buffer_rec - 2] == _CR_) && (buffer_recepcion[pos_buffer_rec - 1] == _LF_))
				{
					parseo_mensaje(buffer_recepcion, pos_buffer_rec);
					pos_buffer_rec = 0;
				}
			}
			else // vaciar UART, limpiar flags y reiniciar índices
			{
				while (!((kLPUART_RxFifoEmptyFlag) & LPUART_GetStatusFlags(COMUNICA_LPUART_PERIPHERAL)))
				{
					LPUART_ReadByte(COMUNICA_LPUART_PERIPHERAL);
				}
				LPUART_ClearStatusFlags(COMUNICA_LPUART_PERIPHERAL, kLPUART_RxOverrunFlag);
				pos_buffer_rec = 0;
			}
		}
	}
}

void envio_buffer_serie(char *buffer, size_t size)
{
	LPUART_WriteBlocking(COMUNICA_LPUART_PERIPHERAL, (uint8_t*) buffer, size);
}
