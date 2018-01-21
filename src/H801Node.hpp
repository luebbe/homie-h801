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
#include <ArduinoJson.h>

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
  static const uint16_t /*PROGMEM*/ gamma8[];
  uint8_t _red = 0;
  uint8_t _green = 0;
  uint8_t _blue = 0;
  uint8_t _white1 = 0;
  uint8_t _white2 = 0;
  uint16_t _transitionTime = 0;

  void initController();
  void printCaption();
  int tryStrToInt(const String &value, const int maxvalue = 100);

  void setLED(uint8_t ledPin, uint8_t value);
  void setRGB(uint8_t R, uint8_t G, uint8_t B);
  void setW1(uint8_t W1);
  void setW2(uint8_t W2);

protected:
  virtual bool handleInput(const String &property, const HomieRange &range, const String &value) override;
  virtual void setup() override;
  virtual void loop() override;

public:
  H801Node(const char *name);

  void beforeSetup();
  void setupHandler();
};
