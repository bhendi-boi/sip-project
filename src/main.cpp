#include <Arduino.h>
#define PIN_TO_SENSOR_1 19
#define PIN_TO_SENSOR_2 21
#define RELAY_INPUT 22

// * variables to store previous and current state of pin
int pinStateCurrent[] = {LOW, LOW};
int pinStatePrevious[] = {LOW, LOW};

// * variable to store status of the fan
// * HIGH for on and LOW for viceversa
int status;
int prevStatus;

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
  Serial.begin(9600);
  pinMode(PIN_TO_SENSOR_1, INPUT);
  pinMode(RELAY_INPUT, OUTPUT);
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
