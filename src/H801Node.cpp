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
int H801Node::toByte(const int value, const int factor)
{
  return value * 255 / factor;
}

int H801Node::toPercent(const int value, const int factor)
{
  return value * factor / 255;
}

int H801Node::tryStrToInt(const String &value, const int maxvalue)
{
  return constrain(value.toInt(), 0, maxvalue);
}

void H801Node::fadeToHSVConvert(int h, int s, int v)
{
  // The FastLED hsv<->rgb converter works with values from 0..255
  // Input values from OpenHAB
  // H = 0°..360°
  // S = 0%..100%
  // V = 0%..100%
  // _endValue[s] are percent values again

  // Convert to byte value range
  _curHsv.hue = toByte(h, 360);
  _curHsv.sat = toByte(s);
  _curHsv.val = toByte(v);

  fadeToHSV(_curHsv);
}

void H801Node::fadeToHSV(CHSV hsvIn)
{
  struct CRGB rgbOut;

  // do the math
  hsv2rgb_rainbow(hsvIn, rgbOut);

  // convert back to percent values
  _endValue[COLORINDEX::RED] = toPercent(rgbOut.red);
  _endValue[COLORINDEX::GREEN] = toPercent(rgbOut.green);
  _endValue[COLORINDEX::BLUE] = toPercent(rgbOut.blue);

  _effectState = esSTARTFADE;
}

void H801Node::fadeToRGB()
{
  struct CRGB rgbIn;
  rgbIn.r = toByte(_endValue[COLORINDEX::RED]);
  rgbIn.g = toByte(_endValue[COLORINDEX::GREEN]);
  rgbIn.b = toByte(_endValue[COLORINDEX::BLUE]);

  // Convert to HSV in order to have the current HSV value available for
  // color cycling
  _curHsv = rgb2hsv_approximate(rgbIn);

  _effectState = esSTARTFADE;
}

bool H801Node::parseHSV(const String &value)
{
  // Expects H,S,V as comma separated values
  int h, s, v;

  if (sscanf(value.c_str(), "%d,%d,%d", &h, &s, &v) == 3)
  {
    fadeToHSVConvert(h, s, v);
    return true;
  }
  return false;
}

bool H801Node::parseRGB(const String &value)
{
  // Expects R,G,B as comma separated values
  int r, g, b;

  if (sscanf(value.c_str(), "%d,%d,%d", &r, &g, &b) == 3)
  {
    _endValue[COLORINDEX::RED] = r;
    _endValue[COLORINDEX::GREEN] = g;
    _endValue[COLORINDEX::BLUE] = b;
    fadeToRGB();
    return true;
  }
  return false;
}

bool H801Node::handleInput(const String &property, const HomieRange &range, const String &value)
{
  if (property == cPropOn)
  {
    // Not really settable, always reports true, because the controller is connected to a physical switch
    setProperty(cPropOn).send("true");
  }

  else if (property == cPropEffectMode)
  {
    if (value == "none")
    {
      setEffectMode(emNONE);
    }
    else if (value == "fade")
    {
      setEffectMode(emFADE);
    }
    else if (value == "fast")
    {
      setEffectMode(emFASTCYCLE);
    }
    else if (value == "slow")
    {
      setEffectMode(emSLOWCYCLE);
    }
  }

  else if (property == cPropSpeed)
  {
    _transitionTime = tryStrToInt(value);
  }
  else if (property == cPropHSV)
  {
    parseHSV(value);
  }
  else if (property == cPropRGB)
  {
    parseRGB(value);
  }
  else if (property == cPropRed)
  {
    _endValue[COLORINDEX::RED] = tryStrToInt(value);
    fadeToRGB();
  }
  else if (property == cPropGreen)
  {
    _endValue[COLORINDEX::GREEN] = tryStrToInt(value);
    fadeToRGB();
  }
  else if (property == cPropBlue)
  {
    _endValue[COLORINDEX::BLUE] = tryStrToInt(value);
    fadeToRGB();
  }
  else if (property == cPropWhite1)
  {
    _endValue[COLORINDEX::WHITE1] = tryStrToInt(value);
    _effectState = esSTARTFADE;
  }
  else if (property == cPropWhite2)
  {
    _endValue[COLORINDEX::WHITE2] = tryStrToInt(value);
    _effectState = esSTARTFADE;
  }

  return true;
}

