//----------------------------------------------------------------------
// Titel	:	flash.c
//----------------------------------------------------------------------
// Sprache	:	C
// Datum	:	17.05.2025
// Version	:	1.0
// Autor	:	Diveturtle93
// Projekt	:	Bootloader
// Quelle	:	https://github.com/ferenc-nemeth/stm32-bootloader/blob/master/Src/flash.c
//----------------------------------------------------------------------

// Einfuegen der standard Include-Dateien
//----------------------------------------------------------------------

//----------------------------------------------------------------------

// Einfuegen der STM Include-Dateien
//----------------------------------------------------------------------

//----------------------------------------------------------------------

// Einfuegen der eigenen Include Dateien
//----------------------------------------------------------------------
#include "flash.h"
//----------------------------------------------------------------------

// Definiere Typedefines
//----------------------------------------------------------------------
typedef void (*fnc_ptr)(void);
//----------------------------------------------------------------------

// Flash loeschen
//----------------------------------------------------------------------
flash_status flash_erase (uint32_t address)
{
	// Flash entsperren
	HAL_FLASH_Unlock();
	
	// Variablen definieren
	flash_status status = FLASH_ERROR;
	FLASH_EraseInitTypeDef erase_init;
	uint32_t error = 0;
	
	// Flash initialisieren
	erase_init.Banks = FLASH_BANK_1;

#if defined (STM32F1) || defined (STM32G0)
	// Flash Erase definieren
	erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
#endif

#ifdef STM32F1
	// Flash Page ermitteln
	erase_init.PageAddress = address;
	
	// Kalkuliere die Anzahl der Pages von der Startadresse bis zum Ende
	erase_init.NbPages = GetPage(FLASH_APP_END_ADDRESS) - GetPage(address);
#endif

#ifdef STM32F7
	// Flash Erase definieren
	erase_init.TypeErase = FLASH_TYPEERASE_SECTORS;

	// Kalkuliere die Anzahl der Sectoren von der Startadresse bis Ende
	erase_init.Sector = FLASH_SECTOR_1;

	// Kalkuliere die Anzahl der Sectoren von der Startadresse bis Ende
	erase_init.NbSectors = FLASH_SECTOR_11;

	// Flash VoltageRange setzen
	erase_init.VoltageRange = VOLTAGE_RANGE_3;
#endif

#ifdef STM32G0
	// Flash Page ermitteln
	erase_init.Page = GetPage(address);
	
	// Kalkuliere die Anzahl der Pages von der Startadresse bis zum Ende
	erase_init.NbPages = GetPage(FLASH_APP_END_ADDRESS) - erase_init.Page + 1;
#endif

	// Loesche Flash
	if (HAL_OK == HAL_FLASHEx_Erase(&erase_init, &error))
	{
		status = FLASH_OK;
	}

	// Flash sperren
	HAL_FLASH_Lock();

	return status;
}
//----------------------------------------------------------------------

// Flash schreiben
//----------------------------------------------------------------------
flash_status flash_write (uint32_t address, uint8_t *data, uint32_t length)
{
	// Variablen definieren
	flash_status status = FLASH_OK;
	uint64_t tmp;

	// Flash entsperren
	HAL_FLASH_Unlock();

	// Schleife fuer Array zum schreiben
	for (uint32_t i = 0; (i < length) && (FLASH_OK == status); i++)
	{
		// Wenn Ende des Speichers erreicht, dann Fehler
		if (FLASH_APP_END_ADDRESS <= address)
		{
			status |= X_FLASH_ERROR_SIZE;
		}
		else
		{
			// Umsortierung von 8 Bit auf 64 Bit
			tmp = (((uint64_t)(data[7 + (i*8)])) << 56);
			tmp += (((uint64_t)(data[6 + (i*8)])) << 48);
			tmp += (((uint64_t)(data[5 + (i*8)])) << 40);
			tmp += (((uint64_t)(data[4 + (i*8)])) << 32);
			tmp += (((uint64_t)(data[3 + (i*8)])) << 24);
			tmp += (((uint64_t)(data[2 + (i*8)])) << 16);
			tmp += (((uint64_t)(data[1 + (i*8)])) << 8);
			tmp += (((uint64_t)(data[0 + (i*8)])) << 0);

			// Flashen, falls Fehler wird dieser ausgegeben
			if (HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, tmp))
			{
				status |= FLASH_ERROR_WRITE;
			}

			// Zuruecklesen des Speicherinhaltes, wenn falsch dann Fehler
			for (uint8_t j = 0; j < 2; j++)
			{
				// Flash validieren, wenn nicht OK, dann fehlerhaft
				if (FLASH_OK != flash_validation(address + (j*4), &data[0 + (i*8) + (j*4)]))
				{
					status |= FLASH_ERROR_READBACK;
				}
			}

			// Addresse verschieben
			address += 8;
		}
	}

	// Flash sperren
	HAL_FLASH_Lock();

	return status;
}
//----------------------------------------------------------------------

// Flash Adress Inhalt validieren
//----------------------------------------------------------------------
flash_status flash_validation (uint32_t address, uint8_t *data)
{
	// Variablen definieren
	flash_status status = FLASH_OK;
	uint32_t tmp = 0, temp = 0;

	// Umsortierung von 8 Bit auf 32 Bit
	tmp = (((uint32_t)(data[3])) << 24);
	tmp += (((uint32_t)(data[2])) << 16);
	tmp += (((uint32_t)(data[1])) << 8);
	tmp += (((uint32_t)(data[0])) << 0);

	// Auslesen Speicheradresse
	temp = (*(volatile uint32_t*)address);

	// Vergleich der Daten mit Speicherinhalt, wenn ungleich dann fehlerhaft
	if (tmp != temp)
	{
		status = FLASH_ERROR_VALID;
	}

	return status;
}
//----------------------------------------------------------------------

// In Application springen
//----------------------------------------------------------------------
void flash_jump_to_app (void)
{
	fnc_ptr jump_to_app;
	jump_to_app = (fnc_ptr)(*((volatile uint32_t*) (FLASH_APP_START_ADDRESS + 4)));

	HAL_DeInit();

	// Aendere den Mainstack Pointer
	__set_MSP(*(volatile uint32_t*)FLASH_APP_START_ADDRESS);
	jump_to_app();
}
//----------------------------------------------------------------------

// Flash Adresse ermitteln
//----------------------------------------------------------------------
#ifndef STM32F7
uint32_t GetPage (uint32_t address)
{
	return (address - FLASH_BASE) / FLASH_PAGE_SIZE;
}
#endif
//----------------------------------------------------------------------
