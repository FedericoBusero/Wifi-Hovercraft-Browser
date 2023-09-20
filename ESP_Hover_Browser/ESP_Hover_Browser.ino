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
#include <ESPAsyncWebServer.h> // https://github.com/me-no-dev/ESPAsyncWebServer

#include "GY521.h" // library; https://github.com/RobTillaart/GY521/
/*
   Wemos D1 mini:
   SCL: D1
   SDA: D2
   XDA: niet aangesloten
   XCL: niet aangesloten
   AD0: niet aangesloten  . De vice heeft 0x68 als I2C adres, waarschijnlijk wordt het 0x69 als je dit naar 3.3V verhoogt
   INT: niet aangesloten
   VCC: 3V
   GND: uiteraard
*/
GY521 sensor(0x68);

#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#include <AsyncTCP.h> // https://github.com/me-no-dev/AsyncTCP
#include <ESP32Servo.h> // https://github.com/madhephaestus/ESP32Servo 

#define DEBUG_SERIAL Serial

#define PWM_RANGE 255 // PWM range voor analogWrite (in ESP32Servo)

#define PIN_SERVO          18 
#define PIN_MOTOR          19 
#define PIN_LEDCONNECTIE1  LED_BUILTIN

#define LED_BRIGHTNESS_ON  HIGH
#define LED_BRIGHTNESS_OFF LOW

#else // ESP8266

ADC_MODE(ADC_VCC); // Nodig voor het inlezen van het voltage met ESP.getVcc

#include <ESP8266WiFi.h>
#include <Servo.h>
#include <ESPAsyncTCP.h> // https://github.com/me-no-dev/ESPAsyncTCP

#define PWM_RANGE 1023 // PWM range voor analogWrite
#define MOTOR_FREQ 400 // Frequentie van analogWrite in Hz, bepaalt het geluid van de motor

// #define MODE_ESP01

#ifdef MODE_ESP01

#define PIN_SERVO          0
#define PIN_MOTOR          3
#define PIN_LEDCONNECTIE1  1
#define PIN_LEDCONNECTIE2  2

// Pas de voltagefactor aan, dat is bij elke chip verschillend. Calibreer bv. met USB stroom die 3.3V op de chip moet geven
#define VOLTAGE_FACTOR 1060.0f 
#define VOLTAGE_THRESHOLD 2.4 // onder dit voltage valt de chip uit om de batterij te beschermen

#else // Wemos D1 mini, NodeMCU, ...
#define DEBUG_SERIAL Serial

#define PIN_SERVO          D5 // D5 = GPIO14  op NodeMCU & Wemos D1 mini
#define PIN_MOTOR          D8 // D8 = GPIO15 op NodeMCU & Wemos D1 mini
// De ingebouwde LED zit meestal op GPIO2 of GPIO16
#define PIN_LEDCONNECTIE1   2 
#define PIN_LEDCONNECTIE2   16 

// Pas de voltagefactor aan, dat is bij elke chip verschillend. Calibreer bv. met USB stroom die 3.3V op de chip moet geven
#define VOLTAGE_FACTOR 910.0f 
#define VOLTAGE_THRESHOLD 2.4 // onder dit voltage valt de chip uit om de batterij te beschermen

#endif // MODE_ESP01

#define LED_BRIGHTNESS_ON  LOW
#define LED_BRIGHTNESS_OFF HIGH

#endif // ARDUINO_ARCH_ESP32

#define USE_SOFTAP
#define WIFI_SOFTAP_CHANNEL 1 // 1-13
const char ssid[] = "hover-";
const char password[] = "12345678";

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
#define TIMEOUT_MS_MOTORS 2500L // Timeout om motoren uit veiligheid stil te leggen, na x milliseconden niks te hebben ontvangen
#define TIMEOUT_MS_LED 1L        // Aantal milliseconden dat LED blijft branden na het ontvangen van een boodschap
#define TIMEOUT_MS_VOLTAGE 1000L // Aantal milliseconden tussen update voltage

