
#include <stdint.h>
#include <stdlib.h>
#include <Arduino.h>
#include "northStarFuncs.h"

#define UINT8_MAX        255 // not in stdint.h for some reason

// red   = abs(accel.x) scaled to 1G
// green = abs(accel.y) scaled to 1G
// blue  = abs(accel.z) scaled to 1G
uint8_t conditionAccels(float reading) 
{
  uint8_t colorValue;
  
  // abs(reading)
  if (reading < 0.0) {reading = -reading;}
  
  // scale from 0 to ~10 m/s^2 to 0 to 255
  reading = reading * 25; 
  
  // limit to max
  if (reading > UINT8_MAX ) { 
    colorValue = UINT8_MAX ; 
  } else {
    colorValue = reading;
  }
  return colorValue;
}

uint8_t conditionMags(float heading, float offset, float range) 
{
  float h = heading + offset;
  float colorValFloat;
  uint8_t colorVal;
  
  if (h < -180.0) { h = h + 360.0; }
  if (h > 180.0) { h = h - 360.0; }
  // now h is pointing in the direction of interest
  
  colorValFloat = 255.0 - abs(h)*255.0/range;
  if (colorValFloat < 0.0) { colorValFloat = 0.0; }
  colorVal = colorValFloat; 
  return colorVal;
}
 
// arbitrary conversion: squares to reduces vibe (<1 rad/s)
// so as to highlight real movement
uint8_t conditionGyros(float reading) 
{
  uint8_t colorValue;
  float colorValueFloat;
  reading = abs(reading);
  colorValueFloat = (reading*reading) * 10.0; 
  if (colorValueFloat > 255) {
     colorValue = 255;
  } else {
     colorValue = colorValueFloat;
  }

  return colorValue;
}

eModes changeMode(eModes current)
{
  switch (current) {
    case(OFF):   Serial.println("A"); return ACCEL;
    case(ACCEL): Serial.println("G"); return GYRO;
    case(GYRO):  Serial.println("M"); return MAG;
    case(MAG):   Serial.println("X"); return OFF;
  }
}

