# STM32 Bootloader XModem

Der Bootloader verwendet das XModem Protocoll zur Übertragung des neuen Application Quellcodes.
Dafür wird auf einem UART die serielle Schnittstelle zur Verfügung gestellt. Mit einem Programm,
welches den XModem-Transfer unterstützt, z.B. TeraTerm, kann dann eine neue Application auf
den Mikrocontroller aufgespielt werden.

## Aktuell unterstützt

Folgende Mikrocontroller werden vom Bootloader unterstützt, dabei wird nur Bank1 verwendet:

| µC | Startadresse | Endadresse | Bank | Speicher |
|:-- |:------------ |:---------- |:---- |:--------:|
| F105 | 0x08008000 | 0x0803FFFF | Bank1 | 256kB |
| G071 | 0x08008000 | 0x0801FFFF | Bank1 | 128kB |

Standardmäßig ist werden die Start- und Endadresse im Programm gesetzt. Diese Werte können aber
jeweils im eigenen Programmcode vordefiniert werden. Dabei können dann drei defines gesetzt werden.

	- FLASH_APP_START_ADDRESS	: Gibt die Startadresse der Application an
	- FLASH_APP_END_ADDRESS		: Setzt die Endadresse der Application
	- FLASH_APP_VALID_ADDRESS	: Wird benötigt, um eine vorhanden App zu erkennen


## Application

Bei der Application muss dann in der STM32xxx_Flash.ld noch folgende Anpassung geamacht werden.

```C
MEMORY
{
  RAM		(xrw)	: ORIGIN = 0x20000000,	LENGTH = 36K
  FLASH		(rx)	: ORIGIN = 0x8008000,	LENGTH = 96K
}
```

Die Länge hängt hierbei von der Größe des Speichers sowie der Startadresse ab. Unterschiedliche
Mikrocontroller haben eine unterschiedliche Größe. Je nach Startadresse wird der noch zur
Verfügung stehende Speicher ebenfalls kleiner.

Eine weitere Anpassung ist in der system_stm32xxx.c notwendig. Hier muss die Vector-Tabelle
angepasst werden. Zudem muss sie aktiviert sein. Dafür wird nach dem Kommentar

```C
#define USER_VECT_TAB_ADDRESS
```

gesucht und die Kommentierung aufgehoben. Direkt darunter muss dann noch das Offset festgelegt
werden. Das Offset hängt von der Startadresse ab.

```C
#define VECT_TAB_OFFSET         0x00008000U
```

---

## Einführung

Das Xmodem-Protokoll wurde vor Jahren entwickelt, um die Kommunikation zwischen zwei Computern
zu vereinfachen. Dank seines Halbduplex-Betriebs, 128-Byte-Paketen, ACK/NACK-Antworten und
CRC-Datenprüfung hat das Xmodem-Protokoll Einzug in viele Anwendungen gehalten. Tatsächlich
verfügen die meisten modernen PC-Kommunikationspakete über ein Xmodem-Protokoll.


## Funktionstheorie

Xmodem ist ein Halbduplex-Kommunikationsprotokoll. Der Empfänger bestätigt nach dem Empfang eines
Pakets dieses entweder (ACK) oder nicht (NAK). Die CRC-Erweiterung des ursprünglichen Protokolls
verwendet einen robusteren 16-Bit-CRC zur Validierung des Datenblocks und wird hier verwendet.
Xmodem ist empfängergesteuert. Das heißt, der Empfänger sendet ein „C“ an den Sender, um seine
Empfangsbereitschaft im CRC-Modus anzuzeigen. Der Sender sendet dann ein 133 Byte langes Paket.
Der Empfänger validiert es und antwortet mit einem ACK oder NAK. Anschließend sendet der Sender
entweder das nächste Paket oder das letzte Paket erneut. Dieser Vorgang wird fortgesetzt, bis der
Empfänger ein EOT empfängt und es dem Sender ordnungsgemäß bestätigt. Nach dem ersten Handshake
steuert der Empfänger den Datenfluss durch ACK und NAK an den Sender.

