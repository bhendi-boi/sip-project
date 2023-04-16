#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#define PIN_TO_SENSOR_1 19
#define PIN_TO_SENSOR_2 21
#define RELAY_INPUT 22

// WiFi credentials
const char *WIFI_SSID = "bhendi";
const char *WIFI_PASS = "qy3opdib2";

AsyncWebServer server(80);

// * variables to store previous and current state of pin
int pinStateCurrent[] = {LOW, LOW};
int pinStatePrevious[] = {LOW, LOW};

// * variable to store status of the fan
// * HIGH for on and LOW for viceversa
int status;
int prevStatus;

void initSPIFFS()
{
  if (!SPIFFS.begin())
  {
    Serial.println("Cannot mount SPIFFS volume...");
    while (1)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
  }
}

void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.printf("Trying to connect [%s] ", WiFi.macAddress().c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.printf(" %s\n", WiFi.localIP().toString().c_str());
}
String processor(const String &var)
{
  return String(var == "STATE" && status ? "on" : "off");
}

void onRootRequest(AsyncWebServerRequest *request)
{
  request->send(SPIFFS, "/index.html", "text/html", false, processor);
}

void initWebServer()
{
  server.on("/", onRootRequest);
  server.serveStatic("/", SPIFFS, "/");
  server.begin();
}
void logData()
{
  Serial.println("");
}

void remeberingPreviosState()
{
  pinStatePrevious[0] = pinStateCurrent[0];
  pinStatePrevious[1] = pinStateCurrent[1];
}

void readingInput()
{
  pinStateCurrent[0] = digitalRead(PIN_TO_SENSOR_1);
  pinStateCurrent[0] = digitalRead(PIN_TO_SENSOR_2);
}

void turnOn()
{
  Serial.println("Motion detected!, Fan is turned ON");
  digitalWrite(RELAY_INPUT, LOW);
}

void turnOff()
{
  Serial.println("Motion stopped!, Fan is turned OFF");
  digitalWrite(RELAY_INPUT, HIGH);
}

int getMotionStatus()
{
  // * pinState change high to low
  // * motion stopped
  if ((pinStatePrevious[0] == HIGH && pinStateCurrent[0] == LOW) || (pinStatePrevious[1] == HIGH && pinStateCurrent[1] == LOW))
  {
    return 0;
  }
  return 1;
}

// * setup
void setup()
{

  pinMode(PIN_TO_SENSOR_1, INPUT);
  pinMode(PIN_TO_SENSOR_2, INPUT);
  pinMode(RELAY_INPUT, OUTPUT);

  Serial.begin(115200);
  delay(500);

  initSPIFFS();
  initWiFi();
  initWebServer();
}

// * loop
void loop()
{

  remeberingPreviosState();
  readingInput();

  prevStatus = status;
  status = getMotionStatus();
  if (!status)
  {
    turnOff();
  }
  else
  {
    if (!prevStatus)
    {
      turnOn();
    }
  }
}
