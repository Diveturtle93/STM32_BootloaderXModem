//----------------------------------------------------------------------
// Titel	:	Flash.h
//----------------------------------------------------------------------
// Sprache	:	C
// Datum	:	17.05.2025
// Version	:	1.0
// Autor	:	Diveturtle93
// Projekt	:	Bootloader
// Quelle	:	https://github.com/ferenc-nemeth/stm32-bootloader/blob/master/Src/flash.h
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
#include "stm32g0xx.h"
//----------------------------------------------------------------------

// Konstanten definieren
//----------------------------------------------------------------------
#define FLASH_APP_START_ADDRESS		(0x8008000)								// Flash Startadresse fuer Application
#define FLASH_APP_END_ADDRESS		(0x8010000 - 1)							// Am Ende noch etwas Platz lassen
#define FLASH_APP_VALID_ADDRESS		(0x8007FF8)								// Flash Speicheradresse, wenn Application installed und valid
//----------------------------------------------------------------------

// Typedefines definieren
//----------------------------------------------------------------------
typedef enum
{
	FLASH_OK				= 0x00,
	X_FLASH_ERROR_SIZE		= 0x01,
	FLASH_ERROR_WRITE		= 0x02,
	FLASH_ERROR_READBACK	= 0x04,
	FLASH_ERROR_VALID		= 0x08,
	FLASH_ERROR				= 0xFF,
} flash_status;
//----------------------------------------------------------------------

// Funktionen definieren
//----------------------------------------------------------------------
flash_status flash_erase (uint32_t address);
flash_status flash_write (uint32_t address, uint8_t *data, uint32_t length);
uint32_t GetPage (uint32_t address);
flash_status flash_validation (uint32_t address, uint8_t *data);
void flash_jump_to_app (void);
//----------------------------------------------------------------------

#endif /* INC_FLASH_H_ */
//----------------------------------------------------------------------
