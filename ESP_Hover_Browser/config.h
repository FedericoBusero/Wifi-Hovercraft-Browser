#pragma once

// Board settings

// Uncomment één van volgende defines

// #define ENV_HOVER_ESP8266_ESP01_LEDPIN1_V0
// #define ENV_HOVER_ESP8266_ESP01_LEDPIN2_V0
// #define ENV_HOVER_ESP8266_LOLIND1MINILITE
// #define ENV_HOVER_ESP8266_NODEMCU

// Als de defines in platformio.ini gedefinieerd zijn:
// #define ENV_USER_DEFINED

/*
Als je een ander board wenst te gebruiken, zijn volgende defines nodig:
* Als je seriële output wenst (en de RX/TX pinnen zijn niet in gebruik voor andere doelen):
#define DEBUG_SERIAL Serial

Volgende pinnen worden gedefinieerd:
- PIN_SERVO          
- PIN_MOTOR          
- (optioneel) PIN_LEDCONNECTIE   

Daarnaast zijn volgende defines verplicht (maar kunnen omgewisseld worden)
#define LED_BRIGHTNESS_ON  HIGH
#define LED_BRIGHTNESS_OFF LOW

Op ESP8266-chips wordt het voltage gemeten, voeg volgende defines toe
// Pas de voltagefactor aan, dat is bij elke chip verschillend. Calibreer bv. met USB stroom die 3.3V op de chip moet geven
#define VOLTAGE_FACTOR 1060.0f 
#define VOLTAGE_THRESHOLD 2.4 // onder dit voltage valt de chip uit om de batterij te beschermen

*/

#if defined(ENV_HOVER_ESP8266_ESP01_LEDPIN1_V0)

#define PIN_SERVO          0
#define PIN_MOTOR          3
#define PIN_LEDCONNECTIE   1

// Pas de voltagefactor aan, dat is bij elke chip verschillend. Calibreer bv. met USB stroom die 3.3V op de chip moet geven
#define VOLTAGE_FACTOR 1060.0f 
#define VOLTAGE_THRESHOLD 2.4 // onder dit voltage valt de chip uit om de batterij te beschermen

#define LED_BRIGHTNESS_ON  LOW
#define LED_BRIGHTNESS_OFF HIGH

#elif defined(ENV_HOVER_ESP8266_ESP01_LEDPIN2_V0)

#define PIN_SERVO          0
#define PIN_MOTOR          3
#define PIN_LEDCONNECTIE   2

// Pas de voltagefactor aan, dat is bij elke chip verschillend. Calibreer bv. met USB stroom die 3.3V op de chip moet geven
#define VOLTAGE_FACTOR 1060.0f 
#define VOLTAGE_THRESHOLD 2.4 // onder dit voltage valt de chip uit om de batterij te beschermen

#define LED_BRIGHTNESS_ON  LOW
#define LED_BRIGHTNESS_OFF HIGH

#elif defined (ENV_HOVER_ESP8266_LOLIND1MINILITE)

#define DEBUG_SERIAL Serial

#define PIN_SERVO          D2 // D2 = GPIO4  op NodeMCU & Wemos D1 mini
#define PIN_MOTOR          D8 // D8 = GPIO15 op NodeMCU & Wemos D1 mini
// De ingebouwde LED zit meestal op GPIO2 of GPIO16
#define PIN_LEDCONNECTIE    2 

// Pas de voltagefactor aan, dat is bij elke chip verschillend. Calibreer bv. met USB stroom die 3.3V op de chip moet geven
#define VOLTAGE_FACTOR 910.0f 
#define VOLTAGE_THRESHOLD 2.4 // onder dit voltage valt de chip uit om de batterij te beschermen

#define LED_BRIGHTNESS_ON  LOW
#define LED_BRIGHTNESS_OFF HIGH

#elif defined (ENV_HOVER_ESP8266_NODEMCU)

#define DEBUG_SERIAL Serial

#define PIN_SERVO          D2 // D2 = GPIO4  op NodeMCU & Wemos D1 mini
#define PIN_MOTOR          D8 // D8 = GPIO15 op NodeMCU & Wemos D1 mini
// De ingebouwde LED zit meestal op GPIO2 of GPIO16
#define PIN_LEDCONNECTIE    D0 // D0=GPIO16

// Pas de voltagefactor aan, dat is bij elke chip verschillend. Calibreer bv. met USB stroom die 3.3V op de chip moet geven
#define VOLTAGE_FACTOR 910.0f 
#define VOLTAGE_THRESHOLD 2.4 // onder dit voltage valt de chip uit om de batterij te beschermen

#define LED_BRIGHTNESS_ON  LOW
#define LED_BRIGHTNESS_OFF HIGH

#elif defined ENV_USER_DEFINED
// defines staan buiten de code
#else
// Geen ENV_XX geselecteerd
#error "Defineer één van bovenstaande defines"

#endif
