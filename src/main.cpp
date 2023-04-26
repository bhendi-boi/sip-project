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
AsyncWebSocket ws("/ws");

// * variables to store previous and current state of pin
int pinStateCurrent[] = {LOW, LOW};
int pinStatePrevious[] = {LOW, LOW};

// * variable to store status of the fan
// * HIGH for on and LOW for viceversa
int status;
int prevStatus;
int manuallyTurnedOff;
int noOfTries = 0;

// * custom functions written to abstract the logic
void logData()
{
  Serial.printf("noOfTries: %d\n", noOfTries);
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
  noOfTries = 0;
  Serial.println("Humans detected!, Fan is turned ON");
  digitalWrite(RELAY_INPUT, HIGH);
}

void turnOff()
{
  if (manuallyTurnedOff)
  {
    digitalWrite(RELAY_INPUT, LOW);
    return;
  }
  noOfTries++;
  if (noOfTries >= 3)
  {
    Serial.println("No sign of Humans!, Fan is turned OFF");
    digitalWrite(RELAY_INPUT, LOW);
  }
}

int areHumansPresent()
{
  // * pinState change high to low
  if ((pinStatePrevious[0] == HIGH && pinStateCurrent[0] == LOW) || (pinStatePrevious[1] == HIGH && pinStateCurrent[1] == LOW))
  {
    return 0;
  }
  return 1;
}

void notifyClients()
{
  ws.textAll(status ? "true" : "false");
}

// * Functions for handling exchange of data using websocket
// * https://m1cr0lab-esp32.github.io/remote-control-with-websocket/websocket-data-exchange/
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    if (strcmp((char *)data, "toggle") == 0)
    {
      status = !status;
      manuallyTurnedOff = !status;
      if (manuallyTurnedOff)
      {
        Serial.println("Fan is manually turned off");
        turnOff();
      }
      else
      {
        turnOn();
      }
      notifyClients();
    }
  }
}

void onEvent(AsyncWebSocket *server,
             AsyncWebSocketClient *client,
             AwsEventType type,
             void *arg,
             uint8_t *data,
             size_t len)
{

  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

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
  initWebSocket();
}

// * loop
void loop()
{
  ws.cleanupClients();
  if (manuallyTurnedOff)
  {
  }
  else
  {
    logData();
    remeberingPreviosState();
    readingInput();

    prevStatus = status;
    status = areHumansPresent();
    if (!status)
    {
      turnOff();
      delay(10000);
    }
    else
    {
      if (!prevStatus)
      {
        turnOn();
      }
    }
  }
  delay(5000);
}