unsigned long last_activity_message;

// We maken een servo "object" aan om de servo aan te sturen.
Servo servo1;

// De minimum en maximum hoek van de servo, pas dit gerust aan als de servo de uitersten niet kan halen
// De waarden zijn minimaal 0, maximaal 180
#define SERVO_HOEK_MIN 0
#define SERVO_HOEK_MAX 180

int Servopositie_x;   // -180 .. 180
int TrimServopositie; // -180 .. 180
int doel_servohoek = (SERVO_HOEK_MIN + SERVO_HOEK_MAX) / 2;;

bool gyroBeschikbaar = false;

// Bij het verhogen van de snelheid van de motor, doen we dat in stappen om niet te bruusk op te trekken
// want dit kan de hovercraft onbestuurbaar maken of teveel stroom trekken waardoor de chip gaat resetten
// Pas gerust aan, 1=traag optrekken, 5=snel optrekken
// De waarde is minimaal 1 en maximaal 1023
#define MAX_MOTOR_SPEED_STAP 4
int motor_snelheid = 0;
int doel_motorsnelheid;
int max_motorsnelheid;
bool motors_halt;

void setup_pin_mode_output(int pin)
{
#ifdef ESP8266
  if ((pin == 1) || (pin == 3)) // RX & TX
  {
    pinMode (pin, FUNCTION_3);
  }
#endif
  pinMode(pin, OUTPUT);
}

