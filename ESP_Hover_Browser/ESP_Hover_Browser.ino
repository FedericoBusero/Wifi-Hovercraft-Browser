/*
 * Code voor het besturen van een hover mbv Wifi via de browser
 *
 * Hoe gebruiken?
 * Voeg wifi netwerk hover-xxxx toe met paswoord 12345678
 * Er is op dat netwerk uiteraard geen internet, dus "wifi behouden" aanvinken indien dat gevraagd wordt
 * Dan ga je naar de browser (chrome, firefox, safari, ..) naar de website http://192.168.4.1 of http://h.be
 *
 * De bovenste regel toont de connectie-status. Op ESP8266 wordt het voltage getoond tijdens de connectie, te calibreren met VOLTAGE_FACTOR
 * De bovenste slider dient om de servo te trimmen, de slider eronder om de maximum snelheid in te stellen,
 * met de joystick worden servo (links-rechts) en motor (midden-boven) bestuurd
 *
 */

#include <ArduinoWebsockets.h> // uit arduino library manager : "ArduinoWebsockets" by Gil Maimon, https://github.com/gilmaimon/ArduinoWebsockets
#include "config.h"

#ifdef USE_GY521
#include "GY521.h" // library; https://github.com/RobTillaart/GY521/ minimum versie 0.5.3
GY521 sensor(0x68);//of 0x69 naargelang bord

#endif

// Architectuur afhankelijke settings
#if defined(CONFIG_IDF_TARGET_ESP32C3)

#include <ESPAsyncWebSrv.h> // ESPAsyncWebSrv, version 1.2.6 by dvarrel : https://github.com/dvarrel/ESPAsyncWebSrv/
#include <WiFi.h>
#include <AsyncTCP.h>   // https://github.com/me-no-dev/AsyncTCP
#include <ESP32Servo.h> // https://github.com/madhephaestus/ESP32Servo

#define PWM_RANGE 255 // PWM range voor analogWrite

#elif defined(ARDUINO_ARCH_ESP32)

#include <ESPAsyncWebServer.h> // https://github.com/me-no-dev/ESPAsyncWebServer
#include <WiFi.h>
#include <AsyncTCP.h>   // https://github.com/me-no-dev/AsyncTCP
#include <ESP32Servo.h> // https://github.com/madhephaestus/ESP32Servo

#define PWM_RANGE 255 // PWM range voor analogWrite

#else // ESP8266
ADC_MODE(ADC_VCC); // Nodig voor het inlezen van het voltage met ESP.getVcc

#include <ESPAsyncWebServer.h> // https://github.com/me-no-dev/ESPAsyncWebServer
#include <ESP8266WiFi.h>
#include <Servo.h>
#include <ESPAsyncTCP.h> // https://github.com/me-no-dev/ESPAsyncTCP

#define PWM_RANGE 1023 // PWM range voor analogWrite

#endif // ARDUINO_ARCH_ESP32

#define USE_SOFTAP
const char ssid[] = WIFI_SOFTAP_SSID_PREFIX;
const char password[] = WIFI_SOFTAP_PASSWORD;

#ifdef USE_SOFTAP
#include <DNSServer.h>
DNSServer dnsServer;
#endif

#include "hovercontrol_html.h" // Deze code niet verplaatsen naar de ino file, want de preprocessor kan die overhoop halen

using namespace websockets;
WebsocketsServer server;
AsyncWebServer webserver(80);
WebsocketsClient sclient;

// timeoutes
#define TIMEOUT_MS_MOTORS 1200L   // Timeout om motoren uit veiligheid stil te leggen, na x milliseconden niks te hebben ontvangen, moet groter zijn dan retransmit in html code
#define TIMEOUT_MS_LED 1L         // Aantal milliseconden dat LED blijft branden na het ontvangen van een boodschap
#define TIMEOUT_MS_VOLTAGE 10000L // Aantal milliseconden tussen update voltage

unsigned long last_activity_message;

#include "Easer.h"

