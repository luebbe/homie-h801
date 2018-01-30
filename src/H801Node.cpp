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

  fadeToHSV();
}

void H801Node::fadeToHSV()
{
  struct CRGB rgbOut;

  // do the math
  hsv2rgb_rainbow(_curHsv, rgbOut);

  // convert back to percent values
  _endValue[COLORINDEX::RED] = toPercent(rgbOut.red);
  _endValue[COLORINDEX::GREEN] = toPercent(rgbOut.green);
  _endValue[COLORINDEX::BLUE] = toPercent(rgbOut.blue);

  _animationState = STARTFADE;
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

  _animationState = STARTFADE;
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
  if (property == "animation")
  {
    if (value == "off")
    {
      _animationMode = FADEOFF;
      _endValue[COLORINDEX::RED] = 0;
      _endValue[COLORINDEX::GREEN] = 0;
      _endValue[COLORINDEX::BLUE] = 0;
      fadeToRGB();
    }
    if (value == "fade")
    {
      _animationMode = FADEONCE;
    }
    else if (value == "fast")
    {
      _animationMode = FASTCYCLE;
    }
    else if (value == "slow")
    {
      _animationMode = SLOWCYCLE;
    }
  }
  else if (property == "speed")
  {
    _transitionTime = tryStrToInt(value);
  }
  else if (property == "hsv")
  {
    parseHSV(value);
  }
  else if (property == "rgb")
  {
    parseRGB(value);
  }
  else if (property == "red")
  {
    _endValue[COLORINDEX::RED] = tryStrToInt(value);
    fadeToRGB();
  }
  else if (property == "green")
  {
    _endValue[COLORINDEX::GREEN] = tryStrToInt(value);
    fadeToRGB();
  }
  else if (property == "blue")
  {
    _endValue[COLORINDEX::BLUE] = tryStrToInt(value);
    fadeToRGB();
  }
  else if (property == "white1")
  {
    _endValue[COLORINDEX::WHITE1] = tryStrToInt(value);
    _animationState = STARTFADE;
  }
  else if (property == "white2")
  {
    _endValue[COLORINDEX::WHITE2] = tryStrToInt(value);
    _animationState = STARTFADE;
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

void H801Node::loop()
{
  switch (_animationMode)
  {
  case FADEOFF:
  case FADEONCE:
    _waitTime = _transitionTime / cFadeSteps;
    loopFade();
    break;
  case FASTCYCLE:
    _waitTime = _transitionTime / cFastCycleSteps;
    loopCycle();
    break;
  case SLOWCYCLE:
    _waitTime = _transitionTime / cSlowCycleSteps;
    loopCycle();
    break;
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
    fadeToHSV();
    for (int i = COLORINDEX::RED; i <= COLORINDEX::WHITE2; i++)
    {
      _curValue[i] = _endValue[i];
    }
    setColor();
  }
}

void H801Node::loopFade()
{
  if (_animationState == STARTFADE)
  {
    // char feedback[20];
    // sprintf(feedback, "%d,%d,%d", _endValue[COLORINDEX::RED], _endValue[COLORINDEX::GREEN], _endValue[COLORINDEX::BLUE]);
    // setProperty("rgb").send(feedback);
    calculateSteps();
    _loopCount = 0;
    _animationState = DOFADE;
  };

  if (_transitionTime == 0)
  {
    for (int i = COLORINDEX::RED; i <= COLORINDEX::BLUE; i++)
    {
      _curValue[i] = _endValue[i];
      setColor();
      _animationState = DONE;
    }
  }
  else
  {
    if (_animationState == DOFADE)
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
  advertise("animation").settable(); // Animation mode (fade|cycle)
  advertise("speed").settable();     // Transition speed for colors and effects
  advertise("hsv").settable();       // Expects H,S,V as comma separated values (Hue=0°..360°, Sat=0%..100%, Val=0%..100%)
  advertise("rgb").settable();       // Expects R,G,B as comma separated values (R,G,B=0%..100%)
  // advertise("red").settable();     // RGB values from 0% to 100%
  // advertise("green").settable();   //
  // advertise("blue").settable();    //
  // advertise("hue").settable();        // hue from 0° to 360°
  // advertise("saturation").settable(); // from 0% to 100%
  // advertise("value").settable();      // from 0% to 100%
  advertise("white1").settable(); // White channels from 0% to 100%
  advertise("white2").settable(); //
  // advertise("effect").settable();     //
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
    _animationState = DONE;
#ifdef DEBUG
    Homie.getLogger() << "DONE" << endl;
#endif
  }
}
