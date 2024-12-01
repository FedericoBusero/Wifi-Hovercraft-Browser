#pragma once

// Board settings

// Uncomment één van volgende defines

// #define ENV_HOVERSERVO_ESP8266_ESP01_LEDPIN1_V0
// #define ENV_HOVERSERVO_ESP8266_ESP01_LEDPIN2_V0
// #define ENV_HOVERSERVO_ESP8266_LOLIND1MINILITE
// #define ENV_HOVERSERVO_ESP8266_NODEMCU

// #define ENV_HOVERSERVOGYRO_ESP8266_ESP01_LEDPIN2_V0
// #define ENV_HOVERSERVOGYRO_ESP32C3_SUPERMINI_WS2812FX_V0
// #define ENV_HOVERSERVOGYRO_ESP32C3_SUPERMINI_WS2812FX_V1

// Als de defines in platformio.ini gedefinieerd zijn:
// #define ENV_USER_DEFINED

/*
Als je een ander board wenst te definiëren, zijn volgende defines nodig:
* Als een gyro gebruikt wordt, zijn volgende defines nodig:
- USE_FASTIMU
- FASTIMU_TYPE
- IMU_I2C_ADDRESS
- GYRO_REGELING_P
- GYRO_REGELING_MAX_DRAAI
- GYRO_REGELING_BIAS
- GYRO_DIRECTION : GYRO_DIRECTION_X, GYRO_DIRECTION_Y of GYRO_DIRECTION_Z
- GYRO_LPF_TF
- (optioneel) SERVO_ANTI_BIBBER : aantal graden, als de doelpositie van de servo kleiner is dan deze waarde t.o.v. de huidige postie, blijft de servo gewoon staan op de huidige positie
- (optioneel) GYRO_FLIP : gebruik de negatieve waarde van de gyro: als de gyro omgekeerd hangt
- (optioneel) PIN_SDA en PIN_SCL : indien niet gedefinieerd, worden de standaard Wire library pinnen van het bord gebruikt. 
  Als één van de I2C pinnen ook als PIN_LEDCONNECTIE gebruikt wordt, definieer ook PIN_LED_DUALUSE

* Als je seriële output wenst (en de RX/TX pinnen zijn niet in gebruik voor andere doelen):
#define DEBUG_SERIAL Serial

Volgende pinnen worden gedefinieerd:
- PIN_SERVO          
- PIN_MOTOR          
- MOTORZ_TIME_UP
- (optioneel) PIN_LEDCONNECTIE   

Daarnaast zijn volgende defines verplicht (maar kunnen omgewisseld worden)
#define LED_BRIGHTNESS_ON  HIGH
#define LED_BRIGHTNESS_OFF LOW

Om een LED-strip aan te sturen moeten volgende defines toegevoegd worden:
USE_WS2812FX, PIN_WS2812FX, WS2812FX_NUMLEDS, WS2812FX_RGB_ORDER,
WS2812FX_BRIGHTNESS, WS2812FX_SPEED, WS2812FX_COLOR en WS2812FX_MODE

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

#if defined(CONFIG_IDF_TARGET_ESP32C3)
#define VOLTAGE_THRESHOLD 3.1 // onder dit voltage uit, om op hol slaan te vermijden op ESP32C3. Gemeten op batterij zelf.
#else
#define VOLTAGE_THRESHOLD 2.7 // onder dit voltage uit, om de batterij te beschermen, gemeten na de spanningsregelaar bij ESP8266.
#endif

#endif

#if defined(ENV_HOVERSERVO_ESP8266_ESP01_LEDPIN1_V0)

#define PIN_SERVO          0
#define PIN_MOTOR          3
#define PIN_LEDCONNECTIE   1

#define MOTORZ_TIME_UP 200 // ms om motor naar vol vermogen te brengen

// Pas de voltagefactor aan, dat is bij elke chip verschillend. Calibreer bv. met USB stroom die 3.3V op de chip moet geven
#define VOLTAGE_FACTOR 1060.0f 

#define LED_BRIGHTNESS_ON  LOW
#define LED_BRIGHTNESS_OFF HIGH

#elif defined(ENV_HOVERSERVO_ESP8266_ESP01_LEDPIN2_V0)

#define PIN_SERVO          1
#define PIN_MOTOR          3
#define PIN_LEDCONNECTIE   2

#define MOTORZ_TIME_UP 200 // ms om motor naar vol vermogen te brengen

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

#define MOTORZ_TIME_UP 200 // ms om motor naar vol vermogen te brengen

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

#define MOTORZ_TIME_UP 200 // ms om motor naar vol vermogen te brengen

// Pas de voltagefactor aan, dat is bij elke chip verschillend. Calibreer bv. met USB stroom die 3.3V op de chip moet geven
#define VOLTAGE_FACTOR 910.0f 

#define LED_BRIGHTNESS_ON  LOW
#define LED_BRIGHTNESS_OFF HIGH


#elif defined(ENV_HOVERSERVOGYRO_ESP8266_ESP01_LEDPIN2_V0)
#define USE_FASTIMU
#define FASTIMU_TYPE MPU6050
#define IMU_I2C_ADDRESS 0x68 // alternatief 0x69
#define GYRO_DIRECTION GYRO_DIRECTION_X
#define GYRO_REGELING_P         4.0
#define GYRO_REGELING_MAX_DRAAI 0.5
#define GYRO_REGELING_BIAS      1.0
#define GYRO_LPF_TF             0.050 // Tf in seconds
#define SERVO_ANTI_BIBBER       3.0

#define PIN_SERVO          1
#define PIN_MOTOR          3

#define PIN_LEDCONNECTIE   2 
#define PIN_LED_DUALUSE
#define PIN_SDA            2            
#define PIN_SCL            0

#define MOTORZ_TIME_UP 200 // ms om motor naar vol vermogen te brengen

// Pas de voltagefactor aan, dat is bij elke chip verschillend. Calibreer bv. met USB stroom die 3.3V op de chip moet geven
#define VOLTAGE_FACTOR 1060.0f 

#define LED_BRIGHTNESS_ON  LOW
#define LED_BRIGHTNESS_OFF HIGH


#elif defined(ENV_HOVERSERVOGYRO_ESP32C3_SUPERMINI_WS2812FX_V0) // GY-521 (2024 Fri3d)
// #define DEBUG_SERIAL Serial

#define USE_FASTIMU
#define FASTIMU_TYPE MPU6050
#define IMU_I2C_ADDRESS 0x68 // alternatief 0x69
#define GYRO_DIRECTION GYRO_DIRECTION_Y
#define GYRO_FLIP
#define GYRO_REGELING_P         4.0
#define GYRO_REGELING_MAX_DRAAI 0.5
#define GYRO_REGELING_BIAS      1.0
#define SERVO_ANTI_BIBBER       3.0
#define GYRO_LPF_TF             0.050 // Tf in seconds

#define PIN_SERVO          5
#define PIN_MOTOR          6

#define PIN_LEDCONNECTIE   8
#define PIN_LED_DUALUSE
#define PIN_SDA            3            
#define PIN_SCL            4

#define PIN_BATMONITOR     1

#define MOTORZ_TIME_UP 2000 // ms om motor naar vol vermogen te brengen

#define LED_BRIGHTNESS_ON  LOW
#define LED_BRIGHTNESS_OFF HIGH

#define USE_WS2812FX
#define PIN_WS2812FX       8
#define WS2812FX_NUMLEDS    6
#define WS2812FX_RGB_ORDER  NEO_BGR //voor "fairy" type
#define WS2812FX_BRIGHTNESS 200 // 0 .. 255
#define WS2812FX_SPEED 1000 // in ms
#define WS2812FX_COLOR 0x007BFF // blauw, 0x007BFF geeft violet en blauw met 0xFF0000 op fairy type met NEO_GRB?
#define WS2812FX_MODE FX_MODE_FADE // FX_MODE_BLINK, ... Volledige lijst op https://github.com/kitesurfer1404/WS2812FX/blob/master/src/modes_arduino.h

#define VOLTAGE_FACTOR 850.0f

#elif defined(ENV_HOVERSERVOGYRO_ESP32C3_SUPERMINI_WS2812FX_V1) // LSM6DS3 
// #define DEBUG_SERIAL Serial

#define USE_FASTIMU
#define FASTIMU_TYPE LSM6DS3
#define IMU_I2C_ADDRESS 0x6B
#define GYRO_DIRECTION GYRO_DIRECTION_Y
#define GYRO_FLIP
#define GYRO_REGELING_P         4.0
#define GYRO_REGELING_MAX_DRAAI 0.5
#define GYRO_REGELING_BIAS      1.0
// #define SERVO_ANTI_BIBBER       3.0
#define GYRO_LPF_TF             0.050 // Tf in seconds

#define PIN_SERVO          5
#define PIN_MOTOR          6

#define PIN_LEDCONNECTIE   8
#define PIN_LED_DUALUSE
#define PIN_SDA            3            
#define PIN_SCL            4

// #define PIN_BATMONITOR     1

#define MOTORZ_TIME_UP 2000 // ms om motor naar vol vermogen te brengen

#define LED_BRIGHTNESS_ON  LOW
#define LED_BRIGHTNESS_OFF HIGH

#define USE_WS2812FX
#define PIN_WS2812FX       1
#define WS2812FX_NUMLEDS    6
#define WS2812FX_RGB_ORDER  NEO_BGR //voor "fairy" type
#define WS2812FX_BRIGHTNESS 255 // 0 .. 255
#define WS2812FX_SPEED 1000 // in ms
#define WS2812FX_COLOR 0x007BFF // blauw, 0x007BFF geeft violet en blauw met 0xFF0000 op fairy type met NEO_GRB?
#define WS2812FX_MODE FX_MODE_FADE // FX_MODE_BLINK, ... Volledige lijst op https://github.com/kitesurfer1404/WS2812FX/blob/master/src/modes_arduino.h

#define VOLTAGE_FACTOR 850.0f

#elif defined ENV_USER_DEFINED
// defines staan buiten de code

#else
// Geen ENV_XX geselecteerd
#error "Defineer één van bovenstaande defines"

#endif
