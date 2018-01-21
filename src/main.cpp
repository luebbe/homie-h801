#define FW_NAME "homie-h801"
#define FW_VERSION "1.0.0"

#include <Homie.h>
#include <ArduinoOTA.h>

#include "welcome.cpp"
#include "H801Node.hpp"

Welcome welcome(FW_NAME, FW_VERSION);

H801Node h801node("h801");

void setupHandler()
{
  ArduinoOTA.setHostname(Homie.getConfiguration().deviceId);
  ArduinoOTA.begin();
}

void loopHandler()
{
  ArduinoOTA.handle();
}

void setup()
{
  Homie_setFirmware(FW_NAME, FW_VERSION);

  // 
  Serial1.begin(SERIAL_SPEED);
  Homie.setLoggingPrinter(&Serial1);
  Homie.setLedPin(LED_PIN_RED, HIGH);

  welcome.show();

  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);
  Homie.setup();
}

void loop()
{
  Homie.loop();
}