#define SERVO_SWEEP_TIME 200 // in ms

// We maken een servo "object" aan om de servo aan te sturen.
Servo servo1;

// De minimum en maximum hoek van de servo, pas dit gerust aan als de servo de uitersten niet kan halen
// De waarden zijn minimaal 0, maximaal 180
#define SERVO_HOEK_MIN 0
#define SERVO_HOEK_MAX 180

#define SERVO_HOEK_MIN_NOTRIM 35
#define SERVO_HOEK_MAX_NOTRIM 145
#define SERVO_HOEK_MID ((SERVO_HOEK_MIN_NOTRIM + SERVO_HOEK_MAX_NOTRIM) / 2)

int ui_joystick_x;
int ui_joystick_y;
int ui_slider1; // -180 .. 180
int ui_slider2; // 0 .. 360
Easer servohoek;

#define MOTOR_FREQ 400     // Frequentie van analogWrite in Hz, bepaalt het geluid van de motor
#define MOTORZ_TIME_UP 2000 // ms to go to ease to full power of a motor

Easer motorZ_snelheid;
bool motors_halt;

bool gyroBeschikbaar = false;

#ifdef USE_WS2812FX
#include <WS2812FX.h> // https://github.com/kitesurfer1404/WS2812FX
WS2812FX ws2812fx = WS2812FX(WS2812FX_NUMLEDS, PIN_WS2812FX, WS2812FX_RGB_ORDER + NEO_KHZ800);
#endif

void setup_pin_mode_output(int pin)
{
#ifdef ESP8266
  if ((pin == 1) || (pin == 3)) // RX & TX
  {
    pinMode(pin, FUNCTION_3);
  }
#endif
  pinMode(pin, OUTPUT);
}

#ifdef USE_GY521
float getGyro()
{
  float measured_value = 0.0;
  sensor.read();
  switch (GYRO_DIRECTION)
  {
  case GYRO_DIRECTION_X:
    measured_value = sensor.getGyroX();
    break;

  case GYRO_DIRECTION_Y:
    measured_value = sensor.getGyroY();
    break;

  case GYRO_DIRECTION_Z:
    measured_value = sensor.getGyroZ();
    break;
  }
#ifdef GYRO_FLIP
  measured_value = -measured_value;
#endif
  return measured_value;
}
#endif

