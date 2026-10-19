# Runbook STM32 Bootloader XModem

Handgriffe am Gerät für alle Projekte, die diesen Bootloader einbinden. Die Prozeduren
sind für jedes Gerät gleich. Was vom Gerät abhängt, steht hier als Platzhalter `‹…›` und
ist im Runbook der jeweiligen Anwendung festgelegt, z. B. in
[EAuto_Bootloader](https://github.com/Diveturtle93/EAuto_Bootloader).

| Platzhalter     | Bedeutung                                                    |
|-----------------|--------------------------------------------------------------|
| `‹Port›`        | COM-Port des USB-UART-Adapters am PC                         |
| `‹Baudrate›`    | Baudrate der Bootloader-Schnittstelle                        |
| `‹Reset›`       | Wie das Gerät neu gestartet wird (Reset-Taste, Versorgung)   |
| `‹Firmware›`    | Name und Ablage der `.bin`-Datei der Application             |

Wie der Bootloader arbeitet, steht in der [README](README.md#beschreibung).

## Firmware übertragen

### Voraussetzungen
- PC mit TeraTerm und USB-UART-Adapter
- Firmware als `.bin`-Datei: `‹Firmware›`. Keine `.hex`- oder `.elf`-Datei.
- Schnittstelle: `‹Baudrate›`, 8N1, kein Handshake

### Schritte
1. Adapter an die Bootloader-Schnittstelle des Geräts und an den PC anschließen.
2. TeraTerm öffnen: **Datei → Neue Verbindung → Seriell**, `‹Port›` wählen.
3. **Einstellungen → Serieller Port**: `‹Baudrate›`, 8 Datenbits, keine Parität, 1 Stoppbit,
   Flusssteuerung **keine**.
4. Gerät neu starten: `‹Reset›`.
5. Im Terminal erscheinen fortlaufend `C`-Zeichen (`CCCC…`). Ab jetzt läuft die Zeit:
   Ist schon eine Application im Gerät, startet sie nach kurzer Zeit von selbst.
6. Sofort **Datei → Transfer → XMODEM → Senden** wählen.
7. Im Dialog die `.bin`-Datei wählen, Option **CRC** oder **1K** einstellen, nicht
   **Checksum**. **Öffnen** klicken.
8. Warten, bis der Fortschrittsdialog sich schließt. Nicht **Abbrechen** klicken, das
   löscht die Application im Gerät.

### Prüfung
- Im Terminal steht `Firmware geupdated!` und `Springe zu Benutzer Application...`.
- Das Gerät läuft mit der neuen Firmware: `‹Erkennungsmerkmal, z. B. Versionsausgabe›`.

### Fehlerbilder

| Symptom | Ursache | Abhilfe |
|---------|---------|---------|
| Keine Zeichen im Terminal | falscher Port, Adapter nicht verbunden oder Gerät ohne Versorgung | Port in TeraTerm und Verkabelung prüfen, Schritt 4 wiederholen |
| Wirre Zeichen statt `C` | falsche Baudrate | Schritt 3 mit `‹Baudrate›` wiederholen |
| `C`-Zeichen hören auf, bevor gesendet wurde | Bootloader ist in die vorhandene Application gesprungen | Schritt 4 wiederholen und Schritt 6 direkt nach dem ersten `C` ausführen |
| Übertragung bricht nach wenigen Paketen ab, Terminal zeigt Abbruch | 3 Fehler in Folge: Option **Checksum** gewählt oder Störungen auf der Leitung | Option **CRC** wählen, Kabel kürzen oder Adapter tauschen, dann [Nach abgebrochenem Upload wiederherstellen](#nach-abgebrochenem-upload-wiederherstellen) |
| Übertragung bricht immer an derselben Stelle ab | Application ist größer als der Speicherbereich, den der Bootloader beschreiben darf | Nicht am Gerät lösbar. Entwickler informieren (siehe [README, Fehlerbilder](README.md#fehlerbilder)) |
| Übertragung erfolgreich, Gerät reagiert danach nicht | falsche Datei (`.hex`/`.elf`) oder Firmware nicht für den Bootloader gebaut | [Nach abgebrochenem Upload wiederherstellen](#nach-abgebrochenem-upload-wiederherstellen) mit der richtigen `.bin`. Hilft das nicht, Entwickler informieren |

## Ohne Update in die Application springen

### Voraussetzungen
- TeraTerm verbunden wie in [Firmware übertragen](#firmware-übertragen), Schritte 1–3

### Schritte
1. Gerät neu starten: `‹Reset›`.
2. Sobald `C`-Zeichen erscheinen, im Terminal die Taste `A` drücken.

### Prüfung
- Im Terminal steht `Application vorhanden.` und `Springe zu Benutzer Application...`.

### Fehlerbilder

| Symptom | Ursache | Abhilfe |
|---------|---------|---------|
| `Sprung zu Application nicht moeglich.` | Keine gültige Application im Gerät | [Firmware übertragen](#firmware-übertragen) |

## Nach abgebrochenem Upload wiederherstellen

Nach einem Abbruch ist die alte Application gelöscht. Das ist gewollt, damit keine halb
geschriebene Firmware startet. Der Bootloader wartet dann dauerhaft auf einen neuen Upload.

### Voraussetzungen
- Wie in [Firmware übertragen](#firmware-übertragen)
- Die zuletzt funktionierende `.bin`-Datei, falls die neue nicht übertragen werden kann

### Schritte
1. TeraTerm verbinden wie in [Firmware übertragen](#firmware-übertragen), Schritte 1–3.
2. Gerät neu starten: `‹Reset›`.
3. Sobald `C`-Zeichen erscheinen, [Firmware übertragen](#firmware-übertragen) ab Schritt 6
   ausführen. Nicht abwarten, ob die `C`-Zeichen von selbst aufhören.

### Prüfung
- Wie in [Firmware übertragen](#firmware-übertragen).

### Fehlerbilder

| Symptom | Ursache | Abhilfe |
|---------|---------|---------|
| `C`-Zeichen hören nach kurzer Zeit auf, Gerät reagiert nicht | Bootloader hält die gelöschte Application noch für gültig | Schritt 2 wiederholen und Schritt 3 direkt nach dem ersten `C` ausführen |
| Auch nach Neustart keine Zeichen im Terminal | Bootloader selbst beschädigt oder Gerät defekt | Am UART nicht lösbar. Bootloader mit Programmer (ST-Link) neu aufspielen, Entwickler informieren |
| Neue Firmware bricht wieder ab | Ursache aus [Firmware übertragen, Fehlerbilder](#fehlerbilder) besteht weiter | Ursache dort beheben oder die zuletzt funktionierende `.bin` übertragen |
