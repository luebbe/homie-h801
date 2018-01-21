#define FW_NAME "homie-h801"
#define FW_VERSION "1.0.0"

#include <Homie.h>
#include <ArduinoOTA.h>

#include "ota.hpp"
#include "welcome.cpp"
#include "H801Node.hpp"

OtaLogger ota;
Welcome welcome(FW_NAME, FW_VERSION);

H801Node h801node("h801");

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

  welcome.show();

  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);
  Homie.setup();
}

void loop()
{
  Homie.loop();
}