/*
 * h801node.hpp
 * Homie node for a h801 five channel LED controller
 * usually RGBWW
 *
 * Version: 1.0
 * Author: Lübbe Onken (http://github.com/luebbe)
 */

#include <H801Node.hpp>

// Gamma correction, for details see https://learn.adafruit.com/led-tricks-gamma-correction/
// This table maps [0%-100%] to [0-1024] (PWMRANGE of ESP8266's arduino.h)
// See tools directory for C++ program to create table for different ranges

const uint16_t /*PROGMEM*/ H801Node::gamma8[] = {
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 3, 4, 5,
    6, 7, 8, 10, 11, 13, 15, 17, 19, 21, 24, 26, 29, 32, 35, 39,
    42, 46, 50, 54, 59, 63, 68, 73, 79, 84, 90, 96, 103, 109, 116, 124,
    131, 139, 147, 155, 164, 173, 182, 192, 202, 212, 223, 234, 245, 257, 269, 281,
    293, 307, 320, 334, 348, 362, 377, 392, 408, 424, 441, 458, 475, 493, 511, 529,
    548, 568, 587, 608, 628, 650, 671, 693, 716, 739, 762, 786, 811, 836, 861, 887,
    913, 940, 968, 996, 1024};

H801Node::H801Node(const char *name)
    : HomieNode(name, "RGBWW Controller")
{
}

void H801Node::initController()
{
  pinMode(DIMMER_1_PIN_RED, OUTPUT);
  pinMode(DIMMER_2_PIN_GREEN, OUTPUT);
  pinMode(DIMMER_3_PIN_BLUE, OUTPUT);
  pinMode(DIMMER_4_PIN_W1, OUTPUT);
  pinMode(DIMMER_5_PIN_W2, OUTPUT);
  setRGB(0, 0, 0);
  setW1(0);
  setW2(0);
}

int H801Node::tryStrToInt(const String &value, const int maxvalue)
{
  int _value = value.toInt();
  if (_value < 0)
  {
    _value = 0;
  }
  if (_value > maxvalue)
  {
    _value = maxvalue;
  }
  return _value;
}

bool H801Node::handleInput(const String &property, const HomieRange &range, const String &value)
{
  if (property == "command")
  {
    //parseJsonCommand(value);
  }
  else if (property == "speed")
  {
  }
  else if (property == "red")
  {
    _red = tryStrToInt(value);
  }
  else if (property == "green")
  {
    _green = tryStrToInt(value);
  }
  else if (property == "blue")
  {
    _blue = tryStrToInt(value);
  }
  else if (property == "white1")
  {
    _white1 = tryStrToInt(value);
  }
  else if (property == "white2")
  {
    _white2 = tryStrToInt(value);
  }

  return true;
}

void H801Node::printCaption()
{
  Homie.getLogger() << cCaption << endl;
}

void H801Node::setLED(uint8_t ledPin, uint8_t value)
{
  uint16_t gammaValue = gamma8[value];
  analogWrite(ledPin, gammaValue);
}

void H801Node::setRGB(uint8_t R, uint8_t G, uint8_t B)
{
  setLED(DIMMER_1_PIN_RED, R);
  setLED(DIMMER_2_PIN_GREEN, G);
  setLED(DIMMER_3_PIN_BLUE, B);
}

void H801Node::setW1(uint8_t W1)
{
  setLED(DIMMER_4_PIN_W1, W1);
}

void H801Node::setW2(uint8_t W2)
{
  setLED(DIMMER_5_PIN_W2, W2);
}

void H801Node::loop()
{
  if(_transitionTime == 0) {
    setRGB(_red, _green, _blue);
    setW1(_white1);
    setW2(_white2);
  }
  else {

  }
}

void H801Node::beforeSetup()
{
  initController();
  advertise("command").settable(); // Parses a complete JSON command, so that every property can be set in one MQTT message
  advertise("speed").settable();   // Transition speed for colors and effects
  advertise("red").settable();     // RGB values from 0% to 100%
  advertise("green").settable();   //
  advertise("blue").settable();    //
  // advertise("hue").settable();        // hue from 0° to 360°
  // advertise("saturation").settable(); // from 0% to 100%
  // advertise("value").settable();      // from 0% to 100%
  advertise("white1").settable();     // White channels from 0% to 100%
  advertise("white2").settable();     //
  // advertise("effect").settable();     //
}

void H801Node::setupHandler()
{
  //  setProperty(cVoltageUnit).send("V");
}

void H801Node::setup()
{
  printCaption();
}
