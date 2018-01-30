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
#include "hsv2rgb.h"

// #define DEBUG     // uncomment this line if you want to see the start and end values in fading
// #define DEBUGFADE // uncomment this line if you want to see all the single steps in fading

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

  // Cycles for different animations. The milliseconds per step are calculated from _transitionTime
  const int cFadeSteps = 4 * 100; // Number of steps needed for a proper crossfade.
  const int cFastCycleSteps = 100;
  const int cSlowCycleSteps = cFastCycleSteps / 20;
  const int cStartTransitionTime = 5000;

  static const uint16_t /*PROGMEM*/ gamma8[];

  enum ANIMATIONMODE
  {
    FADEOFF = 0,
    FADEONCE,  // fade to target color and stop
    FASTCYCLE, // cycle through the color wheel continuously
    SLOWCYCLE
  };

  enum ANIMATIONSTATE
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
  int8_t _step[5] = {0, 0, 0, 0, 0};      // Every _step milliseconds the corresponding dimmer value is incremented or decremented

  struct CHSV _curHsv;  // The current HSV values

  uint16_t _loopCount = 0;

  ANIMATIONMODE _animationMode = FADEONCE;
  ANIMATIONSTATE _animationState = DONE;

  unsigned long _lastLoop = 0;
  unsigned long _transitionTime = cStartTransitionTime;
  unsigned long _waitTime = cStartTransitionTime / cFadeSteps;

  void printCaption();

  // Helper functions for conversion between different normalizations
  // Standard = Byte to Percent and vice versa
  int toByte(const int value, const int factor = 100);
  int toPercent(const int value, const int factor = 100);

  int tryStrToInt(const String &value, const int maxvalue = 100);

  void fadeToHSVConvert(int h, int s, int v);

  void fadeToHSV(CHSV hsvIn);
  void fadeToRGB();

  bool parseHSV(const String &value);
  bool parseRGB(const String &value);

  // Methods for calculating the fade steps and the fading itself
  int calculateStep(int curValue, int endValue);
  void calculateSteps();
  int calculateVal(int step, int val, int i);
  void crossFade();

  void setColor();

  void loopCycle();
  void loopFade();

protected:
  virtual bool handleInput(const String &property, const HomieRange &range, const String &value) override;
  virtual void setup() override;
  virtual void loop() override;

public:
  H801Node(const char *name);

  void beforeSetup();
};