void H801Node::printCaption()
{
  Homie.getLogger() << cCaption << endl;
}

void H801Node::setColor()
{
  for (int i = COLORINDEX::RED; i <= COLORINDEX::WHITE2; i++)
  {
    analogWrite(_pins[i], gamma8[_curValue[i]]);
  }
}

void H801Node::setEndColor(bool done)
{
  for (int i = COLORINDEX::RED; i <= COLORINDEX::WHITE2; i++)
  {
    _curValue[i] = _endValue[i];
  }
  setColor();

  if (done)
  {
    setRGBState();
    _effectState = esDONE;
  }
}

void H801Node::setEffectMode(EFFECTMODE mode)
{
#ifdef DEBUG
  Homie.getLogger() << "MODE " << _effectMode << "->" << mode << endl;
#endif
  switch (mode)
  {
  case emNONE:
    _effectMode = emNONE;
    // _waitTime = 0;
    setProperty(cPropEffectMode).send("none");
    break;
  case emFADE:
    _effectMode = emFADE;
    _waitTime = _transitionTime / cFadeSteps;
    setProperty(cPropEffectMode).send("fade");
    setRGBState();
    break;
  case emFASTCYCLE:
    _effectMode = emFASTCYCLE;
    _effectState = esDOFADE;
    _waitTime = _transitionTime / cFastCycleSteps;
    setProperty(cPropEffectMode).send("fast");
    break;
  case emSLOWCYCLE:
    _waitTime = _transitionTime / cSlowCycleSteps;
    _effectMode = emSLOWCYCLE;
    _effectState = esDOFADE;
    setProperty(cPropEffectMode).send("slow");
    break;
  }
}

void H801Node::setRGBState()
{
  char rgbState[20];
  sprintf(rgbState, "%d,%d,%d", _endValue[COLORINDEX::RED], _endValue[COLORINDEX::GREEN], _endValue[COLORINDEX::BLUE]);
  setProperty(cPropRGB).send(rgbState);
#ifdef DEBUG
  Homie.getLogger() << "Done RGB=" << rgbState << endl;
#endif
}

void H801Node::loop()
{
  if (_effectState != esDONE)
  {
    switch (_effectMode)
    {
    // case emNONE:
    //   Homie.getLogger() << "MODE = emNONE " << _effectState << endl;
    //   setEndColor(true);
    //   break;
    case emFADE:
      loopFade();
      break;
    case emFASTCYCLE:
      loopCycle();
      break;
    case emSLOWCYCLE:
      loopCycle();
      break;
    }
  }
}

void H801Node::loopCycle()
{
  unsigned long now = millis();
  // one step each _transitionTime in milliseconds
  if ((now - _lastLoop > _waitTime) || (_lastLoop == 0))
  {
    _lastLoop = now;
    _curHsv.hue = (_curHsv.hue + 1) % 255;
    fadeToHSV(_curHsv);
    setEndColor(false);
  }
}

void H801Node::loopFade()
{
  if (_effectState == esSTARTFADE)
  {
    calculateSteps();
    _loopCount = 0;
    _effectState = esDOFADE;
  };

  if (_transitionTime == 0)
  {
    setEndColor(true);
  }
  else
  {
    if (_effectState == esDOFADE)
    {
      unsigned long now = millis();
      // one step each _transitionTime in milliseconds
      if ((now - _lastLoop > _waitTime) || (_lastLoop == 0))
      {
        _lastLoop = now;
        crossFade();
      }
    }
  }
}

