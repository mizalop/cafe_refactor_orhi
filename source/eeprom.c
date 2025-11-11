/**
 * \file eeprom.c
 * \brief Funciones de inicialización, escritura y lectura de EEPROM.
 *
 * \version Markibar_Control_1_0
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

#include "fsl_debug_console.h"

#include "AG5_define.h"
#include "task_dosis.h"
#include "task_cafe.h"
#include "hmi.h"
#include "backlight.h"
#include "avisoFresas.h"

#include "eeprom.h"


/*******************************************************************************
 * Definiciones
 ******************************************************************************/

// EEESIZE (FCCOB4[3:0]) '0111' = 128 Bytes
#define EEPROM_DATA_SET_SIZE_CODE (0x37U)
// DEPART (FCCOB5[3:0) '1001' Dataflash: 8KB EEPROM backup: 24KB
#define FLEXNVM_PARTITION_CODE    (0x9U)


/*******************************************************************************
 * Variables Globales
 ******************************************************************************/

static uint32_t dflashBlockBase = 0;
static uint32_t dflashTotalSize = 0;
static uint32_t eepromBlockBase = 0;
static uint32_t eepromTotalSize = 0;
static uint32_t flexramBlockBase = 0;
static uint32_t flexramTotalSize = 0;


/*******************************************************************************
 * Variables Privadas
 ******************************************************************************/

/*! @brief Flash driver Structure */
static flexnvm_config_t s_flashDriver;


/*******************************************************************************
 * Funciones
 ******************************************************************************/

status_t inicializar_eeprom(void)
{
	ftfx_security_state_t securityStatus = kFTFx_SecurityStateNotSecure; /* Return protection status */
	status_t result; /* Return code from each flash driver function */

	/* Clean up Flash driver Structure */
	memset(&s_flashDriver, 0, sizeof(flexnvm_config_t));

	/* Setup flash driver structure for device and initialize variables. */
	result = FLEXNVM_Init(&s_flashDriver);
	if (kStatus_FTFx_Success != result)
	{
		PRINTF("\r\n\r\n\r\n\t---- FLASH ERROR! ----");
		return result;
	}

	/* Check security status. */
	result = FLEXNVM_GetSecurityState(&s_flashDriver, &securityStatus);
	if (kStatus_FTFx_Success != result)
	{
		PRINTF("\r\n\r\n\r\n\t---- FLASH ERROR! ----");
		return result;
	}

	if (kFTFx_SecurityStateNotSecure != securityStatus)
	{
		PRINTF("\r\n EEprom operation will not be executed, as Flash is SECURE!");
		return result;
	}
	else
	{
		/* Get flash properties*/
		FLEXNVM_GetProperty(&s_flashDriver, kFLEXNVM_PropertyDflashBlockBaseAddr, &dflashBlockBase);
		FLEXNVM_GetProperty(&s_flashDriver, kFLEXNVM_PropertyFlexRamBlockBaseAddr, &flexramBlockBase);
		FLEXNVM_GetProperty(&s_flashDriver, kFLEXNVM_PropertyFlexRamTotalSize, &flexramTotalSize);

		FLEXNVM_GetProperty(&s_flashDriver, kFLEXNVM_PropertyEepromTotalSize, &eepromTotalSize);
		if (!eepromTotalSize)
		{
			/* Note: The EEPROM backup size must be at least 16 times the EEPROM partition size in FlexRAM. */
			uint32_t eepromDataSizeCode = EEPROM_DATA_SET_SIZE_CODE;
			uint32_t flexnvmPartitionCode = FLEXNVM_PARTITION_CODE;

			PRINTF("\r\n There is no available EEprom (FlexNVM) on this Device by default.");
			PRINTF("\r\n Trying to configure FlexNVM block as EEprom.");

			result = FLEXNVM_ProgramPartition(&s_flashDriver, kFTFx_PartitionFlexramLoadOptLoadedWithValidEepromData,
					eepromDataSizeCode, flexnvmPartitionCode);
			if (kStatus_FTFx_Success != result)
			{
				PRINTF("\r\n\r\n\r\n\t---- FLASH ERROR! ----");
				return result;
			}

			/* Reset MCU */
			PRINTF("\r\n Perform a system reset \r\n");
			NVIC_SystemReset();
		}

		FLEXNVM_GetProperty(&s_flashDriver, kFLEXNVM_PropertyDflashTotalSize, &dflashTotalSize);
		eepromBlockBase = dflashBlockBase + dflashTotalSize;

		/* Print flash information - EEprom. */
		PRINTF("\r\n EEprom Information: ");
		PRINTF("\r\n EEprom Base Address: (0x%x) ", eepromBlockBase);
		PRINTF("\r\n EEprom Total Size:\t%d B", eepromTotalSize);

		PRINTF("\r\n Make FlexRAM available for EEPROM ");
		result = FLEXNVM_SetFlexramFunction(&s_flashDriver, kFTFx_FlexramFuncOptAvailableForEeprom);
		if (kStatus_FLASH_Success != result)
		{
			PRINTF("\r\n\r\n\r\n\t---- FLASH ERROR! ----");
			return result;
		}

		PRINTF("\r\n Now EEPROM data is read and written by accessing the FlexRAM address space ");
		PRINTF("\r\n FlexRAM Base Address: (0x%x) ", flexramBlockBase);
	}

	return result;
}

status_t grabar_word_eeprom(uint8_t posicion, uint16_t data)
{
	return FLEXNVM_EepromWrite(&s_flashDriver, flexramBlockBase + posicion, (uint8_t *)&data, sizeof(data));
}

status_t grabar_int_eeprom(uint8_t posicion, uint32_t data)
{
	return FLEXNVM_EepromWrite(&s_flashDriver, flexramBlockBase + posicion, (uint8_t *)&data, sizeof(data));
}

uint16_t leer_word_eeprom(uint8_t posicion)
{
	return *(volatile uint16_t *)(flexramBlockBase + posicion);
}

uint32_t leer_int_eeprom(uint8_t posicion)
{
	return *(volatile uint32_t *)(flexramBlockBase + posicion);
}
