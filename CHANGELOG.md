# Changelog

Alle nennenswerten Änderungen an diesem Projekt werden hier dokumentiert.
Format nach [Keep a Changelog](https://keepachangelog.com/de/1.1.0/),
Versionierung nach [Semantic Versioning](https://semver.org/lang/de/).

## [Unveröffentlicht]

### Hinzugefügt
- UART-Bootloader, der eine neue Application per XModem-CRC empfängt und in den Flash schreibt.
- Empfang von 128-Byte-Paketen (`SOH`) und 1024-Byte-Paketen (`STX`) mit CRC-16-Prüfung (Polynom `0x1021`).
- Prüfung von Paketnummer und Komplement; fehlerhafte Pakete werden mit `NAK` neu angefordert.
- Abbruch der Übertragung mit `CAN` nach `X_MAX_ERROR` aufeinanderfolgenden Fehlern oder sofort bei einem Flash-Fehler.
- Unterstützung für STM32F1, STM32F7, STM32G0 und STM32H7 mit Page- bzw. Sector-Löschung je Familie.
- `stmflash_config.h` mit Standardadressen für Application, Gültigkeitskennung, Konfigurationsbereich und Flash-Größe je Familie, überschreibbar in `main.h`.
- Gültigkeitskennung der Application im Flash, die nach erfolgreichem Upload gesetzt und beim Start geprüft wird.
- Automatischer Sprung in eine gültige Application, wenn innerhalb der Wartezeit kein Upload beginnt.
- Sofortiger Sprung in eine gültige Application durch Senden von `A` oder `a`.
- Bricht der Host den Upload mit `CAN` ab, wird der Application-Bereich gelöscht und die Gültigkeitskennung zurückgesetzt.
- Flash-Schreiben mit Rücklesen und Vergleich jedes geschriebenen Worts.
- Flash-API mit `flash_erase()`, `flash_write()`, `flash_validation()` und `flash_jump_to_app()`.
- Status-Enums `xmodem_status` und `flash_status` für die Fehlerrückgabe.
- Versions-Defines `XMODEM_MAJOR`, `XMODEM_MINOR`, `XMODEM_PATCH` und `XMODEM_DEV` in `xmodem.h`.
- README mit Protokollbeschreibung, Flash-Konfiguration und Anleitung zum Einbinden.
