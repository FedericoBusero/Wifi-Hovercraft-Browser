# Wifi-Hovercraft-Browser
Wifi bestuurde (vanuit een browser ) hovercraft op een ESP8266 (NodeMCU, Wemos D1 mini)

Video: https://www.youtube.com/watch?v=TWfIe7EutRM

## Communicatie
- WifiPoint / SoftAP
- SSID = hover- + 6 laatste hexadecimale karakters van het Wifi-MAC adres van de ESP8266 chip
- Wifi-paswoord: 12345678
- App: browser (Chrome, Firefox, safari, ...)
- URL : http://192.168.4.1

## App User interface 
- Bovenste slider: stel maximum snelheid in
- Joystick: besturing servo (links-rechts) en motor (midden-boven)

## pinallocatie
| Functie       | Pin | GPIO   |
| ------------- | --- | ------ |
| LEDCONNECTIE  | D0  | GPIO16 |
| SERVO         | D2  | GPIO4  |
| MOTOR_WIFI    | D8  | GPIO15 |

## Arduino ESP8266 board issue
Het is aangeraden om de meest recente Arduino ESP8266 board versie 3.0.2 niet te gebruiken, die werkt extreem slecht in combinatie met deze software. Beter is voorlopig ESP8266 versie 2.7.4 te blijven gebruiken.

## Arduino bibliotheken
Volgende bibliotheken zijn nodig:
- ArduinoWebsockets by Gil Maimon, gemakkelijk te installeren vanuit de Arduino Library manager: https://github.com/gilmaimon/ArduinoWebsockets
- https://github.com/me-no-dev/ESPAsyncTCP
- https://github.com/me-no-dev/ESPAsyncWebServer

## Hoe maak je een Hovercraft?
De bouwinstructies van de hovercraft - zonder wifi verbinding- zijn te vinden op https://maakbib.be/Lasergesneden-hovercraft/
Voor workshops hovercraft bouwen kan je terecht bij MasynMachien: https://www.instructables.com/masynmachiens-workshops/
