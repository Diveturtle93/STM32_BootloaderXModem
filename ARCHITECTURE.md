# Architektur STM32 Bootloader XModem

Der Bootloader besteht aus zwei Schichten: `xmodem.c` führt das Protokoll und
entscheidet, ob empfangen oder in die Application gesprungen wird, `flash.c` kapselt
Löschen, Schreiben, Prüfen und den Sprung je STM32-Familie. Alle Adressen kommen aus
`stmflash_config.h`. Es gibt keine Interrupts und keinen Puffer über ein Paket hinaus:
Jedes Paket wird blockierend empfangen, sofort geschrieben und zurückgelesen.

## Komponenten

```mermaid
flowchart TD
    Main[main.c<br/>Hauptschleife der Anwendung] --> XM[xmodem.c<br/>Protokoll, CRC, Gültigkeitskennung]
    XM --> FL[flash.c<br/>Löschen, Schreiben, Rücklesen, Sprung]
    XM --> UART[STM32_Basicuart<br/>uartReceive, uartTransmit]
    XM -.-> CFG[stmflash_config.h<br/>Adressen je Familie]
    FL -.-> CFG
    CFG -.-> MainH[main.h<br/>Familien-Symbol, eigene Adressen]
    FL --> HALFL[HAL FLASH / FLASHEx]
    UART --> HALUART[HAL UART]
    FL -->|flash_jump_to_app| App[Application ab<br/>FLASH_APP_START_ADDRESS]
```

