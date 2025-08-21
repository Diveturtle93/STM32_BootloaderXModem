//----------------------------------------------------------------------
// Titel	:	XModem.h
//----------------------------------------------------------------------
// Sprache	:	C
// Datum	:	16.05.2025
// Version	:	1.0
// Autor	:	Diveturtle93
// Projekt	:	Bootloader XModem
// Quelle	:	https://github.com/ferenc-nemeth/stm32-bootloader/blob/master/Src/xmodem.h
//----------------------------------------------------------------------

// Dateiheader definieren
//----------------------------------------------------------------------
#ifndef INC_XMODEM_H_
#define INC_XMODEM_H_
//----------------------------------------------------------------------

// Einfuegen der standard Include-Dateien
//----------------------------------------------------------------------

//----------------------------------------------------------------------

// Einfuegen der STM Include-Dateien
//----------------------------------------------------------------------

//----------------------------------------------------------------------

// Konstanten definieren
//----------------------------------------------------------------------
#define X_MAX_ERROR							3								// Anzahl maximal zulaessiger Fehler

// Paket groessen definieren
//----------------------------------------------------------------------
#define X_PACKET_NUMBER_SIZE				2								// Paket groesse
#define X_PACKET_SIZE						128								// Paket 128 Byte
#define X_PACKET_1K_SIZE					1024							// Paket 1024 Byte
#define X_PACKET_CRC_SIZE					2								// CRC Paket groesse
//----------------------------------------------------------------------

// Indizes definieren
//----------------------------------------------------------------------
#define X_PACKET_NUMBER_INDEX				0								// Index
#define X_PACKET_NUMBER_COMPLEMENT_INDEX	1								// Index Komplementaer
#define X_PACKET_CRC_HIGH_INDEX				0								// CRC
#define X_PACKET_CRC_LOW_INDEX				1								// CRC Komplementaer
//----------------------------------------------------------------------

// Protokoll Befehle definieren
//----------------------------------------------------------------------
#define X_SOH								1								// Start of header (128 Bytes)
#define X_STX								2								// Start of header (1024 Bytes)
#define X_EOT								4								// End of Transmission
#define X_ACK								6								// Acknowledge
#define X_NAK								15								// Not Acknowledge
#define X_ETB								17								// End of Transmission
#define X_CAN								18								// Cancel
#define X_APP								65								// Abort
#define X_C									67								// ASCII "C" to notify the host, we want to use CRC16
//----------------------------------------------------------------------

// XModem Status definieren
//----------------------------------------------------------------------
typedef enum
{
	X_OK								= 0x00,								// Aktion erfolgreich
	X_ERROR_CRC							= 0x01,								// CRC Fehler
	X_ERROR_NUMBER						= 0x02,								// Paket Anzahl Fehlert
	X_ERROR_UART						= 0x04,								// Uart Kommunikationsfehler
	X_ERROR_FLASH						= 0x08,								// Flash Fehler
	X_ERROR_VALID						= 0x10,								// Application Validierung fehlgeschlagen
	X_ERROR								= 0xFF,								// Unbestimmter Fehler
} xmodem_status;
//----------------------------------------------------------------------

// Funktionen definieren
//----------------------------------------------------------------------
void xmodem_receive (void);
uint16_t xmodem_calc_crc (uint8_t *data, uint16_t length);
xmodem_status xmodem_handle_packet (uint8_t size);
xmodem_status xmodem_error_handler (uint8_t *error_number, uint8_t max_error_number);
xmodem_status app_validation (uint32_t address);
//----------------------------------------------------------------------

#endif /* INC_BASICUART_H_ */
//----------------------------------------------------------------------
