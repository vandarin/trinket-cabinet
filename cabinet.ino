#include <LEDFader.h>

/**
 * 
 * Switch:
 * on:
 * strip on (fade to 255)
 * 
 * off:
 * strip off (fade to 0)
 * 
 * auto:
 * 
 * #if motion
 * fade to 128
 * wait 10 sec
 * fade to 255
 * # was on, no motion
 * wait 5 min
 * fade to 64
 * wait 1 min
 * fade to 0
 * #no motion:
 * off
 * 
 * 
 */

#define MOVING 1
#define STILL 0

LEDFader led;
const byte motionPin = 0;
const byte switchPin = 1; //analog 1, digital 2

// fadeTime is the total time it will take to complete the ease (in milliseconds)
const unsigned int fadeTime = 1000 * 1;

const unsigned int halfwayOnDelay = fadeTime + (1000 * 10); // on the way up, stay at half brightness for 10 sec

const unsigned int halfwayOffDelay = fadeTime + (1000 * 60); // on the way down, stay at half brightness for 60 sec
const unsigned int offDelay = halfwayOffDelay + (1000 * 60 * 5); // stay on for 5 minutes plus halfwayOffDelay (ensures we always fade)

unsigned long motionEndTime, motionStartTime;
byte motionState, prevMode;

// the setup routine runs once when you press reset:
void setup() {
  // initialize the digital pin as an output.
  pinMode(motionPin, INPUT);
  pinMode(switchPin, INPUT);
  led = LEDFader(1);
  led.set_value(0);
  motionStartTime = 0;
  motionEndTime = millis();
}

// the loop routine runs over and over again forever:
void loop() {
  unsigned long currentTime = millis();
  
  // Need to call update each loop cycle to adjust the PWM if needed
  led.update();
  byte motionSensed = digitalRead(motionPin);
  if (motionState == STILL && motionSensed == MOVING) {
    // we were still, now we're moving
    motionStartTime = millis();
  }
  if (motionState == MOVING && motionSensed == STILL) {
    // we were moving, now we're still
    motionEndTime = millis();
  }
  motionState = motionSensed;
  if (prevMode != getMode()) {
    // keep the LEDs at current brightness while changing modes
    led.stop_fade();
    prevMode = getMode();
  }
  switch(getMode()) {
    case 2: // AUTO
     // LED no longer fading
      if (!led.is_fading()) {
        if (motionState == MOVING) {
          if (currentTime - halfwayOnDelay >= motionStartTime) {
            led.fade(255, fadeTime);
          } else {
            led.fade(55, fadeTime);
          }
        } else {  // NOT MOVING
          if (currentTime - offDelay >= motionEndTime) {
            led.fade(0,fadeTime);
          } else if(currentTime - halfwayOffDelay >= motionEndTime) {
            led.fade(55, fadeTime * 2); // fade down slowly
          }
        }
      }
      break;
    case 1:
      if (!led.is_fading()) {
        led.fade(255, fadeTime);
      }
      break;
    case 0:
      if (!led.is_fading()) {
        led.fade(0, fadeTime);
      }
      break;
  }
}


byte getMode() {
  // voltage divider on SPDT switch so we only have to use one pin
  int sensor = analogRead(switchPin);
  if (sensor > 800)
    return 2;
  if (sensor > 300)
    return 1;
  return 0;
}