| Modul | Datei(en) | Aufgabe |
|---|---|---|
| Protokoll | `xmodem.c`, `xmodem.h` | Hauptschleife `xmodem_receive()`, Paket empfangen und prüfen, Fehler zählen, Gültigkeitskennung lesen und schreiben |
| Flash | `flash.c`, `flash.h` | Page- bzw. Sector-Löschung, Schreiben in der Breite der Familie mit Rücklesen, Vergleich von 4 Byte, Sprung in die Application |
| Konfiguration | `stmflash_config.h` | Wählt `FLASH_PAGE` oder `FLASH_SECTOR`, Standardadressen und Sector-Tabellen je Familie; jeder Wert in `main.h` überschreibbar |
| UART | `basicuart.h` (extern) | Blockierendes Senden und Empfangen mit Timeout, siehe [README](README.md#abhängigkeiten) |

Die Familie wird zur Compile-Zeit über `STM32F1`, `STM32F7`, `STM32G0` oder `STM32H7`
aus `main.h` gewählt. Alle familienabhängigen Stellen sind `#if`-Zweige, es gibt keine
Laufzeitumschaltung.

## Datenfluss

Ablauf eines Uploads. Alles läuft in der Hauptschleife, die Aufrufe an `basicuart`
blockieren bis zum Empfang oder Timeout.

```mermaid
sequenceDiagram
    participant Main as Hauptschleife
    participant XM as xmodem.c
    participant UART as basicuart
    participant FL as flash.c
    participant HW as Flash

    Main->>XM: xmodem_receive()
    XM->>FL: app_validation() → flash_validation() ×2
    loop Warten auf Header
        XM->>UART: uartReceive(1 Byte)
        UART-->>XM: Timeout
        XM->>UART: C
        Note over XM: Gültige App: nach 100 Timeouts Sprung
    end
    UART-->>XM: SOH / STX
    XM->>XM: xmodem_handle_packet()
    XM->>UART: uartReceive(Nummer, Daten, CRC)
    XM->>XM: xmodem_calc_crc()
    opt erstes Paket
        XM->>FL: flash_erase(FLASH_APP_START_ADDRESS)
        FL->>HW: HAL_FLASHEx_Erase
    end
    XM->>XM: Paketnummer, Komplement, CRC prüfen
    XM->>FL: flash_write(Adresse, Daten, Länge)
    FL->>HW: HAL_FLASH_Program je Wort
    FL->>FL: flash_validation() je 4 Byte
    FL-->>XM: FLASH_OK
    XM->>UART: ACK
    Note over XM: weitere Pakete wie oben
    UART-->>XM: EOT / ETB
    XM->>UART: ACK
    XM->>FL: flash_write(FLASH_APP_VALID_ADDRESS, Kennung)
    XM->>FL: flash_jump_to_app()
```

Zustände innerhalb von `xmodem_receive()`:

```mermaid
stateDiagram-v2
    [*] --> Warten: Kennung prüfen
    Warten --> Warten: Timeout, C senden
    Warten --> Sprung: A / a oder 100 Timeouts bei gültiger App
    Warten --> Empfang: SOH / STX, Flash löschen
    Empfang --> Empfang: Paket OK → ACK, Fehler → NAK
    Empfang --> Sprung: EOT / ETB, Kennung schreiben
    Empfang --> Abbruch: X_MAX_ERROR Fehler oder Flash-Fehler
    Warten --> Abbruch: CAN vom Host
    Empfang --> Abbruch: CAN vom Host
    Abbruch --> [*]: Rückkehr, nächster Aufruf beginnt neu
    Sprung --> [*]: Application läuft
```

Fehler werden in `xmodem_status` und `flash_status` als Bitmaske gesammelt, damit ein
Paket mehrere Ursachen gleichzeitig melden kann. `xmodem_error_handler()` zählt nur
aufeinanderfolgende Fehler. Ein Flash-Fehler setzt den Zähler direkt auf `X_MAX_ERROR`,
weil eine Wiederholung des Pakets den Flash nicht repariert.

## Speicher und Ressourcen

Aufteilung des Flash mit den Standardwerten aus `stmflash_config.h`:

| Bereich | STM32F1 | STM32G0 | STM32F7 | STM32H7 |
|---|---|---|---|---|
| Bootloader | `0x08000000`–`0x08007FFF` | `0x08000000`–`0x08007FFF` | Sektor 0 | Sektor 0 |
| Gültigkeitskennung (8 Byte) | `0x08007FF8`, letzte Page des Bootloaders | `0x08007FF8`, letzte Page des Bootloaders | `0x0800FFF8`, Ende Sektor 1 | `0x080DFFE0`, Ende Sektor 6 |
| Application | ab `0x08008000` | ab `0x08008000` | Sektor 1 ab `0x08008000` | Sektor 1 ab `0x08020000` |
| Konfiguration der Application | `0x08038000`, im App-Bereich | `0x08018000`, im App-Bereich | `0x08010000`, Sektor 2 | `0x080DFF00`, Sektor 6 |
| Löschen beim Upload | alle Pages bis `FLASH_APP_END_ADDRESS` | alle Pages bis `FLASH_APP_END_ADDRESS` | nur Sektor 1 | nur Sektor 1 |

Die Gültigkeitskennung ist das feste Muster `app_valid_array` in `xmodem.c`. Sie liegt
außerhalb des Bereichs, den die Application belegt, und wird erst nach `EOT` geschrieben.
Ein abgebrochener oder unvollständiger Upload hinterlässt so keine Kennung, die zu einem
Sprung in eine halbe Application führt. Ob `flash_erase()` die Kennung mitlöscht, hängt
von der Familie ab (siehe Tabelle). Der Konfigurationsbereich wird vom Bootloader nicht
benutzt, er ist für die Application reserviert.

Schreibbreite je Familie, daraus ergibt sich der Teiler in `xmodem_handle_packet()`:

| Familie | HAL-Typ | Breite | Wörter je 128-Byte-Paket |
|---|---|---|---|
| STM32F1, STM32G0 | `FLASH_TYPEPROGRAM_DOUBLEWORD` | 8 Byte | 16 |
| STM32F7 | `FLASH_TYPEPROGRAM_WORD` | 4 Byte | 32 |
| STM32H7 | `FLASH_TYPEPROGRAM_FLASHWORD` | 32 Byte | 4 |

| Ressource | Verwendung |
|---|---|
| Stack | `xmodem_handle_packet()` legt 1028 Byte Empfangspuffer auf dem Stack an, der Stack des Bootloaders muss dafür reichen |
| Statische Daten | Paketnummer, aktuelle Schreibadresse, Merker für das erste Paket, Kennung und Löschmuster (je 8 Byte) |
| UART | eine Schnittstelle über `basicuart`, Baudrate und Timeout dort festgelegt; der Timeout bestimmt die Wartezeit vor dem automatischen Sprung |
| Flash | Bank 1, `HAL_FLASH_Unlock()` / `HAL_FLASH_Lock()` um jeden Lösch- und Schreibvorgang |

## Nebenläufigkeit

| Kontext | Was läuft dort | Gemeinsame Daten |
|---|---|---|
| Hauptschleife | der gesamte Bootloader: Empfang, CRC, Flash, Sprung | – |
| Interrupts | keine eigenen; HAL-Zeitbasis über SysTick für UART-Timeout und Flash-Wartezeiten | – |

Vor dem Sprung ruft `flash_jump_to_app()` `HAL_DeInit()` auf, setzt den Main Stack
Pointer aus dem ersten Wort der Vektortabelle der Application und springt an deren
Reset-Handler aus dem zweiten Wort. Die Vektortabelle verschiebt die Application selbst
(`VECT_TAB_OFFSET`, siehe [README](README.md#4-application-anpassen--vektortabelle)).

## Erweiterung

- **Neue STM32-Familie:** in `stmflash_config.h` `FLASH_PAGE` oder `FLASH_SECTOR` setzen,
  alle Standardadressen und bei Sektoren die Tabelle `ADDR_FLASH_SECTOR_n_BANK1` ergänzen.
  In `flash.c` die `#if`-Zweige in `flash_erase()` (Löschtyp, Page/Sector), `flash_write()`
  (Zusammensetzen des Worts, HAL-Typ, Rücklesen, Adressschritt) und bei Sektoren
  `GetSector()` ergänzen. In `xmodem.c` den Teiler in `xmodem_handle_packet()` und die
  Länge beim Schreiben der Kennung nach `EOT` anpassen. README-Tabellen im selben Commit
  nachziehen.
- **Neues Steuerbyte vom Host:** Define in `xmodem.h` anlegen und einen `case` im
  `switch (header)` von `xmodem_receive()` ergänzen. Das Byte darf nicht mit `SOH`,
  `STX`, `EOT`, `ETB` oder `CAN` kollidieren.
- **Andere Wartezeit vor dem Sprung:** Die Zahl 100 steht direkt in `xmodem_receive()`,
  die Dauer eines Versuchs ist der UART-Timeout in `basicuart`.
