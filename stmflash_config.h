//----------------------------------------------------------------------
// Titel	:	stmflash_config.h
//----------------------------------------------------------------------
// Sprache	:	C
// Datum	:	03.09.2025
// Version	:	1.0
// Autor	:	Diveturtle93
// Projekt	:	Bootloader
// Quelle	:
//----------------------------------------------------------------------

// Dateiheader definieren
//----------------------------------------------------------------------
#ifndef INC_STMFLASH_CONFIG_H_
#define INC_STMFLASH_CONFIG_H_
//----------------------------------------------------------------------

// Einfuegen der standard Include-Dateien
//----------------------------------------------------------------------

//----------------------------------------------------------------------

// Einfuegen der STM Include-Dateien
//----------------------------------------------------------------------
#include "main.h"
//----------------------------------------------------------------------

// Definiere Page oder Sector
//----------------------------------------------------------------------
#if defined (STM32F1) || defined (STM32G0)
	#define FLASH_PAGE
#endif

#if defined (STM32F7) || defined (STM32H7)
	#define FLASH_SECTOR
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
#if defined (STM32F1) || defined (STM32G0)
	#define FLASH_APP_VALID_ADDRESS			(0x8007FF8)						// Flash Speicheradresse, wenn Application installed und valid
#endif

#ifdef STM32F7
	#define FLASH_APP_VALID_ADDRESS			(0x800FFF8)						// Flash Speicheradresse, wenn Application installed und valid beim STM32F7
#endif

#ifdef STM32H7
	#define FLASH_APP_VALID_ADDRESS			(0x80DFFE0)						// Flash Speicheradresse, wenn Application installed und valid beim STM32H7
#endif
#endif
//----------------------------------------------------------------------
// Flash Konfiguration Adresse definieren
//----------------------------------------------------------------------
#ifndef FLASH_APP_CONFIG_ADDRESS
#ifdef STM32F1
	#define FLASH_APP_CONFIG_ADDRESS		(0x8038000)						// Flash Speicheradresse, wenn Application installed und valid beim STM32F1
#endif

#ifdef STM32F7
	#define FLASH_APP_CONFIG_ADDRESS		(0x8010000)						// Flash Speicheradresse, wenn Application installed und valid beim STM32F7
#endif

#ifdef STM32G0
	#define FLASH_APP_CONFIG_ADDRESS		(0x8018000)						// Flash Speicheradresse, wenn Application installed und valid beim STM32G0
#endif

#ifdef STM32H7
	#define FLASH_APP_CONFIG_ADDRESS		(0x80DFF00)						// Flash Speicheradresse, wenn Application installed und valid beim STM32H7
#endif
#endif
//----------------------------------------------------------------------
// Flash default Endadresse definieren
//----------------------------------------------------------------------
#ifndef FLASH_APP_END_ADDRESS
#ifdef STM32F1
	#define FLASH_APP_END_ADDRESS			0x8040000-1						// Flash Endadresse fuer Application beim STM32F1
#endif

#if defined (STM32F7) || defined (STM32H7)
	#define FLASH_APP_END_ADDRESS			0x80E0000-1						// Flash Endadresse fuer Application beim STM32F7 und STM32H7
#endif

#ifdef STM32G0
	#define FLASH_APP_END_ADDRESS			0x8020000-1						// Flash Endadresse fuer Application beim STM32G0
#endif
#endif
//----------------------------------------------------------------------

// Flash Adressen fuer Sectoren definieren (STM32F767ZIT)
//----------------------------------------------------------------------
#ifdef STM32F7
	#define ADDR_FLASH_SECTOR_0_BANK1		0x8000000
	#define ADDR_FLASH_SECTOR_1_BANK1		0x8008000
	#define ADDR_FLASH_SECTOR_2_BANK1		0x8010000
	#define ADDR_FLASH_SECTOR_3_BANK1		0x8018000
	#define ADDR_FLASH_SECTOR_4_BANK1		0x8020000
	#define ADDR_FLASH_SECTOR_5_BANK1		0x8040000
	#define ADDR_FLASH_SECTOR_6_BANK1		0x8080000
	#define ADDR_FLASH_SECTOR_7_BANK1		0x80C0000
	#define ADDR_FLASH_SECTOR_8_BANK1		0x8100000
	#define ADDR_FLASH_SECTOR_9_BANK1		0x8140000
	#define ADDR_FLASH_SECTOR_10_BANK1		0x8180000
	#define ADDR_FLASH_SECTOR_11_BANK1		0x81C0000
#endif
//----------------------------------------------------------------------
// Flash Adressen fuer Sectoren definieren (STM32H743ZI)
//----------------------------------------------------------------------
#ifdef STM32H7
	// Bank1
	#define ADDR_FLASH_SECTOR_0_BANK1		0x8000000
	#define ADDR_FLASH_SECTOR_1_BANK1		0x8020000
	#define ADDR_FLASH_SECTOR_2_BANK1		0x8040000
	#define ADDR_FLASH_SECTOR_3_BANK1		0x8060000
	#define ADDR_FLASH_SECTOR_4_BANK1		0x8080000
	#define ADDR_FLASH_SECTOR_5_BANK1		0x80A0000
	#define ADDR_FLASH_SECTOR_6_BANK1		0x80C0000
	#define ADDR_FLASH_SECTOR_7_BANK1		0x80E0000

	// Bank2
	#define ADDR_FLASH_SECTOR_0_BANK2		0x8100000
	#define ADDR_FLASH_SECTOR_1_BANK2		0x8120000
	#define ADDR_FLASH_SECTOR_2_BANK2		0x8140000
	#define ADDR_FLASH_SECTOR_3_BANK2		0x8160000
	#define ADDR_FLASH_SECTOR_4_BANK2		0x8180000
	#define ADDR_FLASH_SECTOR_5_BANK2		0x81A0000
	#define ADDR_FLASH_SECTOR_6_BANK2		0x81C0000
	#define ADDR_FLASH_SECTOR_7_BANK2		0x81E0000
#endif
//----------------------------------------------------------------------

#endif /* INC_STMFLASH_CONFIG_H_ */
//----------------------------------------------------------------------