void updateMotors()
{
  if (motors_halt)
  {
    analogWrite(PIN_MOTOR, 0);
  }
  else
  {
    float regelX;
    if (gyroBeschikbaar) // gyro
    {
      // "gyro"-regeling
      float Pfactor = 2.4; 
      float max_draai_factor = 2.0;

      sensor.read();
      float werkelijke_draaisnelheid = sensor.getGyroZ(); // getGyroX, getGyroY zijn ook mogelijk afhankelijk van positie sensor
      // sturen in verhouding tot afwijking, X van joystick bepaalt hoe snel we willen draaien
      float doel_draaisnelheid = (float)Servopositie_x * (-1.0) * max_draai_factor; 
      regelX = Pfactor * (werkelijke_draaisnelheid-doel_draaisnelheid); 
    }
    else
    {
      /* We berekenen naar welke doelpositie we de servo willen krijgen:
        we herschalen de som van de slider posities in de browser ( Servopositie_x (-180 .. 180) en TrimServopositie (-180 .. 180) )
        naar de minimum en maximum graden die de servo motor aankan (SERVO_HOEK_MIN .. SERVO_HOEK_MAX)
      */
      regelX = Servopositie_x;
    }
    doel_servohoek = map(regelX + TrimServopositie, -360, 360, SERVO_HOEK_MIN, SERVO_HOEK_MAX);
    servo1.write(constrain(doel_servohoek,SERVO_HOEK_MIN,SERVO_HOEK_MAX));  // We verplaatsen de servo naar de nieuwe positie doel_servohoek

    /*
      We gaan de motor nog niet onmiddellijk naar zijn snelheid doel_motorsnelheid brengen, maar elke keer dat we hier passeren
      gaan we ietsje dichter naar zijn doel. Daartoe mag hij elke keer maximum MAX_MOTOR_SPEED_STAP verhogen in snelheid
    */
    /*
#ifdef DEBUG_SERIAL
    DEBUG_SERIAL.print(F("doel_motorsnelheid="));
    DEBUG_SERIAL.println(doel_motorsnelheid);
    DEBUG_SERIAL.print(F("motor_snelheid="));
    DEBUG_SERIAL.println(motor_snelheid);
#endif
  */
    motor_snelheid = min(doel_motorsnelheid, motor_snelheid + MAX_MOTOR_SPEED_STAP);
  
    analogWrite(PIN_MOTOR, motor_snelheid); // We passen de snelheid van de motor aan naar zijn nieuwe snelheid motor_snelheid
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
  TrimServopositie = 0;
  Servopositie_x = 0;
  doel_servohoek = (SERVO_HOEK_MIN + SERVO_HOEK_MAX) / 2;

  motor_snelheid = 0;
  doel_motorsnelheid = 0;
  max_motorsnelheid = (300*PWM_RANGE)/360;
  motors_halt = false;  
  
  updateMotors();
}


void setup()
{
  setup_pin_mode_output(PIN_MOTOR);
  
#ifdef ESP8266
  // Aangezien de PWM range van analogWrite afhankelijk van de Arduino ESP8266 versie 255 ofwel 1023 is, stellen we de range vast in op 1023
  analogWriteRange(PWM_RANGE);
  
  // Verander de frequentie van analogWrite van 1000 Hz naar 400 Hz voor een aangenamer geluid
  analogWriteFreq(MOTOR_FREQ);
#endif
  analogWrite(PIN_MOTOR, 0); 

  delay(200); // 200 milliseconden wachten tot de stroom stabiel is
  
#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.begin(115200);
  DEBUG_SERIAL.println(F("\nHover Browser setup started"));
#endif

  setup_pin_mode_output(PIN_LEDCONNECTIE1);
#ifdef PIN_LEDCONNECTIE2
  setup_pin_mode_output(PIN_LEDCONNECTIE2);
#endif

  // De LEd flasht 2x om te tonen dat er een reboot is
  digitalWrite(PIN_LEDCONNECTIE1, LED_BRIGHTNESS_ON);
#ifdef PIN_LEDCONNECTIE2
  digitalWrite(PIN_LEDCONNECTIE2, LED_BRIGHTNESS_ON);
#endif
  delay(10);
  digitalWrite(PIN_LEDCONNECTIE1, LED_BRIGHTNESS_OFF);
#ifdef PIN_LEDCONNECTIE2
  digitalWrite(PIN_LEDCONNECTIE2, LED_BRIGHTNESS_OFF);
#endif
  delay(100);
  digitalWrite(PIN_LEDCONNECTIE1, LED_BRIGHTNESS_ON);
#ifdef PIN_LEDCONNECTIE2
  digitalWrite(PIN_LEDCONNECTIE2, LED_BRIGHTNESS_ON);
#endif
  delay(10);
  digitalWrite(PIN_LEDCONNECTIE1, LED_BRIGHTNESS_OFF);
#ifdef PIN_LEDCONNECTIE2
  digitalWrite(PIN_LEDCONNECTIE2, LED_BRIGHTNESS_OFF);
#endif


  // steering servo PWM
  setup_pin_mode_output(PIN_SERVO);
  /* we verbinden de servo met de gekozen servopin PIN_SERVO en leggen de uiterste signalen vast:
     een blokgolf signaal van 544ms stemt overeen met de servo-arm op 0° en 2400ms met 180°).
  */
  servo1.attach(PIN_SERVO, 544, 2400);

  
  init_motors();

  digitalWrite(PIN_LEDCONNECTIE1, LED_BRIGHTNESS_ON );
#ifdef PIN_LEDCONNECTIE2
  digitalWrite(PIN_LEDCONNECTIE2, LED_BRIGHTNESS_ON );
#endif

  // setup gyro module
  Wire.begin();

  delay(100);

  gyroBeschikbaar = false;
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
    sensor.setAccelSensitivity(2);  // 8g
    sensor.setGyroSensitivity(1);   // 500 degrees/s

    sensor.setThrottle();
#ifdef DEBUG_SERIAL
    DEBUG_SERIAL.println("start...");
#endif

    // set all calibration errors to zero
    sensor.gxe = 0;
    sensor.gye = 0;
    sensor.gze = 0;
    sensor.read();
  }

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
  for (int i = 0; i < 15 && WiFi.status() != WL_CONNECTED; i++) {
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

  webserver.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
#ifdef DEBUG_SERIAL
    DEBUG_SERIAL.println(F("on HTTP_GET: return"));
#endif
    request->send(200, "text/html", index_html);
  });

  webserver.begin();
  server.listen(82);
