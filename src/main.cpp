#define FW_NAME "homie-h801"
#define FW_VERSION "1.0.1"

#include <Homie.h>
#include "ota.hpp"
#include "welcome.cpp"
#include "H801Node.hpp"

void beforeOta();

OtaLogger ota(beforeOta);
Welcome welcome(FW_NAME, FW_VERSION);

H801Node h801node("h801");

void beforeOta()
{
  h801node.blackout();
}

void setupHandler()
{
  ota.setup();
}

void loopHandler()
{
  ota.loop();
}

void setup()
{
  Homie_setFirmware(FW_NAME, FW_VERSION);

  // !!!Attention!!!
  // The H801 controller uses serial1 for output
  Serial1.begin(SERIAL_SPEED);
  Homie.setLoggingPrinter(&Serial1);

  // The green LED on H801 is connected to Serial
  Homie.setLedPin(LED_PIN_RED, LOW);

  Homie.disableResetTrigger();

  welcome.show();

  h801node.beforeSetup();

  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);
  Homie.setup();
}

void loop()
{
  Homie.loop();
}