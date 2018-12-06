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

// #define DEBUG // uncomment this line if you want to see the start and end values in fading
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

  // Settable properties
  const char *cPropOn = "on"; // Not really settable, always reports true, because the controller is connected to a physical switch
  const char *cPropEffectMode = "effect";
  const char *cPropRGB = "rgb";
  const char *cPropHSV = "hsv";
  const char *cPropSpeed = "speed";
  const char *cPropRed = "red";
  const char *cPropGreen = "green";
  const char *cPropBlue = "blue";
  const char *cPropWhite1 = "white1";
  const char *cPropWhite2 = "white2";
  // Cycles for different animations. The milliseconds per step are calculated from _transitionTime
  const int cFadeSteps = 4 * 100; // Number of steps needed for a proper crossfade.
  const int cFastCycleSteps = 100;
  const int cSlowCycleSteps = cFastCycleSteps / 20;
  const int cStartTransitionTime = 5000;

  static const uint16_t /*PROGMEM*/ gamma8[];

  enum EFFECTMODE
  {
    emNONE = 0,  // set color immediately
    emFADE,      // fade to target color and stop
    emFASTCYCLE, // cycle through the color wheel continuously
    emSLOWCYCLE
  };

  enum EFFECTSTATE
  {
    esDONE = 0, // fading finished, nothing to do
    esSTARTFADE,
    esDOFADE
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
  uint8_t _curValue[5] = {60, 60, 60, 60, 60}; // The current percent value of the dimmer. Also the power on values 
  uint8_t _endValue[5] = {0, 0, 0, 0, 0};      // The target percent value of the dimmer
  int8_t _step[5] = {0, 0, 0, 0, 0};           // Every _step milliseconds the corresponding dimmer value is incremented or decremented

  struct CHSV _curHsv; // The current HSV values

  uint16_t _loopCount = 0;

  EFFECTMODE _effectMode = emFADE;
  EFFECTSTATE _effectState = esDONE;

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
  void setEndColor(bool done);

  void setEffectMode(EFFECTMODE mode);
  void setRGBState();

  void loopCycle();
  void loopFade();

protected:
  virtual bool handleInput(const String &property, const HomieRange &range, const String &value) override;
  virtual void setup() override;
  virtual void loop() override;

public:
  H801Node(const char *name);

  void beforeSetup();
  void blackout();
};
