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

#define DEBUG

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
  const int cFadeSteps = 400;
  const char *cCaption = "• H801 RGBWW Controller:";
  static const uint16_t /*PROGMEM*/ gamma8[];

  enum ANIMATIONMODE
  {
    DONE = 0, // fading finished, nothing to do
    STARTFADE,
    DOFADE
  };

  enum COLORINDEX // Enum for array indices
  {
    RED = 0,
    GREEN,
    BLUE,
    WHITE1,
    WHITE2
  };

  uint8_t _pins[5] = {DIMMER_1_PIN_RED, DIMMER_2_PIN_GREEN, DIMMER_3_PIN_BLUE, DIMMER_4_PIN_W1, DIMMER_5_PIN_W2};
  uint8_t _curValue[5] = {0, 0, 0, 0, 0}; // The current percent value of the dimmer
  uint8_t _endValue[5] = {0, 0, 0, 0, 0}; // The target percent value of the dimmer
  int8_t _step[5] = {0, 0, 0, 0, 0};

  uint16_t _loopCount = 0;

  unsigned long _lastLoop = 0;
  unsigned long _transitionTime = 25; // one step each _transitionTime in milliseconds

  ANIMATIONMODE _animationMode = DONE;

  void printCaption();
  int tryStrToInt(const String &value, const int maxvalue = 100);

  int calculateStep(int curValue, int endValue);
  void calculateSteps();
  int calculateVal(int step, int val, int i);
  void crossFade();

  void setColor();

protected:
  virtual bool handleInput(const String &property, const HomieRange &range, const String &value) override;
  virtual void setup() override;
  virtual void loop() override;

public:
  H801Node(const char *name);

  void beforeSetup();
  void setupHandler();
};
