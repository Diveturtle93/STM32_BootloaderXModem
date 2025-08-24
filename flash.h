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

// Konstanten definieren
//----------------------------------------------------------------------
#define FLASH_APP_START_ADDRESS				(0x8008000)						// Flash Startadresse fuer Application
#define FLASH_APP_VALID_ADDRESS				(0x8007FF8)						// Flash Speicheradresse, wenn Application installed und valid
//----------------------------------------------------------------------

// Flash default Endadresse definieren
//----------------------------------------------------------------------
#ifndef FLASH_APP_END_ADDRESS
	#define FLASH_APP_END_ADDRESS			FLASH_END
/*	#ifdef STM32F1
		#define FLASH_APP_END_ADDRESS		(0x803FFFF)						// Flash Endadresse Festlegen fuer STM32F105
	#endif

	#ifdef STM32F7
		#define FLASH_APP_END_ADDRESS		(0x81FFFFF)						// Flash Endadresse Festlegen fuer STM32F105
	#endif

	#ifdef STM32G0
		#define FLASH_APP_END_ADDRESS		(0x801FFFF)						// Flash Endadresse Festlegen fuer STM32F105
	#endif*/
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
uint32_t GetPage (uint32_t address);										// Pageanzahl bekommen
//----------------------------------------------------------------------

#endif /* INC_FLASH_H_ */
//----------------------------------------------------------------------
