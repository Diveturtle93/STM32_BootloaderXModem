# STM32 Bootloader XModem
 
Ein in C implementierter UART-Bootloader für STM32-Mikrocontroller, der das
XModem-CRC-Protokoll zur Übertragung einer neuen Firmware verwendet. Mit einem
Terminal-Programm wie TeraTerm kann eine neue Application bequem über die
serielle Schnittstelle aufgespielt werden, ohne dass ein Programmer oder
Debugger angeschlossen sein muss.
 
Die Bibliothek basiert auf dem Projekt von [ferenc-nemeth/stm32-bootloader](https://github.com/ferenc-nemeth/stm32-bootloader)
und wurde für mehrere STM32-Serien und erweiterbare Flash-Konfiguration
weiterentwickelt.
 
## Beschreibung
 
Der Bootloader startet nach dem Reset und prüft, ob im Flash-Speicher bereits
eine gültige Application vorhanden ist. Wenn ja, wartet er ca. 100 Sekunden auf
einen Firmware-Upload und springt danach automatisch in die Application. Wird
keine gültige Application gefunden, wartet der Bootloader dauerhaft auf einen
Upload.
 
Die Übertragung erfolgt über XModem mit 16-Bit-CRC-Prüfung. Der Empfang ist
empfängergesteuert: Der STM32 sendet zunächst ein `C` an den PC, worauf der
Sender mit dem ersten 133-Byte-Paket antwortet. Jedes Paket wird mit ACK
bestätigt oder bei Fehler mit NAK abgelehnt und erneut angefordert. Nach dem
letzten Paket schreibt der Bootloader die Firmware in den Flash und springt in
die Application.
 
## Unterstützte Mikrocontroller
 
| Mikrocontroller | Startadresse App | Endadresse App | Bank  | Gesamtspeicher | Bootloader | Application |
|-----------------|------------------|----------------|-------|----------------|------------|-------------|
| STM32F105       | `0x08008000`     | `0x0803FFFF`   | Bank1 | 256 kB         | 32 kB      | 224 kB      |
| STM32F767       | `0x08008000`     | `0x0800FFFF`   | Bank1 | 2048 kB        | 32 kB      | 32 kB       |
| STM32G071       | `0x08008000`     | `0x0801FFFF`   | Bank1 | 128 kB         | 32 kB      | 96 kB       |
| STM32H743       | `0x08020000`     | `0x0803FFFF`   | Bank1 | 2048 kB        | 128 kB     | 128 kB      |
 
Es wird ausschließlich Bank 1 verwendet. Nicht jeder Mikrocontroller nutzt den
gesamten verfügbaren Speicher.
 
## Dateien
 
| Datei               | Beschreibung                                                                     |
|---------------------|----------------------------------------------------------------------------------|
| `xmodem.h`          | Protokollkonstanten, Steuerbytes, `xmodem_status`-Enum, Funktionsdeklarationen   |
| `xmodem.c`          | XModem-Empfangslogik, CRC-Berechnung, Paketvalidierung, Application-Start        |
| `flash.h`           | Flash-API für Löschen und Schreiben                                              |
| `flash.c`           | Flash-Implementierung (Page/Sector je nach STM32-Familie)                        |
| `stmflash_config.h` | Flash-Adressen, Speichergröße und Sector-Mapping für alle unterstützten Familien |
 
## XModem-Protokoll
 
### Paketformat
 
| Byte 1 | Byte 2        | Byte 3                  | Byte 4–131  | Byte 132–133 |
|--------|---------------|-------------------------|-------------|--------------|
| SOH    | Paketnummer   | Komplement Paketnummer  | 128 Byte Daten | 16-Bit CRC |
 
- Byte 1 darf nur `SOH`, `STX`, `EOT`, `CAN` oder `ETB` sein, andernfalls liegt
ein Fehler vor.
- Bytes 2 und 3 bilden eine Paketnummer mit Prüfsumme: die Summe beider Bytes
ergibt immer `0xFF`.
- Die Paketnummer beginnt bei 1 und rollt nach 255 auf 0 über.
- Der CRC-16 wird ausschließlich über die 128 Datenbytes (Bytes 4–131) berechnet,
mit dem Polynom `0x1021`.
### Steuerbytes
 
| Symbol | Wert   | Beschreibung                               |
|--------|--------|--------------------------------------------|
| `SOH`  | `0x01` | Start of Header (128-Byte-Paket)           |
| `STX`  | `0x02` | Start of Header (1024-Byte-Paket)          |
| `EOT`  | `0x04` | End of Transmission                        |
| `ACK`  | `0x06` | Acknowledge – Paket korrekt empfangen      |
| `NAK`  | `0x15` | Not Acknowledge – Paket wiederholen        |
| `ETB`  | `0x17` | End of Transmission Block                  |
| `CAN`  | `0x18` | Cancel – Übertragung abbrechen             |
| `C`    | `0x43` | Empfangsbereitschaft im CRC-Modus          |
 
### Fehlerbedingungen (führen zu NAK)
 
1. Framing-Fehler bei einem beliebigen Byte
2. Überlaufsfehler bei einem beliebigen Byte
3. Doppeltes Paket (Sender hat ein ACK nicht erhalten)
4. CRC-Fehler
5. Timeout – kein Paket innerhalb von 1 Sekunde empfangen
Nach maximal `X_MAX_ERROR` (Standard: 3) aufeinanderfolgenden Fehlern bricht der
Bootloader die Übertragung ab.
 
### Status-Enum `xmodem_status`
 
| Wert               | Beschreibung                          |
|--------------------|---------------------------------------|
| `X_OK`             | Aktion erfolgreich                    |
| `X_ERROR_CRC`      | CRC-Fehler im Paket                   |
| `X_ERROR_NUMBER`   | Fehler bei der Paketnummer            |
| `X_ERROR_UART`     | UART-Kommunikationsfehler             |
| `X_ERROR_FLASH`    | Fehler beim Schreiben in den Flash    |
| `X_ERROR_VALID`    | Application-Validierung fehlgeschlagen|
| `X_ERROR`          | Unbestimmter Fehler                   |
 
## Flash-Konfiguration
 
Die Adressen werden in `stmflash_config.h` automatisch anhand des definierten
Mikrocontroller-Symbols (`STM32F1`, `STM32F7`, `STM32G0`, `STM32H7`) gesetzt.
Alle vier Adressen können vor dem Einbinden der Bibliothek in `main.h` mit eigenen
Werten überschrieben werden:
 
```c
#define FLASH_APP_START_ADDRESS   0x08008000   // Startadresse der Application
#define FLASH_APP_END_ADDRESS     0x0803FFFF   // Endadresse der Application
#define FLASH_APP_VALID_ADDRESS   0x08007FF8   // Adresse des Gültigkeits-Flags
#define FLASH_APP_CONFIG_ADDRESS  0x08038000   // Adresse des Konfigurationsbereichs
```
 
`FLASH_APP_VALID_ADDRESS` darf bei Mikrocontrollern mit Sektoren (F7, H7)
**nicht imselben Sektor** wie der Bootloader liegen, da der gesamte Sektor vor dem
Schreiben gelöscht wird.
 
## Verwendung
 
### 1. Bootloader-Projekt einrichten
 
Alle Dateien (`xmodem.h`, `xmodem.c`, `flash.h`, `flash.c`, `stmflash_config.h`) in
das STM32-Projekt einbinden und den Header in `main.c` einbinden:
 
```c
#include "xmodem.h"
```
 
### 2. Bootloader aufrufen
 
In der Hauptschleife `xmodem_receive()` aufrufen:
 
```c
while (1)
{
    xmodem_receive();
}
```
 
`xmodem_receive()` prüft beim ersten Aufruf, ob eine gültige Application im Flash liegt:
- **Application vorhanden:** ca. 100 Sekunden auf Upload warten, danach automatisch in
die Application springen.
- **Keine Application:** dauerhaft auf Upload warten.

### 3. Application anpassen – Linker-Skript
 
In der Datei `STM32xxxxx_FLASH.ld` muss die Flash-Startadresse der Application auf den
Wert von `FLASH_APP_START_ADDRESS` angepasst werden. Der Standardwert `0x8000000` muss
ersetzt werden. Der neue Wert muss ein Vielfaches von `0x200` sein:
 
```
FLASH (rx) : ORIGIN = 0x8008000, LENGTH = 96K
```
 
Die Länge ergibt sich aus dem verfügbaren Speicher ab der neuen Startadresse.
 
### 4. Application anpassen – Vektortabelle
 
In `system_stm32xxxxx.c` muss die Vektortabelle auf die neue Startadresse verschoben werden.
Dazu die Kommentierung der folgenden Zeile aufheben:
 
```c
#define USER_VECT_TAB_ADDRESS
```
 
Und direkt darunter das Offset auf die Startadresse der Application setzen. Für eine
Startadresse von `0x08008000`:
 
```c
#define VECT_TAB_OFFSET  0x00008000U
```
 
### 5. Firmware über TeraTerm übertragen
 
1. TeraTerm öffnen und mit dem richtigen COM-Port verbinden (gleiche Baudrate wie im
Bootloader konfiguriert).
2. Menü: **File → Transfer → XMODEM → Send**
3. Die `.bin`-Datei der Application auswählen.
4. Der Bootloader empfängt die Firmware, schreibt sie in den Flash und springt danach
automatisch in die neue Application.

## Quellen
 
Diese Bibliothek basiert auf:
- [ferenc-nemeth/stm32-bootloader](https://github.com/ferenc-nemeth/stm32-bootloader)

## Abhängigkeiten
 
- `main.h` – STM32 HAL
- [`basicuart.h`](https://github.com/Diveturtle93/STM32_Basicuart) – UART-Sende- und Empfangsfunktionen
- `millis.h` – Zeitbasis für den Application-Timeout

## Lizenz
 
Dieses Projekt steht unter der [GPL-3.0 Lizenz](LICENSE).
 