float mapFloat(float value, float fromLow, float fromHigh, float toLow, float toHigh)
{
  return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

void updateMotors()
{
  if (motors_halt)
  {
    analogWrite(PIN_MOTOR, 0);
  }
  else
  {
    float regelX = 0.0;
    int doel_motorsnelheid;
    int max_motorsnelheid = map(ui_slider2, 0, 360, PWM_RANGE / 2, PWM_RANGE);

    if (ui_joystick_y <= 0)
    {
      doel_motorsnelheid = map(-ui_joystick_y, 0, 180, 0, max_motorsnelheid);
    }
    else
    {
      doel_motorsnelheid = 0;
    }

    if (gyroBeschikbaar && (doel_motorsnelheid > 5)) // gyro
    {
#ifdef USE_GY521
      // "gyro"-regeling
      const float Pfactor = GYRO_REGELING_P;
      const float max_draai_factor = GYRO_REGELING_MAX_DRAAI;
      const float bias = GYRO_REGELING_BIAS;

      float werkelijke_draaisnelheid = getGyro();
      // sturen in verhouding tot afwijking, X van joystick bepaalt hoe snel we willen draaien
      float doel_draaisnelheid = (float)ui_joystick_x * (-1.0) * max_draai_factor;
      regelX = Pfactor * (werkelijke_draaisnelheid - doel_draaisnelheid) - bias * doel_draaisnelheid;
      regelX = constrain(regelX, -180, 180);
#endif
    }
    else
    {
      regelX = (float)ui_joystick_x; // -180 .. 180
    }

    float TrimServopositie = mapFloat((float)ui_slider1, -180.0, 180.0, SERVO_HOEK_MIN - SERVO_HOEK_MIN_NOTRIM, SERVO_HOEK_MAX - SERVO_HOEK_MAX_NOTRIM);
    float doel_servohoek = mapFloat(regelX, -180.0, 180.0, (float)SERVO_HOEK_MIN_NOTRIM + TrimServopositie, (float)SERVO_HOEK_MAX_NOTRIM + TrimServopositie);
    servohoek.easeTo(constrain(doel_servohoek, SERVO_HOEK_MIN, SERVO_HOEK_MAX));
    servohoek.update();
#ifdef DEBUG_SERIAL
    // DEBUG_SERIAL.print(F("doel_servohoek="));
    // DEBUG_SERIAL.println(doel_servohoek);
    // DEBUG_SERIAL.print(F("servohoek="));
    // DEBUG_SERIAL.println(servohoek.getCurrentValue());
#endif
    servo1.writeMicroseconds(mapFloat(servohoek.getCurrentValue(), SERVO_HOEK_MIN, SERVO_HOEK_MAX, 544, 2400)); // We verplaatsen de servo naar de nieuwe positie servohoek

    /*
  #ifdef DEBUG_SERIAL
      DEBUG_SERIAL.print(F("doel_motorsnelheid="));
      DEBUG_SERIAL.println(doel_motorsnelheid);
  #endif
    */
    motorZ_snelheid.easeTo(doel_motorsnelheid);
    motorZ_snelheid.update();
    analogWrite(PIN_MOTOR, motorZ_snelheid.getCurrentValue()); // We passen de snelheid van de motor aan naar zijn nieuwe snelheid motorZ_snelheid
  }
}

void motors_pause()
{
#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.println(F("motors_pause"));
#endif

  motors_halt = true;
  updateMotors();
}

void motors_resume()
{
#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.println(F("motors_resume"));
#endif
  motors_halt = false;
  updateMotors();
}

void init_motors()
{
  ui_slider1 = 0;
  ui_slider2 = 240; // TODO, mag dit op 0 ??
  ui_joystick_x = 0;
  ui_joystick_y = 0;
  servohoek.setValue(SERVO_HOEK_MID);

  motorZ_snelheid.setValue(0);
  motors_halt = false;

  updateMotors();
}

void led_init()
{
#ifdef PIN_LEDCONNECTIE
  setup_pin_mode_output(PIN_LEDCONNECTIE);
#endif
}

void led_set(int ledmode, boolean except_when_dual_use)
{
#ifdef PIN_LED_DUALUSE
  if (except_when_dual_use)
    return;
#endif
#ifdef PIN_LEDCONNECTIE
  digitalWrite(PIN_LEDCONNECTIE, ledmode);
#endif
}

void init_voltage_monitor()
{
#if defined(ESP32) && defined(PIN_BATMONITOR)
  analogSetAttenuation(ADC_0db); // op de ESP32 varianten gebruiken we een externe weerstandbrug om het batterijvoltage te meten en zetten we de interne weerstandsbrug op "geen spanningsdeling"
#endif
}

float getVoltage()
{
#ifdef ESP8266
  return (float)ESP.getVcc() / (float)VOLTAGE_FACTOR; // op ESP8266 modules is VCC met de ene ADC pin verbonden
#elif defined(ESP32) && defined(PIN_BATMONITOR) && defined(VOLTAGE_FACTOR)
  return (float)analogRead(PIN_BATMONITOR) / (float)VOLTAGE_FACTOR; // op ESP32 modules is VBAT zelf via spanningsdeler met een ADC1 pin te verbinden (ADC2 niet gebruiken)
#else
  return (float)0;
#endif
}

void setup()
{
  setup_pin_mode_output(PIN_MOTOR);

#ifdef ESP8266
  // Aangezien de PWM range van analogWrite afhankelijk van de Arduino ESP8266 versie 255 ofwel 1023 is, stellen we de range vast in op 1023
  analogWriteRange(PWM_RANGE);

  // Verander de frequentie van analogWrite van 1000 Hz naar 400 Hz voor een aangenamer geluid
  analogWriteFreq(MOTOR_FREQ);
#if ARDUINO_ESP8266_MAJOR >= 3
  // workaround extreeme trage servo write vanaf Arduino 3: https://github.com/esp8266/Arduino/issues/8081
  enablePhaseLockedWaveform();
#endif
#elif defined(ESP32)
  // Verander de frequentie van analogWrite van 1000 Hz naar 400 Hz voor een aangenamer geluid
  analogWriteFrequency(MOTOR_FREQ);
#endif
  analogWrite(PIN_MOTOR, 0);

  delay(200); // 200 milliseconden wachten tot de stroom stabiel is

#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.begin(115200);
  DEBUG_SERIAL.println(F("\nHover Browser setup started"));
#endif

  led_init();

  // De LEd flasht 2x om te tonen dat er een reboot is
  led_set(LED_BRIGHTNESS_ON, false);
  delay(10);
  led_set(LED_BRIGHTNESS_OFF, false);
  delay(100);
  led_set(LED_BRIGHTNESS_ON, false);
  delay(10);
  led_set(LED_BRIGHTNESS_OFF, false);

  // steering servo PWM
  setup_pin_mode_output(PIN_SERVO);
  /* we verbinden de servo met de gekozen servopin PIN_SERVO en leggen de uiterste signalen vast:
     een blokgolf signaal van 544ms stemt overeen met de servo-arm op 0° en 2400ms met 180°).
  */
  servo1.attach(PIN_SERVO, 544, 2400);

  servohoek.begin(SERVO_HOEK_MID);
  servohoek.set_speed(SERVO_SWEEP_TIME / 180);
#ifdef SERVO_ANTI_BIBBER
  servohoek.setAntiBibber(SERVO_ANTI_BIBBER); // als bestemming <= x graden verwijderd, blijf gewoon staan
#endif

  motorZ_snelheid.begin(0, false);
  motorZ_snelheid.set_speed((float)MOTORZ_TIME_UP / (float)PWM_RANGE);

  init_motors();

  led_set(LED_BRIGHTNESS_ON, false);

  gyroBeschikbaar = false;

#ifdef USE_GY521
  // setup gyro module
#ifdef PIN_SDA
  Wire.begin(PIN_SDA, PIN_SCL);
#else
  Wire.begin();
#endif
  delay(100);
  for (int t = 0; t < 3; t++) // 3 keer proberen of gyro beschikbaar is
  {
    if (sensor.wakeup() == false)
    {
#ifdef DEBUG_SERIAL
      DEBUG_SERIAL.print(millis());
      DEBUG_SERIAL.println("\tCould not connect to GY521");
#endif
      delay(1000);
    }
    else
    {
      gyroBeschikbaar = true;
      break;
    }
  }

  if (gyroBeschikbaar)
  {
    sensor.setAccelSensitivity(2); // 8g
    sensor.setGyroSensitivity(1);  // 500 degrees/s
    sensor.setDLPFMode(6);         // 5 Hz Digital Low pass filter. Dit is extreem belangrijk, zoniet krijg je veel te veel ruis op de gyro meting t.g.v. trillingen van de motor

#ifdef DEBUG_SERIAL
    DEBUG_SERIAL.println("start...");
#endif

    // set all calibration errors to zero
    sensor.gxe = 0;
    sensor.gye = 0;
    sensor.gze = 0;
    sensor.read();
  }
#endif // USE_GY521
#ifdef PIN_LED_DUALUSE
  led_init();
#endif

  // Wifi instellingen
  WiFi.persistent(true);

  uint8_t macAddr[6];
  WiFi.macAddress(macAddr);

#if defined(USE_SOFTAP)
  WiFi.disconnect();
  /* zet een access point op */
  WiFi.mode(WIFI_AP);

  // ssidmac = ssid + 4 laatste hexadecimale waarden van het MAC-adres
  char ssidmac[33];
  sprintf(ssidmac, "%s%02X%02X", ssid, macAddr[4], macAddr[5]);
  WiFi.softAP(ssidmac, password, WIFI_SOFTAP_CHANNEL);
  IPAddress apIP = WiFi.softAPIP();
#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.print(F("SoftAP SSID="));
  DEBUG_SERIAL.println(ssidmac);
  DEBUG_SERIAL.print(F("IP: "));
  DEBUG_SERIAL.println(apIP);
#endif
  /* DNS server opzetten die alle domeinen vertaalt naar apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "h.be", apIP);

#else
  WiFi.softAPdisconnect(true);
  // host_name = "Hover-" + 6 hexadecimale waarden van het MAC-adres
  char host_name[33];
  sprintf(host_name, "Hover-%02X%02X%02X", macAddr[3], macAddr[4], macAddr[5]);
#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.print(F("Hostname: "));
  DEBUG_SERIAL.println(host_name);
#endif
#ifdef ESP8266
  WiFi.hostname(host_name);
#else // ESP32
  WiFi.setHostname(host_name);
#endif

  // Connect to wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Even wachten tot er verbinding is met het wifi netwerk
  for (int i = 0; i < 15 && WiFi.status() != WL_CONNECTED; i++)
  {
#ifdef DEBUG_SERIAL
    DEBUG_SERIAL.print('.');
#endif
    delay(1000);
  }

#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.print("\nWiFi connected - IP address: ");
  DEBUG_SERIAL.println(WiFi.localIP());
#endif

#endif

  webserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
               {
#ifdef DEBUG_SERIAL
    DEBUG_SERIAL.println(F("on HTTP_GET: return"));
#endif
    request->send(200, "text/html", index_html); });

  webserver.begin();
  server.listen(82);
#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.print(F("Is server live? "));
  DEBUG_SERIAL.println(server.available());
#endif

#ifdef USE_WS2812FX
  ws2812fx.init();
  ws2812fx.setBrightness(WS2812FX_BRIGHTNESS);
  ws2812fx.setSpeed(WS2812FX_SPEED);
  ws2812fx.setColor(WS2812FX_COLOR);
  ws2812fx.setMode(WS2812FX_MODE);
  ws2812fx.start();
#endif

  init_voltage_monitor();

  last_activity_message = millis();
}

void handle_message(websockets::WebsocketsMessage msg)
{
  const char *msgstr = msg.c_str();
  const char *p;

#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.println();
  DEBUG_SERIAL.print(F("handle_message "));
#endif

  int id = atoi(msgstr);
  int param1 = 0;
  int param2 = 0;

  p = strchr(msgstr, ':');
  if (p)
  {
    param1 = atoi(++p);
    p = strchr(p, ',');
    if (p)
    {
      param2 = atoi(++p);
    }
  }

#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.println(msgstr);
  DEBUG_SERIAL.print(F(" id = "));
  DEBUG_SERIAL.print(id);
  DEBUG_SERIAL.print(F(" param1 = "));
  DEBUG_SERIAL.print(param1);
  DEBUG_SERIAL.print(F(" param2 = "));
  DEBUG_SERIAL.println(param2);
#endif

  led_set(LED_BRIGHTNESS_ON, true);
  last_activity_message = millis();

  switch (id)
  {
  case 0: // ping
    break;

  case 1: // joystick
    ui_joystick_x = param1;
    ui_joystick_y = param2;
    updateMotors();
    break;

  case 2: // slider2
    ui_slider2 = param1;
    updateMotors();
    break;

  case 3: // slider1
    ui_slider1 = param1;
    updateMotors();
    break;
  }
  if (motors_halt)
  {
    motors_resume();
  }
}

void onConnect()
{
#ifdef PIN_LED_DUALUSE
  digitalWrite(PIN_LEDCONNECTIE, LOW);
#else
  led_set(LED_BRIGHTNESS_OFF, false);
#endif
#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.println(F("onConnect"));
#endif
  init_motors();
}

void onDisconnect()
{
#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.println(F("onDisconnect"));
#endif
  init_motors();
#ifdef PIN_LED_DUALUSE
  led_init();
#endif
}

void updatestatusbar()
{
#if defined(ESP8266) or ((defined(ESP32)) && defined(PIN_BATMONITOR))
  static unsigned long lastupdate_voltage = 0;
  unsigned long currentmillis = millis();
  char statusstr[50];

  if (currentmillis > lastupdate_voltage + TIMEOUT_MS_VOLTAGE)
  {
    lastupdate_voltage = currentmillis;
    float voltage = getVoltage();
    if (voltage >= VOLTAGE_THRESHOLD)
    {
      if (gyroBeschikbaar)
      {
#ifdef USE_GY521
        snprintf(statusstr, sizeof(statusstr), "%4.2f V gyro:%4.2f", voltage, getGyro());
#endif
      }
      else
      {
        snprintf(statusstr, sizeof(statusstr), "%4.2f V", voltage);
      }

#ifdef DEBUG_SERIAL
      DEBUG_SERIAL.print("Sending status: ");
      DEBUG_SERIAL.println(statusstr);
#endif
      sclient.send(statusstr);
    }
    else
    {
      snprintf(statusstr, sizeof(statusstr), "Battery low: %4.2f V. Shutting down", voltage);
#ifdef DEBUG_SERIAL
      DEBUG_SERIAL.print("Sending voltage: ");
      DEBUG_SERIAL.println(statusstr);
#endif
      sclient.send(statusstr);
      motors_pause();
      delay(20000); // boodschap wordt 20 seconden getoond in browser alvorens hij disconnecteert
      WiFi.mode(WIFI_OFF);
#ifdef ESP8266
      WiFi.forceSleepBegin();
#endif
      delay(1);
      while (1)
      {
        led_set(LED_BRIGHTNESS_ON, false);
        delay(10);
        led_set(LED_BRIGHTNESS_OFF, false);
        delay(5000);
      }
    }
  }
#endif
}

void loop()
{
  static int is_connected = 0;

#if defined(USE_SOFTAP)
  dnsServer.processNextRequest();
#endif

  if (millis() > last_activity_message + TIMEOUT_MS_LED)
  {
    led_set(LED_BRIGHTNESS_OFF, true);
  }

  if (millis() > last_activity_message + TIMEOUT_MS_MOTORS)
  {

#ifdef DEBUG_SERIAL
    DEBUG_SERIAL.println(F("Safety shutdown ..."));
#endif
    motors_pause();

    last_activity_message = millis();
  }

  if (is_connected)
  {
    if (sclient.available())
    {                 // als return non-nul, dan is er een client geconnecteerd
      sclient.poll(); // als return non-nul, dan is er iets ontvangen

      updatestatusbar();

      static unsigned long lastupdate_motors = 0;
      unsigned long currentmillis = millis();
      if (currentmillis > lastupdate_motors + 10) // min 10 ms tussen aanroepen updatemotors als er geen nieuwe waarde ontvangen is vanuit browser
      {
        lastupdate_motors = currentmillis;
        updateMotors();
      }
    }
    else
    {
      // niet langer geconnecteerd
      onDisconnect();
      is_connected = 0;
    }
  }
  if (server.poll()) // als er een nieuwe socket aangevraagd is
  {
#ifdef DEBUG_SERIAL
    DEBUG_SERIAL.print(F("server.poll is_connected="));
    DEBUG_SERIAL.println(is_connected);
#endif
    if (is_connected)
    {
      sclient.send("CLOSE");
    }

    sclient = server.accept();
#ifdef DEBUG_SERIAL
    DEBUG_SERIAL.println(F("Connection accept"));
#endif
    sclient.onMessage(handle_message);

    onConnect();
    is_connected = 1;
  }

  if (!is_connected)
  {
    led_set((millis() % 1000) > 500 ? LOW : HIGH, false);
  }
#ifdef USE_WS2812FX
  else
  {
    ws2812fx.service();
  }
#endif

  // delay(2);
}