void H801Node::beforeSetup()
{
  for (int i = COLORINDEX::RED; i <= COLORINDEX::WHITE2; i++)
  {
    pinMode(_pins[i], OUTPUT);
    analogWrite(_pins[i], LOW);
  }
  advertise(cPropOn).settable();         // on/off = true/false
  advertise(cPropEffectMode).settable(); // Effect mode (none|fade|cycle)
  advertise(cPropSpeed).settable();         // Transition speed for colors and effects
  advertise(cPropHSV).settable();        // Expects H,S,V as comma separated values (Hue=0°..360°, Sat=0%..100%, Val=0%..100%)
  advertise(cPropRGB).settable();        // Expects R,G,B as comma separated values (R,G,B=0%..100%)
  // advertise(cPropRed).settable();     // RGB values from 0% to 100%
  // advertise(cPropGreen).settable();   //
  // advertise(cPropBlue).settable();    //
  // advertise("hue").settable();        // hue from 0° to 360°
  // advertise("saturation").settable(); // from 0% to 100%
  // advertise("value").settable();      // from 0% to 100%
  advertise(cPropWhite1).settable(); // White channels from 0% to 100%
  advertise(cPropWhite2).settable(); //
}

void H801Node::setup()
{
  printCaption();
}
// From https://www.arduino.cc/en/Tutorial/ColorCrossfader
//
// Modified to work with percent values for each dimmer
//
/* BELOW THIS LINE IS THE MATH -- YOU SHOULDN'T NEED TO CHANGE THIS FOR THE BASICS
*
* The program works like this:
* Imagine a crossfade that moves the red LED from 0-10,
*   the green from 0-5, and the blue from 10 to 7, in
*   ten steps.
*   We'd want to count the 10 steps and increase or
*   decrease color values in evenly stepped increments.
*   Imagine a + indicates raising a value by 1, and a -
*   equals lowering it. Our 10 step fade would look like:
*
*   1 2 3 4 5 6 7 8 9 10
* R + + + + + + + + + +
* G   +   +   +   +   +
* B     -     -     -
*
* The red rises from 0 to 10 in ten steps, the green from
* 0-5 in 5 steps, and the blue falls from 10 to 7 in three steps.
*
* We are working with percent values and there are 400 steps (100*4).
*
* To figure out how big a step there should be between one up- or
* down-tick of one of the LED values, we call calculateStep(),
* which calculates the absolute gap between the start and end values,
* and then divides that gap by 1020 to determine the size of the step
* between adjustments in the value.
*/
int H801Node::calculateStep(int curValue, int endValue)
{
  int step = endValue - curValue; // What's the overall gap?
  if (step)
  {                           // If its non-zero,
    step = cFadeSteps / step; //   divide by cFadeSteps
  }

  return step;
}

void H801Node::calculateSteps()
{
  for (int i = COLORINDEX::RED; i <= COLORINDEX::WHITE2; i++)
  {
    _step[i] = calculateStep(_curValue[i], _endValue[i]);
#ifdef DEBUG
    Homie.getLogger() << "Step[" << i << "] c=" << _curValue[i] << " e=" << _endValue[i] << " s=" << _step[i] << endl;
#endif
  }
}

/* The next function is calculateVal. When the loop value, i,
*  reaches the step size appropriate for one of the
*  colors, it increases or decreases the value of that color by 1.
*  (each dimmer channel is calculated separately.)
*/
int H801Node::calculateVal(int step, int val, int i)
{
  if ((step) && i % step == 0)
  { // If step is non-zero and its time to change a value,
    if (step > 0)
    { //   increment the value if step is positive...
      val += 1;
    }
    else if (step < 0)
    { //   ...or decrement it if step is negative
      val -= 1;
    }
  }

  // Defensive driving: make sure val stays in the range 0-100
  val = constrain(val, 0, 100);

  return val;
}

void H801Node::crossFade()
{
  if (_loopCount < cFadeSteps)
  {
    for (int i = COLORINDEX::RED; i <= COLORINDEX::WHITE2; i++)
    {
      _curValue[i] = calculateVal(_step[i], _curValue[i], _loopCount);
    }

    // Write current values to LED pins
    setColor();

#ifdef DEBUGFADE
    Homie.getLogger() << "Loop count: " << _loopCount;
    for (int i = COLORINDEX::RED; i <= COLORINDEX::WHITE2; i++)
    {
      Homie.getLogger() << " " << _curValue[i];
    }
    Homie.getLogger() << endl;
#endif

    _loopCount++;
  }
  else
  {
    _effectState = esDONE;
    setRGBState();
  }
}
