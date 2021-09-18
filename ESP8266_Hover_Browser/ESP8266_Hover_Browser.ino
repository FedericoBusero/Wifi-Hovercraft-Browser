#include <Servo.h>
#include <ESP8266WiFi.h>

#include <ESPAsyncTCP.h> // https://github.com/me-no-dev/ESPAsyncTCP
#include <ArduinoWebsockets.h> // uit arduino library manager : https://github.com/gilmaimon/ArduinoWebsockets
#include <ESPAsyncWebServer.h> // https://github.com/me-no-dev/ESPAsyncWebServer

#define DEBUG_SERIAL Serial

#define USE_SOFTAP
const char ssid[] = "hover-";
const char password[] = "12345678";

#include "hovercontrol_html.h" // Do not put html code in .ino file to avoid preprocessor problems

using namespace websockets;
WebsocketsServer server;
AsyncWebServer webserver(80);
WebsocketsClient sclient;

// timeoutes
#define TIMEOUT_MS_MOTORS 30000L // Safety shutdown: motors will go to power off position after x milliseconds no message received
#define TIMEOUT_MS_LED 1L        // LED will light up for x milliseconds after message received
long last_activity_message;

#define PIN_SERVO          D2 // D2 = GPIO4  op NodeMCU & Wemos D1 mini

#define PIN_MOTOR          D8 // D8 = GPIO15 op NodeMCU & Wemos D1 mini

#define PIN_LEDCONNECTIE   16 // De ingebouwde LED zit op GPIO2 of GPIO16, dus aanpassen naar 2 als de LED niet werkt

#define LED_BRIGHTNESS_NO_CONNECTION LOW
#define LED_BRIGHTNESS_HANDLEMESSAGE LOW
#define LED_BRIGHTNESS_BOOT          LOW
#define LED_BRIGHTNESS_OFF           HIGH

// We maken een servo "object" aan om de servo aan te sturen.
Servo servo1;

// De minimum en maximum hoek van de servo, pas dit gerust aan als de servo de uitersten niet kan halen
// De waarden zijn minimaal 0, maximaal 180
#define SERVO_HOEK_MIN 0
#define SERVO_HOEK_MAX 180

// We verplaatsen de servo in stapjes om geen al te bruuske bewegingen te maken
// Pas dit gerust aan, 1=servo traag bewegen, 2=normaal en vanaf 4 gaat het heel snel.
// De waarde is minimaal 1 en maximaal 180, dan is er geen vertraging meer
#define SERVO_HOEK_STAP 2
int servohoek = (SERVO_HOEK_MIN + SERVO_HOEK_MAX) / 2;
int doel_servohoek;

// PWM range voor analogWrite
#define PWM_RANGE 1023

// Bij het verhogen van de snelheid van de motor, doen we dat in stappen om niet te bruusk op te trekken
// want dit kan de hovercraft onbestuurbaar maken of teveel stroom trekken waardoor de chip gaat resetten
// Pas gerust aan, 1=traag optrekken, 5=snel optrekken
// De waarde is minimaal 1 en maximaal 1023
#define MAX_MOTOR_SPEED_STAP 4
int motor_snelheid = 0;
int doel_motorsnelheid;
int max_motorsnelheid;

void setup_pin_mode_output(int pin)
{
#if defined(ESP8266) || defined(ESP32)
  if ((pin == 1) || (pin == 3)) // RX & TX
  {
    pinMode (pin, FUNCTION_3);
  }
#endif
  pinMode(pin, OUTPUT);
}

