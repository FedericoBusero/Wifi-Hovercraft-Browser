# Wifi-Hovercraft-Browser
Wifi bestuurde (vanuit een browser ) hovercraft op een ESP8266 (NodeMCU, Wemos D1 mini) of ESP32 en een optionele gyro GY-521

Video: https://www.youtube.com/watch?v=TWfIe7EutRM

## Communicatie
- WifiPoint / SoftAP
- SSID = hover- + 4 laatste hexadecimale karakters van het Wifi-MAC adres van de ESP8266 chip
- Wifi-paswoord: 12345678
- App: browser (Chrome, Firefox, safari, ...)
- URL : http://192.168.4.1 of http://h.be

## App User interface 
![Screenshot_browser_hovercraft.png](Screenshot_browser_hovercraft.png "Hover user interface")
- Bovenste regel: connectiestatus, en op ESP8266 het voltage tijdens connectie (en optioneel de gyro draaisnelheid)
- Bovenste slider: trim de servo
- Tweede slider: stel maximum snelheid in (links = halve kracht, rechts=volle kracht)
- Joystick: besturing servo (links-rechts) en motor (midden-boven)

## pinallocatie Wemos D1 Lite (ESP8266)
Hiertoe moet je volgende regel uncommenten in config.h:
```
// #define ENV_HOVERSERVO_ESP8266_LOLIND1MINILITE
```
| Functie       | Pin | GPIO   |
| ------------- | --- | ------ |
| LEDCONNECTIE  |     | GPIO2  |
| SERVO         | D5  | GPIO14 |
| MOTOR         | D8  | GPIO15 |

## pinallocatie NodeMCU
Hiertoe moet je volgende regel uncommenten in config.h:
```
// #define ENV_HOVERSERVO_ESP8266_NODEMCU
```
| Functie       | Pin | GPIO   |
| ------------- | --- | ------ |
| LEDCONNECTIE  | D0  | GPIO16 |
| SERVO         | D5  | GPIO14 |
| MOTOR         | D8  | GPIO15 |

## pinallocatie ESP01
Naargelang de led pin op GPIO1 of GPIO2 zit moet je volgende regel uncommenten in config.h:
```
// #define ENV_HOVERSERVO_ESP8266_ESP01_LEDPIN1_V0
```
| Functie       | Pin | GPIO  |
| ------------- | --- | ----- |
| LEDCONNECTIE  | TX  | GPIO1 |
| SERVO         |     | GPIO0 |
| MOTOR         | RX  | GPIO3 |

ofwel
```
// #define ENV_HOVERSERVO_ESP8266_ESP01_LEDPIN2_V0
```
| Functie       | Pin | GPIO  |
| ------------- | --- | ----- |
| LEDCONNECTIE  |     | GPIO2 |
| SERVO         |     | GPIO1 |
| MOTOR         | RX  | GPIO3 |

## pinallocatie ESP01 met gyro GY-521
Hiertoe moet je volgende regel uncommenten in config.h:
```
// #define ENV_HOVERSERVOGYRO_ESP8266_ESP01_LEDPIN2_V0
```
| Functie            | Pin | GPIO   |
| ------------------ | --- | ------ |
| SDA & LEDCONNECTIE |     | GPIO2 |
| SCL                |     | GPIO0 |
| SERVO              | TX  | GPIO1 |
| MOTOR              | RX  | GPIO3 |

## Arduino ESP8266 board settings
- Wij blijven de ESP8266 Arduino core 2.7.4 gebruiken, maar er zit nu ook een fix in om het ook op 3.1.2 vlot te laten lopen
- Je kan bij de board settings ook de lwIP settings aanpassen: de default "v2 Lower Memory" is goed, maar "v2 Higher Bandwidth" is beter
- Kies bij "Erase Flash" "All Flash Contents", zoniet kunnen wifi settings van een vorige sessie (of andere configuraties) blijven hangen.

## Arduino bibliotheken
Volgende bibliotheken zijn nodig:

ESP8266:
- ArduinoWebsockets by Gil Maimon, gemakkelijk te installeren vanuit de Arduino Library manager: https://github.com/gilmaimon/ArduinoWebsockets
- https://github.com/me-no-dev/ESPAsyncTCP
- https://github.com/me-no-dev/ESPAsyncWebServer

ESP32:
- AsyncTCP: te installeren van https://github.com/me-no-dev/AsyncTCP
- ESP32Servo vanuit de Arduino library manager te downloaden, dat is deze versie:  https://github.com/madhephaestus/ESP32Servo
- ArduinoWebsockets by Gil Maimon, gemakkelijk te installeren vanuit de Arduino Library manager: https://github.com/gilmaimon/ArduinoWebsockets
- https://github.com/me-no-dev/ESPAsyncWebServer

Daarnaast in geval van gyro GY-521:
- https://github.com/RobTillaart/GY521 versie minimum 0.5.3

## Inspiratie
Bij het ontwikkelen van deze software werden volgende inspiratie-bronnen gebruikt: 
- [ESP32-CAM_TANK van PepeTheFroggie](https://github.com/PepeTheFroggie/ESP32CAM_RCTANK)
- [RobotZero One: ESP32-CAM-rc-car](https://robotzero.one/esp32-cam-rc-car/) met software op https://github.com/robotzero1/esp32cam-rc-car
- [Cellphone controlled RC car](https://github.com/neonious/lowjs_esp32_examples/tree/master/neonious_one/cellphone_controlled_rc_car) 
- de joystick gebaseerd op [Kirupa: Create a Draggable Element in JavaScript](https://www.kirupa.com/html5/drag.htm)

## Micropython
Gebruik je liever micropython dan Arduino? Er is ook een micropython versie voor ESP32: https://github.com/FedericoBusero/HoverMicropyton

## Hoe maak je een Hovercraft?
Voor workshops hovercraft bouwen kan je terecht bij [masynmachien](https://www.masynmachien.be/)
- De bouwinstructies van de hovercraft zijn te vinden op https://drive.google.com/file/d/1SUZypw2QWQQqCWgGDMl3Ls_pUvmDDozy/view?usp=sharing 
- Aanbevolen motor: 
[7.4V 8520 motor 1.25 Plug 48000RPM 1mm shaft](https://www.aliexpress.com/item/1005004245885110.html?spm=a2g0o.order_list.order_list_main.79.5e7b1802S1TcVB)
- Aanbevolen propellors:
[75mm 1mm Propeller](https://www.aliexpress.com/item/1005001492347661.html?spm=a2g0o.order_list.order_list_main.5.5e7b1802S1TcVB)
of
[75mm 1mm Propeller](https://www.aliexpress.com/item/33024513887.html?spm=a2g0o.order_list.order_list_main.10.5e7b1802S1TcVB)
- Aanbevolen batterij:
[3.7V 380mAH Lipo Battery](https://www.aliexpress.com/item/33041901836.html?spm=a2g0o.order_list.order_list_main.175.5e7b1802S1TcVB)
- Inspiratie om een hovercraft te bouwen zonder kit of masynmachien printplaatje vind je [hier](https://docs.google.com/document/d/1fcbP3EX9xSy2-DPe2HyG4lQGwgzG96E5) in de bouwinstructies van een oude versie van de hovercraft:

