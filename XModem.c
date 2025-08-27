//----------------------------------------------------------------------
// Titel	:	XModem.c
//----------------------------------------------------------------------
// Sprache	:	C
// Datum	:	16.05.2025
// Version	:	1.0
// Autor	:	Diveturtle93
// Projekt	:	Bootloader XModem
// Quelle	:	https://github.com/ferenc-nemeth/stm32-bootloader/blob/master/Src/xmodem.c
//				http://ee6115.mit.edu/amulet/xmodem.htm
//----------------------------------------------------------------------

// Einfuegen der standard Include-Dateien
//----------------------------------------------------------------------

//----------------------------------------------------------------------

// Einfuegen der STM Include-Dateien
//----------------------------------------------------------------------
#include "main.h"
//----------------------------------------------------------------------

// Einfuegen der eigenen Include Dateien
//----------------------------------------------------------------------
#include "XModem.h"
#include "BasicUart.h"
#include "flash.h"
//----------------------------------------------------------------------

// Variablen definieren
//----------------------------------------------------------------------
static uint8_t xmodem_packet_number = 1;									// Paketnummer Zaehler
static uint32_t xmodem_actual_flash_address = 0;							// Flash Addresse wo geschrieben wird
static uint8_t x_first_packet_received = false;								// Erstes Paket erkannt
uint8_t app_valid_array[8] = {0x62, 0x82, 0x12, 0x32, 0x52, 0x72, 0x92, 0x22};
uint8_t app_error_array[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
//----------------------------------------------------------------------

// XModem Receive
//----------------------------------------------------------------------
void xmodem_receive (void)
{
	// Variablen definieren
	volatile xmodem_status status = X_OK;
	uint8_t error_number = 0;
	uint8_t xmodem_count = 0;
	uint8_t app_valid = false;

	// Variablen initialisieren
	x_first_packet_received = false;
	xmodem_packet_number = 1;
	xmodem_actual_flash_address = FLASH_APP_START_ADDRESS;

	// Application valid
	app_valid = app_validation(FLASH_APP_VALID_ADDRESS);
	// Wenn Application Fehlerhaft oder nicht installiert
	if (X_ERROR_VALID == app_valid)
	{
		// Keine App vorhanden
		app_valid = false;
	}
	else
	{
		// Application vorhanden
		app_valid = true;
	}

	// Schleife bis Fehler auftritt oder in Application gesprungen wird
	while (X_OK == status)
	{
		// Variablen definieren
		uint8_t header = 0;

		// Empfange Header von UART
		uart_status comm_status = uartReceive(&header, 1);

		// Spam den Host mit ASCII "C" bis was empfangen wird, um mitzuteilen, das wir CRC16 nutzen wollen
		if ((UART_OK != comm_status) && (false == x_first_packet_received))
		{
			uartTransmitChar(X_C);

			// Wenn Application valid
			if (app_valid == true)
			{
				xmodem_count++;

				// Wenn Zaehler groesser 100
				if (xmodem_count >= 100)
				{
					// App ausfuehren
					header = X_APP;
				}
			}
		}
		// UART Timeout oder anderer Fehler
		else if ((UART_OK != comm_status) && (true == x_first_packet_received))
		{
			status = xmodem_error_handler(&error_number, X_MAX_ERROR);
		}
		// Alles andere
		else
		{
			// Tue nichts
		}

		xmodem_status packet_status = X_ERROR;

		// Der Header kann sein: SOH, STX, EOT und CAN
		switch(header)
		{
			// 128 oder 1024 Bytes
			case X_SOH:
			case X_STX:
			{
				// Wenn alles OK
				packet_status = xmodem_handle_packet(header);
				if (X_OK == packet_status)
				{
					uartTransmitChar(X_ACK);
				}
				// Wenn der Fehler vom Flash kommt, sofort alles abbrechen
				else if (X_ERROR_FLASH == packet_status)
				{
					error_number = X_MAX_ERROR;
					status = xmodem_error_handler(&error_number, X_MAX_ERROR);
				}
				// Error waehrend dem Empfangen
				else
				{
					status = xmodem_error_handler(&error_number, X_MAX_ERROR);
				}
				break;
			}

			// Ende der Uebertragung
			case X_EOT:
			case X_ETB:
			{
				// Uebertragung beenden
				uartTransmitChar(X_ACK);
				
				// Flash valid Speicher beschreiben
				flash_write(FLASH_APP_VALID_ADDRESS, &app_valid_array[0], 1);
				
				// Zu Application springen
				uartTransmit("\nFirmware geupdated!\n", 21);
				uartTransmit("Springe zu Benutzer Application...\n", 35);
				flash_jump_to_app();
				break;
			}

			// Abbruch vom Host
			case X_CAN:
			{
				// Übertragung abbrechen
				uartTransmitChar(X_CAN);
				uartTransmitChar(X_CAN);
				status = X_ERROR;
				break;
			}

			// Springe zu vorhandener Application nach Timeout
			case X_APP:
			{
				// Zu Application springen
				uartTransmitChar(X_CAN);
				uartTransmit("\nApplication vorhanden.\n", 24);
				uartTransmit("Springe zu Benutzer Application...\n", 35);
				flash_jump_to_app();
				break;
			}

			// Default
			default:
			{
				// Falsche Header
				if (UART_OK == comm_status)
				{
					status = xmodem_error_handler(&error_number, X_MAX_ERROR);
				}
				break;
			}
		}
	}
}
//----------------------------------------------------------------------

// CRC Berechnung
//----------------------------------------------------------------------
uint16_t xmodem_calc_crc (uint8_t *data, uint16_t length)
{
	// Variablen definieren
	uint16_t crc = 0;

	// Daten CRC berechnen
	while (length)
	{
		length--;

		crc = crc ^ ((uint16_t)*data++ << 8);
		for (uint8_t i = 0; i < 8; i++)
		{
			if (crc & 0x8000)
			{
				crc = (crc << 1) ^ 0x1021;
			}
			else
			{
				crc = crc << 1;
			}
		}
	}
	
	// Rueckgabe CRC
	return crc;
}
//----------------------------------------------------------------------

// XModem Paket Handler
//----------------------------------------------------------------------
xmodem_status xmodem_handle_packet (uint8_t header)
{
	// Variablen definieren
	xmodem_status status = X_OK;
	uint16_t size = 0;

	// 2 Bytes fuer Paketnummer, 1024 fuer Daten und 2 Byte fuer CRC
	uint8_t receive_packet_number[X_PACKET_NUMBER_SIZE];
	uint8_t receive_packet_data[X_PACKET_1K_SIZE];
	uint8_t receive_packet_crc[X_PACKET_CRC_SIZE];

	// Header
	if (X_SOH == header)
	{
		size = X_PACKET_SIZE;
	}
	else if (X_STX == header)
	{
		size = X_PACKET_1K_SIZE;
	}
	else
	{
		// Falscher Header
		status |= X_ERROR;
	}

	uart_status comm_status = UART_OK;

	// Datennummer, Daten und CRC empfangen
	comm_status = uartReceive(&receive_packet_number[0], X_PACKET_NUMBER_SIZE);
	comm_status = uartReceive(&receive_packet_data[0], size);
	comm_status = uartReceive(&receive_packet_crc[0], X_PACKET_CRC_SIZE);

	// Zusammenfuehren der beiden CRC Bytes
	uint16_t crc_received = ((uint16_t)receive_packet_crc[X_PACKET_CRC_HIGH_INDEX] << 8) | ((uint16_t)receive_packet_crc[X_PACKET_CRC_LOW_INDEX]);
	// CRC kalkulieren
	uint16_t crc_calculated = xmodem_calc_crc(&receive_packet_data[0], size);

	// Kommunikationsfehler
	if (UART_OK != comm_status)
	{
		status |= X_ERROR_UART;
	}

	// Wenn dies das erste Datenpaket ist, dann Flash loeschen
	if ((X_OK == status) && (false == x_first_packet_received))
	{
		// Wenn Flash loeschen OK
		if (FLASH_OK == flash_erase(FLASH_APP_START_ADDRESS))
		{
			x_first_packet_received = true;
		}
		else
		{
			// Wenn Loeschen fehlerhaft, dann FLASH Validation Speicher zuruecksetzen
			flash_write(FLASH_APP_VALID_ADDRESS, &app_error_array[0], 1);
			status |= X_ERROR_FLASH;
		}
	}

	// Error Handling und flashen
	if (X_OK == status)
	{
		if (xmodem_packet_number != receive_packet_number[0])
		{
			// Paketnummer fehlerhaft
			status |= X_ERROR_NUMBER;
		}

		if (255 != (receive_packet_number[X_PACKET_NUMBER_INDEX] + receive_packet_number[X_PACKET_NUMBER_COMPLEMENT_INDEX]))
		{
			// Wenn die Summe der Paketnummer und ihrem Komplement nicht 255 ist
			// Die Summe muss immer 255 sein
			status |= X_ERROR_NUMBER;
		}

		if (crc_calculated != crc_received)
		{
			// Fehler in CRC Daten
			status |= X_ERROR_CRC;
		}
	}

	// Flashen wenn keine Fehler aufgetreten sind
	if ((X_OK == status) && (FLASH_OK != flash_write(xmodem_actual_flash_address, &receive_packet_data[0], (uint32_t)size/8)))
	{
		// Wenn schreiben fehlerhaft, dann FLASH Validation Speicher zuruecksetzen
		flash_write(FLASH_APP_VALID_ADDRESS, &app_error_array[0], 1);
		// Fehler waehrend flashen
		status |= X_ERROR_FLASH;
	}

	// Erhoehe die Paketnummer und Flashadresse
	if (X_OK == status)
	{
		xmodem_packet_number++;
		xmodem_actual_flash_address += size;
	}
	
	// Rueckgabe Fehlerstatus
	return status;
}
//----------------------------------------------------------------------

// XModem Error Handler
//----------------------------------------------------------------------
xmodem_status xmodem_error_handler (uint8_t *error_number, uint8_t max_error_number)
{
	// Fehler status setzen
	xmodem_status status = X_OK;

	// Error Counter hochzaehlen
	(*error_number)++;

	// Wenn der Zaehler das maximum erreicht
	if ((*error_number) >= max_error_number)
	{
		// Abbruch
		uartTransmitChar(X_CAN);
		uartTransmitChar(X_CAN);

		status = X_ERROR;
	}
	// Ansonsten sende NAK fuer widerholen
	else
	{
		uartTransmitChar(X_NAK);
		status = X_OK;
	}
	
	// Rueckgabe Fehlerstatus
	return status;
}
//----------------------------------------------------------------------

// Application Validierung
//----------------------------------------------------------------------
xmodem_status app_validation (uint32_t address)
{
	// Fehlerstatus setzen
	xmodem_status status= X_OK;

	// Application validieren ersten 4 Byte
	if (FLASH_OK != flash_validation(address, &app_valid_array[0]))
	{
		status = X_ERROR_VALID;
	}

	// Application validieren zweiten 4 Byte
	if (FLASH_OK != flash_validation(address + 4, &app_valid_array[4]))
	{
		status = X_ERROR_VALID;
	}
	
	// Rueckgabe Fehlerstatus
	return status;
}
//----------------------------------------------------------------------
