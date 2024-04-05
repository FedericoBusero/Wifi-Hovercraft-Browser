#pragma once

// Board settings

// Uncomment één van volgende defines

// #define ENV_HOVER_ESP32C3_LOLINC3PICO
// #define ENV_HOVER_ESP32_LOLIN32LITE
// #define ENV_HOVER_ESP8266_ESP01_LEDPIN1_V0
// #define ENV_HOVER_ESP8266_ESP01_LEDPIN2_V0
// #define ENV_HOVER_ESP8266_LOLIND1MINILITE
// #define ENV_HOVER_ESP32S2_LOLINS2MINI
// #define ENV_HOVER_ESP32S3_LOLINS3MINI

// #define ENV_HOVERSERVOGYRO_ESP32C3_LOLINC3PICO
// #define ENV_HOVERSERVOGYRO_ESP32C3_LOLINC3MINI
// #define ENV_HOVERSERVOGYRO_ESP32_LOLIN32LITE
// #define ENV_HOVERSERVOGYRO_ESP8266_ESP01_LEDPIN2_V0
// #define ENV_HOVERSERVOGYRO_ESP8266_LOLIND1MINILITE
// #define ENV_HOVERSERVOGYRO_ESP32S2_LOLINS2MINI
// #define ENV_HOVERSERVOGYRO_ESP32S3_LOLINS3MINI


#if defined (ENV_HOVER_ESP32C3_LOLINC3PICO)
// ESP32C3 Wemos Lolin C3 Pico 
#define DEBUG_SERIAL Serial

#define PIN_SERVO          1
#define PIN_MOTOR          5
// #define PIN_RGBLED         7

#define LED_BRIGHTNESS_ON  HIGH
#define LED_BRIGHTNESS_OFF LOW

#elif defined(ENV_HOVER_ESP32_LOLIN32LITE)
// ESP32 Wemos Lolin32 lite
#define DEBUG_SERIAL Serial

#define PIN_SERVO          18 
#define PIN_MOTOR          19 
#define PIN_LEDCONNECTIE   LED_BUILTIN

#define LED_BRIGHTNESS_ON  HIGH
#define LED_BRIGHTNESS_OFF LOW

#elif defined(ENV_HOVER_ESP8266_ESP01_LEDPIN1_V0)

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

#elif defined(ENV_HOVER_ESP32S2_LOLINS2MINI)
// ESP32S2 Wemos Lolin S2 mini
#define DEBUG_SERIAL Serial

#define PIN_SERVO          39 
#define PIN_MOTOR          18 
#define PIN_LEDCONNECTIE   15

#define LED_BRIGHTNESS_ON  HIGH
#define LED_BRIGHTNESS_OFF LOW

#elif defined(ENV_HOVER_ESP32S3_LOLINS3MINI)
// ESP32S3 Wemos Lolin S3 mini
#define DEBUG_SERIAL Serial

#define PIN_SERVO          43 
#define PIN_MOTOR          18 
// #define PIN_RGBLED         47

#define LED_BRIGHTNESS_ON  HIGH
#define LED_BRIGHTNESS_OFF LOW

#elif defined (ENV_HOVERSERVOGYRO_ESP32C3_LOLINC3PICO)
// ESP32C3 Wemos Lolin C3 Pico 
#define DEBUG_SERIAL Serial

#define PIN_SERVO          1
#define PIN_MOTOR          5
// #define PIN_RGBLED         7
#define PIN_SDA            8
#define PIN_SCL            10

#define LED_BRIGHTNESS_ON  HIGH
#define LED_BRIGHTNESS_OFF LOW

#elif defined(ENV_HOVERSERVOGYRO_ESP32C3_LOLINC3MINI)
// ESP32C3 Wemos Lolin C3 mini
#define DEBUG_SERIAL Serial

#define PIN_SERVO          21
#define PIN_MOTOR          7 
// #define PIN_RGBLED         7 gr

#define PIN_SDA            8
#define PIN_SCL            10

#define LED_BRIGHTNESS_ON  HIGH
#define LED_BRIGHTNESS_OFF LOW

#elif defined(ENV_HOVERSERVOGYRO_ESP32_LOLIN32LITE)
// ESP32 Wemos Lolin32 lite
#define DEBUG_SERIAL Serial

#define PIN_SERVO          18 
#define PIN_MOTOR          19 
#define PIN_LEDCONNECTIE   LED_BUILTIN
// #define PIN_SDA ..
// #define PIN_SCL ..

#define LED_BRIGHTNESS_ON  HIGH
#define LED_BRIGHTNESS_OFF LOW

#elif defined(ENV_HOVERSERVOGYRO_ESP8266_ESP01_LEDPIN2_V0)

#define PIN_SERVO          0
#define PIN_MOTOR          3

#define PIN_LEDCONNECTIE   2 
#define PIN_LED_DUALUSE
#define PIN_SDA            2            
#define PIN_SCL            0

// Pas de voltagefactor aan, dat is bij elke chip verschillend. Calibreer bv. met USB stroom die 3.3V op de chip moet geven
#define VOLTAGE_FACTOR 1060.0f 
#define VOLTAGE_THRESHOLD 2.4 // onder dit voltage valt de chip uit om de batterij te beschermen

#define LED_BRIGHTNESS_ON  LOW
#define LED_BRIGHTNESS_OFF HIGH

#elif defined (ENV_HOVERSERVOGYRO_ESP8266_LOLIND1MINILITE)

#define DEBUG_SERIAL Serial

#define PIN_SERVO          D2 // D2 = GPIO4  op NodeMCU & Wemos D1 mini
#define PIN_MOTOR          D8 // D8 = GPIO15 op NodeMCU & Wemos D1 mini
// De ingebouwde LED zit meestal op GPIO2 of GPIO16
#define PIN_LEDCONNECTIE    2
#define PIN_LED_DUALUSE
#define PIN_SDA             2
#define PIN_SCL             0

// Pas de voltagefactor aan, dat is bij elke chip verschillend. Calibreer bv. met USB stroom die 3.3V op de chip moet geven
#define VOLTAGE_FACTOR 910.0f 
#define VOLTAGE_THRESHOLD 2.4 // onder dit voltage valt de chip uit om de batterij te beschermen

#define LED_BRIGHTNESS_ON  LOW
#define LED_BRIGHTNESS_OFF HIGH

#elif defined(ENV_HOVERSERVOGYRO_ESP32S2_LOLINS2MINI)
// ESP32S2 Wemos Lolin S2 mini
#define DEBUG_SERIAL Serial

#define PIN_SERVO          39 
#define PIN_MOTOR          18 
#define PIN_LEDCONNECTIE   15
#define PIN_SDA            33 
#define PIN_SCL            35

#define LED_BRIGHTNESS_ON  HIGH
#define LED_BRIGHTNESS_OFF LOW

#elif defined(ENV_HOVERSERVOGYRO_ESP32S3_LOLINS3MINI)
// ESP32S3 Wemos Lolin S3 mini
#define DEBUG_SERIAL Serial

#define PIN_SERVO          43 
#define PIN_MOTOR          18 
// #define PIN_RGBLED         47

#define PIN_SDA            35
#define PIN_SCL            36

#define LED_BRIGHTNESS_ON  HIGH
#define LED_BRIGHTNESS_OFF LOW

#else
// Geen ENV_XX geselecteerd
#error "Defineer één van bovenstaande defines"

#endif
