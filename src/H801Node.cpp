/*
 * h801node.hpp
 * Homie node for a h801 five channel LED controller
 * usually RGBWW
 *
 * Version: 1.0
 * Author: Lübbe Onken (http://github.com/luebbe)
 */

#include <H801Node.hpp>

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
  setRGB(0,0,0);
  setW1(0);
  setW2(0);
}

void H801Node::printCaption()
{
  Homie.getLogger() << cCaption << endl;
}

void H801Node::setRGB(uint8_t R, uint8_t G, uint8_t B)
{
}

void H801Node::setW1(uint8_t w1)
{
}

void H801Node::setW2(uint8_t w2)
{
}

void H801Node::loop()
{
}

void H801Node::setupHandler()
{
  //  setProperty(cVoltageUnit).send("V");
}

void H801Node::setup()
{
  printCaption();
  initController();
  // advertise(cVoltage);
  // advertise(cVoltageUnit);
}
