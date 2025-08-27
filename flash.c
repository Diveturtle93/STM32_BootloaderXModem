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

#ifdef DEBUG_FLASH
	#include "BasicUart.h"
#endif
//----------------------------------------------------------------------

// Definiere Typedefines
//----------------------------------------------------------------------
typedef void (*fnc_ptr)(void);
//----------------------------------------------------------------------

// Flash loeschen
//----------------------------------------------------------------------
flash_status flash_erase (uint32_t address)
{
	// Debug
#ifdef DEBUG_FLASH
	uartTransmit("Flash unlock\n", 13);
	uartTransmit("Flash loeschen starten\n", 23);
#endif

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

#if define (STM32F7) || define (STM32H7)
	// Flash Erase definieren
	erase_init.TypeErase = FLASH_TYPEERASE_SECTORS;

	// Kalkuliere die Anzahl der Sectoren von der Startadresse bis Ende
	erase_init.Sector = GetSector(address);

	// Kalkuliere die Anzahl der Sectoren von der Startadresse bis Ende
	erase_init.NbSectors = 1;

	// Flash VoltageRange setzen
	erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_3;
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

	// Debug
#ifdef DEBUG_FLASH
	uartTransmit("Flash lock\n", 11);
	uartTransmit("Flash loeschen beendet\n", 23);
#endif

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

	// Debug
#ifdef DEBUG_FLASH
	uartTransmit("Flash unlock\n", 13);
	uartTransmit("Flash schreiben starten\n", 24);
#endif

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
			// Umsortierung von 8 Bit auf 64 Bit fuer STM32F1 und G0
			// Umsortierung von 8 Bit auf 32 Bit fue STM32F7
			// Adresse der Daten uebergeben
#if defined (STM32F1) || defined (STM32G0)
			// Fuer STM32F1 und G0 nur 64 Bit daher beginnend bei Byte 7
			tmp = (((uint64_t)(data[7 + (i*8)])) << 56);
			tmp += (((uint64_t)(data[6 + (i*8)])) << 48);
			tmp += (((uint64_t)(data[5 + (i*8)])) << 40);
			tmp += (((uint64_t)(data[4 + (i*8)])) << 32);
			tmp += (((uint64_t)(data[3 + (i*8)])) << 24);
#endif

#ifdef STM32F7
			// Fuer STM32F7 nur 32 Bit daher beginnend bei Byte 3
			tmp = (((uint64_t)(data[3 + (i*4)])) << 24);
#endif

#if defined (STM32F1) || defined (STM32F7) || defined (STM32G0)
			// Restlichen Byte 2 - 0
			tmp += (((uint64_t)(data[2 + (i*8)])) << 16);
			tmp += (((uint64_t)(data[1 + (i*8)])) << 8);
			tmp += (((uint64_t)(data[0 + (i*8)])) << 0);
#endif

#ifdef STM32H7
			// Zeiger auf Speicheradresse der Daten
			tmp = (uint32_t)&data[i*32];
#endif
			
			// Flashen, falls Fehler wird dieser ausgegeben
#if defined (STM32F1) || defined (STM32G0)
			// Doubleword 64Bit fuer STM32G071 notwendig
			if (HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, tmp))
#endif

#ifdef STM32F7
			// STM32F7 wird mit 32Bit geflashed
			if (HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, tmp))
#endif

#ifdef STM32H7
			// STM32H7 wird mit 32 Byte geflashed
			if (HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address, tmp))
#endif
			{
				status |= FLASH_ERROR_WRITE;								// Fehler wenn Flashen nicht OK
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
#if defined (STM32F1) || defined (STM32G0)
			address += 8;													// Fuer STM32F1 und STM32G0
#endif

#ifdef STM32F7
			address += 4;													// Fuer STM32F7
#endif

#ifdef STM32H7
			address += 32;													// Fuer STM32H7
#endif
		}
	}

	// Flash sperren
	HAL_FLASH_Lock();

	// Debug
#ifdef DEBUG_FLASH
	uartTransmit("Flash lock\n", 11);
	uartTransmit("Flash schreiben beenden\n", 24);
#endif

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

	// Debug
#ifdef DEBUG_FLASH
	uartTransmit("Flash validieren starten\n", 25);
#endif

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

	// Debug
#ifdef DEBUG_FLASH
	uartTransmit("Flash validieren beendet\n", 25);
#endif

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

// Flash Page ermitteln
//----------------------------------------------------------------------
#ifndef FLASH_PAGE
uint32_t GetPage (uint32_t address)
{
	// Debug
#ifdef DEBUG_FLASH
	uartTransmit("Page ermittlen\n", 15);
#endif

	return (address - FLASH_BASE) / FLASH_PAGE_SIZE;
}
#endif
//----------------------------------------------------------------------

// Flash Sector ermitteln
//----------------------------------------------------------------------
#ifdef FLASH_SECTOR
uint32_t GetSector(uint32_t address)
{
	// Debug
#ifdef DEBUG_FLASH
	uartTransmit("Sector ermittlen\n", 17);
#endif

    if (address < ADDR_FLASH_SECTOR_0_BANK1) return FLASH_SECTOR_0;
    else if (address < ADDR_FLASH_SECTOR_1_BANK1) return FLASH_SECTOR_1;
    else if (address < ADDR_FLASH_SECTOR_2_BANK1) return FLASH_SECTOR_2;
    else if (address < ADDR_FLASH_SECTOR_3_BANK1) return FLASH_SECTOR_3;
    else if (address < ADDR_FLASH_SECTOR_4_BANK1) return FLASH_SECTOR_4;
    else if (address < ADDR_FLASH_SECTOR_5_BANK1) return FLASH_SECTOR_5;
    else if (address < ADDR_FLASH_SECTOR_6_BANK1) return FLASH_SECTOR_6;
	
#ifdef STM32H7
	else return FLASH_SECTOR_7;
#endif
	
#ifdef STM32F7
    else if (address < ADDR_FLASH_SECTOR_7_BANK1) return FLASH_SECTOR_7;
    else if (address < ADDR_FLASH_SECTOR_8_BANK1) return FLASH_SECTOR_8;
    else if (address < ADDR_FLASH_SECTOR_9_BANK1) return FLASH_SECTOR_9;
    else if (address < ADDR_FLASH_SECTOR_10_BANK1) return FLASH_SECTOR_10;
    else return FLASH_SECTOR_11;
#endif
}
#endif
//----------------------------------------------------------------------