void updateMotors()
{
  /*
    We gaan de servo nog niet onmiddellijk naar zijn nieuwe positie doel_servohoek brengen, maar elke keer dat we hier passeren
    gaan we ietsje dichter naar zijn doel. Daartoe beperken we de verplaatsing t.o.v. de oude servohoek tot maximum SERVO_HOEK_STAP stappen
  */
  servohoek = constrain(doel_servohoek, servohoek - SERVO_HOEK_STAP, servohoek + SERVO_HOEK_STAP);

  servo1.write(servohoek);  // We verplaatsen de servo naar de nieuwe positie servohoek


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

void motors_halt()
{
#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.println(F("motors_halt"));
#endif

  // servo1.write(90);

  // up motor
  analogWrite(PIN_MOTOR, 0);
}



void init_values()
{
  doel_servohoek = 90;
  doel_motorsnelheid = 0;
  max_motorsnelheid = (300*PWM_RANGE)/360;
}


void setup()
{
#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.begin(115200);
  DEBUG_SERIAL.println(F("ESP32-CAMRobotController_ESP32Servo setup started"));
#endif

  setup_pin_mode_output(PIN_LEDCONNECTIE);

  // flash 2 time to show we are rebooting
  digitalWrite(PIN_LEDCONNECTIE, LED_BRIGHTNESS_BOOT);
  delay(10);
  digitalWrite(PIN_LEDCONNECTIE, LED_BRIGHTNESS_OFF);
  delay(100);
  digitalWrite(PIN_LEDCONNECTIE, LED_BRIGHTNESS_BOOT);
  delay(10);
  digitalWrite(PIN_LEDCONNECTIE, LED_BRIGHTNESS_OFF);


  // steering servo PWM
  setup_pin_mode_output(PIN_SERVO);
  /* we verbinden de servo met de gekozen servopin PIN_SERVO en leggen de uiterste signalen vast:
     een blokgolf signaal van 544ms stemt overeen met de servo-arm op 0° en 2400ms met 180°).
  */
  servo1.attach(PIN_SERVO, 544, 2400);

  setup_pin_mode_output(PIN_MOTOR);
  // Aangezien de PWM range van analogWrite afhankelijk van de Arduino ESP8266 versie 255 ofwel 1023 is, stellen we de range vast in op 1023
  analogWriteRange(PWM_RANGE);

  init_values();
  updateMotors();

  digitalWrite(PIN_LEDCONNECTIE, LED_BRIGHTNESS_NO_CONNECTION );

  // Wifi setup
#if defined(USE_SOFTAP)
  /* set up access point */
  WiFi.mode(WIFI_AP);

  // ssidmac = ssid + 6 hexadecimal values of MAC address
  char ssidmac[33];
  uint8_t macAddr[6];
  WiFi.softAPmacAddress(macAddr);
  sprintf(ssidmac, "%s%02X%02X%02X", ssid, macAddr[3], macAddr[4], macAddr[5]);
#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.print(F("SoftAP SSID="));
  DEBUG_SERIAL.println(ssidmac);
#endif
  // WiFi.softAP(ssid, password);
  WiFi.softAP(ssidmac, password);
#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.print(F("IP: "));
  DEBUG_SERIAL.println(WiFi.softAPIP());
#endif
#else
  // Connect to wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
#endif

  // Wait some time to connect to wifi
  for (int i = 0; i < 15 && WiFi.status() != WL_CONNECTED; i++) {
#ifdef DEBUG_SERIAL
    DEBUG_SERIAL.print('.');
#endif
    delay(1000);
  }

#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.println();
  DEBUG_SERIAL.println(F("WiFi connected"));
  DEBUG_SERIAL.println(F("IP address: "));
  DEBUG_SERIAL.println(WiFi.localIP());   // You can get IP address assigned to ESP
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

void handleSlider(int value)
{
#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.print(F("handleSlider value="));
  DEBUG_SERIAL.println(value);
#endif
  max_motorsnelheid=(PWM_RANGE*value)/360;

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

  doel_servohoek = map(x, -90, 90, 35, 135);
  if (y <= 0)
  {
    doel_motorsnelheid = map(-y, 0, 90, 0, max_motorsnelheid);
  }
  else
  {
    doel_motorsnelheid = 0;
  }

  updateMotors();
}

void handle_message(WebsocketsMessage msg) {
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

  digitalWrite(PIN_LEDCONNECTIE, LED_BRIGHTNESS_HANDLEMESSAGE);

  last_activity_message = millis();

  switch (id)
  {
    case 1:
      handleJoystick(param1, param2);
      break;

    case 2: handleSlider(param1);
      break;
  }

}

void onConnect()
{
  digitalWrite(PIN_LEDCONNECTIE, LED_BRIGHTNESS_OFF);

#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.println(F("onConnect"));
#endif
  init_values();
  updateMotors();
}

void onDisconnect()
{
#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.println(F("onDisconnect"));
#endif
  motors_halt();
}

void loop()
{
  static int is_connected = 0;

  if (millis() > last_activity_message + TIMEOUT_MS_LED)
  {
    digitalWrite(PIN_LEDCONNECTIE, LED_BRIGHTNESS_OFF);
  }

  if (millis() > last_activity_message + TIMEOUT_MS_MOTORS)
  {

#ifdef DEBUG_SERIAL
    DEBUG_SERIAL.println(F("Safety shutdown ..."));
#endif
    motors_halt();

    last_activity_message = millis();
  }


  if (is_connected)
  {
    if (sclient.available()) {
      sclient.poll();

      updateMotors();
    }
    else
    {
      // niet langer geconnecteerd
      onDisconnect();
      is_connected = 0;
    }
  }
  else // is_connected is 0
  {
    if (server.poll())
    {
#ifdef DEBUG_SERIAL
      DEBUG_SERIAL.println(F("server.poll"));
#endif
      sclient = server.accept();
#ifdef DEBUG_SERIAL
      DEBUG_SERIAL.println(F("Connection accept"));
#endif
      sclient.onMessage(handle_message);
      onConnect();
      is_connected = 1;
    }
    else
    {
      digitalWrite(PIN_LEDCONNECTIE, (millis() % 1000) > 500 ? LOW : HIGH);
    }
  }
}