#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.print(F("Is server live? "));
  DEBUG_SERIAL.println(server.available());
#endif
  last_activity_message = millis();
}

void handleSliderMaxSpeed(int value)
{
#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.print(F("handleSliderMaxSpeed value="));
  DEBUG_SERIAL.println(value);
#endif
  max_motorsnelheid=map(value,0,360,PWM_RANGE/2,PWM_RANGE);

  updateMotors();
}

void handleSliderTrimServo(int value)
{
#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.print(F("handleSliderTrimServo value="));
  DEBUG_SERIAL.println(value);
#endif

  TrimServopositie = value;

  updateMotors();
}

void handleJoystick(int x, int y)
{
#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.print(F("handleJoystick x="));
  DEBUG_SERIAL.print(x);
  DEBUG_SERIAL.print(F(" y="));
  DEBUG_SERIAL.println(y);
#endif

  Servopositie_x = x;
  if (y <= 0)
  {
    doel_motorsnelheid = map(-y, 0, 180, 0, max_motorsnelheid);
  }
  else
  {
    doel_motorsnelheid = 0;
  }

  updateMotors();
}

void handle_message(websockets::WebsocketsMessage msg) {
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

  digitalWrite(PIN_LEDCONNECTIE1, LED_BRIGHTNESS_ON);
#ifdef PIN_LEDCONNECTIE2
  digitalWrite(PIN_LEDCONNECTIE2, LED_BRIGHTNESS_ON);
#endif
  last_activity_message = millis();

  switch (id)
  {
    case 0:       // ping
      break;
      
    case 1:
      handleJoystick(param1, param2);
      break;

    case 2: handleSliderMaxSpeed(param1);
      break;
      
    case 3: handleSliderTrimServo(param1);
      break;
      
  }
  if (motors_halt)
  {
    motors_resume();
  }
}

void onConnect()
{
  digitalWrite(PIN_LEDCONNECTIE1, LED_BRIGHTNESS_OFF);
#ifdef PIN_LEDCONNECTIE2
  digitalWrite(PIN_LEDCONNECTIE2, LED_BRIGHTNESS_OFF);
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
}

void updatestatusbar()
{
#ifdef ESP8266
  static unsigned long lastupdate_voltage = 0;
  unsigned long currentmillis = millis();
  char statusstr[50];

  if (currentmillis > lastupdate_voltage + TIMEOUT_MS_VOLTAGE)
  {
    lastupdate_voltage = currentmillis;
    float voltage = ESP.getVcc() / VOLTAGE_FACTOR;

    if (voltage >= VOLTAGE_THRESHOLD)
    {
      snprintf(statusstr, sizeof(statusstr), "%4.2f V", voltage);

      if (gyroBeschikbaar)
      {
        sensor.read();
        snprintf(statusstr, sizeof(statusstr), "%4.2f V gz:%4.2f", voltage, sensor.getGyroZ());
      } else
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
      ESP.deepSleep(0);
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
    digitalWrite(PIN_LEDCONNECTIE1, LED_BRIGHTNESS_OFF);
#ifdef PIN_LEDCONNECTIE2
    digitalWrite(PIN_LEDCONNECTIE2, LED_BRIGHTNESS_OFF);
#endif    
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
    if (sclient.available()) { // als return non-nul, dan is er een client geconnecteerd
      sclient.poll(); // als return non-nul, dan is er iets ontvangen

      updatestatusbar();

      updateMotors();
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
    if (is_connected) { 
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
    digitalWrite(PIN_LEDCONNECTIE1, (millis() % 1000) > 500 ? LOW : HIGH);
#ifdef PIN_LEDCONNECTIE2
    digitalWrite(PIN_LEDCONNECTIE2, (millis() % 1000) > 500 ? LOW : HIGH);
#endif
  }
  
  delay(2);
}