| Byte 1 | Byte 2 | Byte 3 | Byte 4 - 131 | Byte 132 - 133 |
|:------ |:------ |:------ |:------------ |:-------------- |
| Start of Header | Packet Number | (Packet Number) | Packet Data | 16-bit CRC |

Tabelle 1. XmodemCRC Paketformat

## Definitions

Die folgenden Definitionen werden für die Protokollflusssteuerung verwendet.

| Symbol | Description | Value |
|:------ |:----------- |:-----:|
| SOH | Start of Header | 0x01 |
| EOT | End of Transmission | 0x04 |
| ACK | Acknowledge | 0x06 |
| NAK | Not Acknowledge | 0x15 |
| ETB | End of Transmission Block (Return to Amulet OS mode) | 0x17 |
| CAN | Cancel (Force receiver to start sending C's) | 0x18 |
| C | ASCII “C” | 0x43 |

Byte 1 des XmodemCRC-Pakets kann nur die Werte SOH, EOT, CAN oder ETB annehmen. Alles andere ist
ein Fehler. Bytes 2 und 3 bilden eine Paketnummer mit Prüfsumme. Addiert man die beiden Bytes, ergibt
sich immer 0xff. Bitte beachten Sie, dass die Paketnummer bei 1 beginnt und auf 0 zurückgesetzt wird,
wenn mehr als 255 Pakete empfangen werden. Die Bytes 4–131 bilden das Datenpaket und können beliebig
sein. Die Bytes 132 und 133 bilden den 16-Bit-CRC. Das High-Byte des CRC befindet sich in Byte 132.
Der CRC wird nur anhand der Datenpaketbytes (4–131) berechnet.

## Synchronisation

Der Empfänger sendet zunächst ein ASCII-Zeichen „C“ (0x43) an den Sender, um die Verwendung der
CRC-Blockvalidierung zu signalisieren. Nach dem Senden des ersten „C“ wartet der Empfänger entweder
auf eine 3-sekündige Zeitüberschreitung oder bis ein Puffer-voll-Flag gesetzt wird. Tritt eine
Zeitüberschreitung beim Empfänger auf, wird ein weiteres „C“ an den Sender gesendet, und die 3-sekündige
Zeitüberschreitung beginnt erneut. Dieser Vorgang wird fortgesetzt, bis der Empfänger ein vollständiges
133-Byte-Paket empfängt.

## Hinweise zum Empfänger

Dieses Protokoll erkennt folgende Bedingungen: 1. Framing-Fehler bei einem beliebigen Byte, 2.
Überlauffehler bei einem beliebigen Byte, 3. Doppeltes Paket, 4. CRC-Fehler, 5. Zeitüberschreitung
beim Empfänger (Paket nicht innerhalb von 1 Sekunde empfangen). Bei jeder NAK sendet der Sender das
letzte Paket erneut. Die Punkte 1 und 2 sind als schwerwiegende Hardwarefehler zu betrachten. Stellen
Sie sicher, dass Sender und Empfänger dieselbe Baudrate sowie dieselben Start- und Stoppbits verwenden.
Punkt 3 besteht normalerweise darin, dass der Sender ein verstümmeltes ACK erhält und das Paket erneut
sendet. Punkt 4 tritt in lauten Umgebungen auf. Und das letzte Problem sollte sich selbst korrigieren,
nachdem der Empfänger dem Sender ein NAK erteilt hat.

![Flowchart](./xFlow1.gif)

## Sample crc calculation code

```c
int calcrc(char *ptr, int count)
{
    int  crc;
    char i;

    crc = 0;
    while (--count >= 0)
    {
        crc = crc ^ (int) *ptr++ << 8;
        i = 8;
        do
        {
            if (crc & 0x8000)
                crc = crc << 1 ^ 0x1021;
            else
                crc = crc << 1;
        } while(--i);
    }
    return (crc);
}
```