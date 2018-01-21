/*
 * h801node.hpp
 * Homie node for a h801 five channel LED controller
 * usually RGBWW
 *
 * Version: 1.0
 * Author: Lübbe Onken (http://github.com/luebbe)
 */

#pragma once

#include <Homie.hpp>

#define DIMMER_1_PIN_RED 15
#define DIMMER_2_PIN_GREEN 13
#define DIMMER_3_PIN_BLUE 12
#define DIMMER_4_PIN_W1 14
#define DIMMER_5_PIN_W2 4

#define LED_PIN_GREEN 1
#define LED_PIN_RED 5

class H801Node : public HomieNode
{
private:
  const char *cCaption = "• H801 RGBWW Controller:";

  void initController();
  void printCaption();

  void setRGB(uint8_t R, uint8_t G, uint8_t B);
  void setW1(uint8_t w1);
  void setW2(uint8_t w2);

protected:
  virtual void setup() override;
  virtual void loop() override;

public:
  H801Node(const char *name);

  void setupHandler();
};
