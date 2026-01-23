//----------------------------------------------------------------------
// Titel	:	flash.h
//----------------------------------------------------------------------
// Sprache	:	C
// Datum	:	17.05.2025
// Version	:	1.0
// Autor	:	Diveturtle93
// Projekt	:	Bootloader
// Quelle	:	https://github.com/ferenc-nemeth/stm32-bootloader/blob/master/Inc/flash.h
//----------------------------------------------------------------------

// Saveguard symbol
//----------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------

// Dateiheader definieren
//----------------------------------------------------------------------
#ifndef INC_FLASH_H_
#define INC_FLASH_H_
//----------------------------------------------------------------------

// Einfuegen der standard Include-Dateien
//----------------------------------------------------------------------

//----------------------------------------------------------------------

// Einfuegen der STM Include-Dateien
//----------------------------------------------------------------------
#include "main.h"
//----------------------------------------------------------------------

// Definiere Debug Symbols
//----------------------------------------------------------------------
#ifdef DEBUG
//	#define DEBUG_FLASH
#endif
//----------------------------------------------------------------------

// Typedefines definieren
//----------------------------------------------------------------------
typedef enum
{
	FLASH_OK				= 0x00,											// Flash OK
	X_FLASH_ERROR_SIZE		= 0x01,											// Flash Groesse fehlerhaft
	FLASH_ERROR_WRITE		= 0x02,											// Flash schreiben fehlerhaft
	FLASH_ERROR_READBACK	= 0x04,											// Flash zuruecklesen fehlerhaft
	FLASH_ERROR_VALID		= 0x08,											// Flash validieren fehlerhaft
	FLASH_ERROR				= 0xFF,											// Flash unbestimmter Fehler
} flash_status;
//----------------------------------------------------------------------

// Funktionen definieren
//----------------------------------------------------------------------
flash_status flash_erase (uint32_t address);								// Flash loeschen
flash_status flash_write (uint32_t address, uint8_t *data, uint32_t length);// Flash schreiben
flash_status flash_validation (uint32_t address, uint8_t *data);			// Flash validieren
void flash_jump_to_app (void);												// Zu Application springen
uint32_t GetPage (uint32_t address);										// Page bekommen
uint32_t GetSector (uint32_t address);										// Sector bekommen
//----------------------------------------------------------------------

#endif /* INC_FLASH_H_ */
//----------------------------------------------------------------------
