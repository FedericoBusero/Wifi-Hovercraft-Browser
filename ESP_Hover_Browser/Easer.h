#ifndef Easer_h
#define Easer_h

#include <Arduino.h>

class Easer
{
  private:
    float startValue;   // where easer started 
    float currentValue;    // current easer value

    float changeValue;
    float ms_per_unit; // time in ms to move 1 unit

    unsigned long durMillis;    // duration
    unsigned long startMillis; // time when we started

    boolean arrived;  // has easer arrived at its destination
    boolean ease_down; // ease when the value goes down (true = ease when going down; false = set immediatly when going down)

    // linear easing function
    // t: current time, b: beginning value, c: change in value, d: duration
    // t and d can be in frames or seconds/milliseconds
    inline float easingFunc (float t, float b, float c, float d) {
      return c * t / d + b;
    }

  public:
    // set up an easer with just a starting value
    // when _ease_down is false, only ease when increasing the value
    void begin(float _startvalue,boolean _ease_down=true) {
      ms_per_unit = 0;
      ease_down = _ease_down;

      setValue(_startvalue);
    }

    void set_speed(float _ms_per_unit) {
      ms_per_unit = _ms_per_unit;
    }

    // reset easer to initial conditions, set current value to new start value
    void setValue(float _startvalue) {
      startValue = _startvalue;
      currentValue = _startvalue;
      changeValue = 0;

      startMillis = millis();
      durMillis = 0;
      arrived = true;
    }

    void easeTo( int destinationvalue, unsigned long dur ) {
      if (!ease_down && (destinationvalue<currentValue))
      {
        setValue(destinationvalue);
        return;
      }
      startValue = currentValue;
      changeValue = destinationvalue - startValue;
      durMillis = dur;
      startMillis = millis();
      arrived = false;
    }

    int getDestination() {
      return startValue+changeValue;
    }

    // ease with default speed
    void easeTo( int destination ) {
      this->update();
      easeTo(destination,abs(destination-currentValue)*ms_per_unit);
    }


    // get current value
    float getCurrentValue() {
      return currentValue;
    }

    // call this regularly in loop()
    void update() {
      unsigned long currentMillis = millis();

      if (arrived)
      {
        return;
      }

      if (currentMillis >= (startMillis + durMillis)) {
        currentValue = startValue + changeValue;
        arrived = true;
      }
      else {
        currentValue = easingFunc( currentMillis - startMillis, startValue, changeValue, durMillis );
      }
    }

    // has easer arrived at its commaned point?
    boolean hasArrived() {
      return arrived;
    }
};

#endif
