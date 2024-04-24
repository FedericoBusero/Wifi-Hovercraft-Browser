#pragma once

// Board settings

// Uncomment één van volgende defines

// #define ENV_HOVERSERVO_ESP8266_ESP01_LEDPIN1_V0
// #define ENV_HOVERSERVO_ESP8266_ESP01_LEDPIN2_V0
// #define ENV_HOVERSERVO_ESP8266_LOLIND1MINILITE
// #define ENV_HOVERSERVO_ESP8266_NODEMCU

// #define ENV_HOVERSERVOGYRO_ESP8266_ESP01_LEDPIN2_V0

// Als de defines in platformio.ini gedefinieerd zijn:
// #define ENV_USER_DEFINED

/*
Als je een ander board wenst te definiëren, zijn volgende defines nodig:
* Als een gyro gebruikt wordt, zijn volgende defines nodig:
- USE_GY521
- GYRO_REGELING_P
- GYRO_REGELING_MAX_DRAAI
- GYRO_REGELING_BIAS
- GYRO_DIRECTION : GYRO_DIRECTION_X, GYRO_DIRECTION_Y of GYRO_DIRECTION_Z
- GYRO_KALMAN_Q
- (optioneel) SERVO_ANTI_BIBBER : aantal graden, als de doelpositie van de servo kleiner is dan deze waarde t.o.v. de huidige postie, blijft de servo gewoon staan op de huidige positie
- (optioneel) PIN_SDA en PIN_SCL : indien niet gedefinieerd, worden de standaard Wire library pinnen van het bord gebruikt. 
  Als één van de I2C pinnen ook als PIN_LEDCONNECTIE gebruikt wordt, definieer ook PIN_LED_DUALUSE

* Als je seriële output wenst (en de RX/TX pinnen zijn niet in gebruik voor andere doelen):
#define DEBUG_SERIAL Serial

Volgende pinnen worden gedefinieerd:
- PIN_SERVO          
- PIN_MOTOR          
- (optioneel) PIN_LEDCONNECTIE   

Daarnaast zijn volgende defines verplicht (maar kunnen omgewisseld worden)
#define LED_BRIGHTNESS_ON  HIGH
#define LED_BRIGHTNESS_OFF LOW

Op ESP8266-chips wordt het voltage gemeten, voeg volgende define toe. Pas de voltagefactor aan, dat is bij elke chip verschillend. 
Calibreer bv. met USB stroom die 3.3V op de chip moet geven
#define VOLTAGE_FACTOR 1060.0f 

*/

enum 
{
    GYRO_DIRECTION_X,
    GYRO_DIRECTION_Y,
    GYRO_DIRECTION_Z,
};

#ifndef ENV_USER_DEFINED

// Volgende defines zijn op alle borden van toepassing
#define WIFI_SOFTAP_SSID_PREFIX "hover-"
#define WIFI_SOFTAP_PASSWORD "12345678"
#define WIFI_SOFTAP_CHANNEL 1 // 1-13

#define VOLTAGE_THRESHOLD 2.7 // onder dit voltage valt de ESP8266-chip uit om de batterij te beschermen

#endif

#if defined(ENV_HOVERSERVO_ESP8266_ESP01_LEDPIN1_V0)

#define PIN_SERVO          0
#define PIN_MOTOR          3
#define PIN_LEDCONNECTIE   1

// Pas de voltagefactor aan, dat is bij elke chip verschillend. Calibreer bv. met USB stroom die 3.3V op de chip moet geven
#define VOLTAGE_FACTOR 1060.0f 

#define LED_BRIGHTNESS_ON  LOW
#define LED_BRIGHTNESS_OFF HIGH

#elif defined(ENV_HOVERSERVO_ESP8266_ESP01_LEDPIN2_V0)

#define PIN_SERVO          1
#define PIN_MOTOR          3
#define PIN_LEDCONNECTIE   2

// Pas de voltagefactor aan, dat is bij elke chip verschillend. Calibreer bv. met USB stroom die 3.3V op de chip moet geven
#define VOLTAGE_FACTOR 1060.0f 

#define LED_BRIGHTNESS_ON  LOW
#define LED_BRIGHTNESS_OFF HIGH

#elif defined (ENV_HOVERSERVO_ESP8266_LOLIND1MINILITE)

#define DEBUG_SERIAL Serial

#define PIN_SERVO          D5 // D5 = GPIO14  op NodeMCU & Wemos D1 mini
#define PIN_MOTOR          D8 // D8 = GPIO15 op NodeMCU & Wemos D1 mini
// De ingebouwde LED zit meestal op GPIO2 of GPIO16
#define PIN_LEDCONNECTIE    2 

// Pas de voltagefactor aan, dat is bij elke chip verschillend. Calibreer bv. met USB stroom die 3.3V op de chip moet geven
#define VOLTAGE_FACTOR 910.0f 

#define LED_BRIGHTNESS_ON  LOW
#define LED_BRIGHTNESS_OFF HIGH

#elif defined (ENV_HOVERSERVO_ESP8266_NODEMCU)

#define DEBUG_SERIAL Serial

#define PIN_SERVO          D5 // D5 = GPIO14  op NodeMCU & Wemos D1 mini
#define PIN_MOTOR          D8 // D8 = GPIO15 op NodeMCU & Wemos D1 mini
// De ingebouwde LED zit meestal op GPIO2 of GPIO16
#define PIN_LEDCONNECTIE    D0 // D0=GPIO16

// Pas de voltagefactor aan, dat is bij elke chip verschillend. Calibreer bv. met USB stroom die 3.3V op de chip moet geven
#define VOLTAGE_FACTOR 910.0f 

#define LED_BRIGHTNESS_ON  LOW
#define LED_BRIGHTNESS_OFF HIGH


#elif defined(ENV_HOVERSERVOGYRO_ESP8266_ESP01_LEDPIN2_V0)
#define USE_GY521
#define GYRO_DIRECTION GYRO_DIRECTION_X
#define GYRO_REGELING_P         4.0
#define GYRO_REGELING_MAX_DRAAI 0.5
#define GYRO_REGELING_BIAS      1.0
#define GYRO_KALMAN_Q           0.04
#define SERVO_ANTI_BIBBER       3.0

#define PIN_SERVO          1
#define PIN_MOTOR          3

#define PIN_LEDCONNECTIE   2 
#define PIN_LED_DUALUSE
#define PIN_SDA            2            
#define PIN_SCL            0

// Pas de voltagefactor aan, dat is bij elke chip verschillend. Calibreer bv. met USB stroom die 3.3V op de chip moet geven
#define VOLTAGE_FACTOR 1060.0f 

#define LED_BRIGHTNESS_ON  LOW
#define LED_BRIGHTNESS_OFF HIGH

#elif defined ENV_USER_DEFINED
// defines staan buiten de code

#else
// Geen ENV_XX geselecteerd
#error "Defineer één van bovenstaande defines"

#endif
