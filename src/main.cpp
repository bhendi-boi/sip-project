#include <Arduino.h>
#define PIN_TO_SENSOR 21

// * variables to store previous and current state of pin
int pinStateCurrent = LOW;
int pinStatePrevious = LOW;
void setup()
{
  Serial.begin(9600);
  pinMode(PIN_TO_SENSOR, INPUT);
}

void loop()
{
  pinStatePrevious = pinStateCurrent;
  pinStateCurrent = digitalRead(PIN_TO_SENSOR);

  // *  pin state change: LOW -> HIGH
  if (pinStatePrevious == LOW && pinStateCurrent == HIGH)
  {
    Serial.println("Motion detected!");
  }
  // *  pin state change: HIGH -> LOW
  else if (pinStatePrevious == HIGH && pinStateCurrent == LOW)
  {
    Serial.println("Motion stopped!");
  }
}
