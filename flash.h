//----------------------------------------------------------------------
// Titel	:	Flash.h
//----------------------------------------------------------------------
// Sprache	:	C
// Datum	:	17.05.2025
// Version	:	1.0
// Autor	:	Diveturtle93
// Projekt	:	Bootloader
// Quelle	:	https://github.com/ferenc-nemeth/stm32-bootloader/blob/master/Inc/flash.h
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
	#define DEBUG_FLASH
#endif
//----------------------------------------------------------------------

// Definiere Page oder Sector
//----------------------------------------------------------------------
#if defined (STM32F7) || defined (STM32H7)
	#define FLASH_SECTOR
#endif

#if defined (STM32F1) || defined (STM32G0)
	#define FLASH_PAGE
#endif
//----------------------------------------------------------------------

// Konstanten definieren
//----------------------------------------------------------------------
// Flash default Startadresse definieren
//----------------------------------------------------------------------
#ifndef FLASH_APP_START_ADDRESS
#ifndef STM32H7
	#define FLASH_APP_START_ADDRESS			(0x8008000)						// Flash Startadresse fuer Application
#else
	#define FLASH_APP_START_ADDRESS			(0x8020000)						// Flash Startadresse fuer Application beim STM32H7
#endif
#endif
//----------------------------------------------------------------------
// Flash Validation Adresse definieren
// Darf bei Mikrocontrollern mit Sectoren nicht im selben Sector wie Bootloader liegen.
//----------------------------------------------------------------------
#ifndef FLASH_APP_VALID_ADDRESS
#ifndef STM32H7
	#define FLASH_APP_VALID_ADDRESS			(0x8007FF8)						// Flash Speicheradresse, wenn Application installed und valid
#else
	#define FLASH_APP_VALID_ADDRESS			(0x803FFE0)						// Flash Speicheradresse, wenn Application installed und valid beim STM32H7
#endif
#endif
//----------------------------------------------------------------------
// Flash default Endadresse definieren
//----------------------------------------------------------------------
#ifndef FLASH_APP_END_ADDRESS
	#define FLASH_APP_END_ADDRESS			0x8010000-1
#endif
//----------------------------------------------------------------------

// Flash Adressen fuer Sectoren definieren (STM32F767ZIT)
//----------------------------------------------------------------------
#ifdef STM32F7
	#define ADDR_FLASH_SECTOR_0_BANK1	0x8000000
	#define ADDR_FLASH_SECTOR_1_BANK1	0x8008000
	#define ADDR_FLASH_SECTOR_2_BANK1	0x8010000
	#define ADDR_FLASH_SECTOR_3_BANK1	0x8018000
	#define ADDR_FLASH_SECTOR_4_BANK1	0x8020000
	#define ADDR_FLASH_SECTOR_5_BANK1	0x8040000
	#define ADDR_FLASH_SECTOR_6_BANK1	0x8080000
	#define ADDR_FLASH_SECTOR_7_BANK1	0x80C0000
	#define ADDR_FLASH_SECTOR_8_BANK1	0x8100000
	#define ADDR_FLASH_SECTOR_9_BANK1	0x8140000
	#define ADDR_FLASH_SECTOR_10_BANK1	0x8180000
	#define ADDR_FLASH_SECTOR_11_BANK1	0x81C0000
#endif
//----------------------------------------------------------------------
// Flash Adressen fuer Sectoren definieren (STM32H743ZI)
//----------------------------------------------------------------------
#ifdef STM32H7
	#define ADDR_FLASH_SECTOR_0_BANK1	0x8000000
	#define ADDR_FLASH_SECTOR_1_BANK1	0x8020000
	#define ADDR_FLASH_SECTOR_2_BANK1	0x8040000
	#define ADDR_FLASH_SECTOR_3_BANK1	0x8060000
	#define ADDR_FLASH_SECTOR_4_BANK1	0x8080000
	#define ADDR_FLASH_SECTOR_5_BANK1	0x80A0000
	#define ADDR_FLASH_SECTOR_6_BANK1	0x80C0000
	#define ADDR_FLASH_SECTOR_7_BANK1	0x80E0000
#endif
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
flash_status flash_erase (uint32_t address);								// Flash loeschen
flash_status flash_write (uint32_t address, uint8_t *data, uint32_t length);// Flash schreiben
flash_status flash_validation (uint32_t address, uint8_t *data);			// Flash validieren
void flash_jump_to_app (void);												// Zu Application springen
uint32_t GetPage (uint32_t address);										// Page bekommen
uint32_t GetSector (uint32_t address);										// Sector bekommen
//----------------------------------------------------------------------

#endif /* INC_FLASH_H_ */
//----------------------------------------------------------------------
