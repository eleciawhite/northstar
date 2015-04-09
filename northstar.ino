
/* NorthStar 
** This is a small system that demonstrates the inertial properties of
** an accelerometer, magnetometer, and gyroscope.
** 
** An Arduino UNO is used in conjunction with a ThingM MinM, 
** a small I2C LED, to show the results from the Adafruit 9-DOF IMU
** breakout board which has an LSM303 accelerometer+magnetometer combo
** and a L3GD20H angular rate sensor (gyro). These are all I2C sensors.
**
** The MinM and the IMU are connected to 5V out, GND, SDA and SCL.
** The ATTiny85 is the I2C bus master. It is critical to have pullups. 
** The IMU has on-board pullups but the MinM LED won't work if it is 
** alone on the bus (unless you add pullups).
**/  
#include <stdint.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_9DOF.h>

#include "BlinkM_funcs.h"

#include "northStarFuncs.h"
eModes gMode = OFF;  

/* Assign a unique ID to the sensors */
Adafruit_9DOF                 dof   = Adafruit_9DOF();
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);
Adafruit_L3GD20_Unified       gyro  = Adafruit_L3GD20_Unified(20);

#define BLINKM_ADDR     (0x09)           // 7 bit I2C address for BlinkM LED

#define BUTTON_PIN      13

void initSensors()
{
  uint8_t r = 1, b = 0, g = 0;
  if(!accel.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println(F("No LSM303 accel detected"));
    while(1) {
        BlinkM_setRGB(BLINKM_ADDR, r, g, b);
        delay(250);
        BlinkM_setRGB(BLINKM_ADDR, 0, g, b);
        delay(250);
    };
  }
  Serial.println(F("Accel ok"));
  if(!mag.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("No LSM303 mag detected");
    while(1);
  }
  Serial.println(F("Mag ok"));
  
  if(!gyro.begin())
  {
    /* There was a problem detecting the L3GD20 ... check your connections */
    Serial.println("No L3GD20 gyro detected");
    while(1);
  } else {
      gyro.enableAutoRange(true);
  }
  Serial.println(F("Gyro ok"));
}

void setup(){
  Serial.begin(115200);
  Serial.println(F("NorthStart inertial testing"));

  BlinkM_begin(); /// initialize I2C lib
  BlinkM_off(BLINKM_ADDR);
  
  /* Initialise the sensors */
  initSensors();
  
  /* Initialise the button */
  pinMode(BUTTON_PIN, OUTPUT); // INPUT_PULLUP didn't work, this is probably not that healthy for the board but it does work
  digitalWrite(BUTTON_PIN, HIGH);
}
// button press with hysterisis
boolean buttonPressed()
{
  static int buttonState;             // the current reading from the input pin
  static int lastButtonState = HIGH;   // the previous reading from the input pin
  static long lastDebounceTime = 0;  // the last time the output pin was toggled
  const long kDebounceDelay = 50;    // the debounce time; increase if the output flickers
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
    lastButtonState = reading;
    Serial.print("x");
  } 
  if ((millis() - lastDebounceTime) > kDebounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only indicate button pressed if low, input pulled up and button connects to low
      if (buttonState == LOW) {
        Serial.println("press\n");
        return true;
      }
    }
  }
  return false;
}


void loop()
{
  sensors_event_t gyro_event;
  sensors_event_t accel_event;
  sensors_event_t mag_event;
  sensors_vec_t   orientation;
  uint8_t r = 0, b = 0, g = 0;
  int delayMs = 10;

  switch (gMode) {
  case ACCEL:
    accel.getEvent(&accel_event);
    // the color brightness is the abs value of the direction,
    // scaled to one G
    r = conditionAccels(accel_event.acceleration.x);
    g = conditionAccels(accel_event.acceleration.y);
    b = conditionAccels(accel_event.acceleration.z);
    delayMs = 10; // 100 Hz readings on the accel
    break;
  case MAG:
    accel.getEvent(&accel_event);
    mag.getEvent(&mag_event);
    if (dof.fusionGetOrientation(&accel_event, &mag_event, &orientation)) {
      // N = white, E = green, W = red, S = dark
      r = conditionMags(orientation.heading, -45.0, 120.0);
      g = conditionMags(orientation.heading, 45.0, 120.0);
      b = conditionMags(orientation.heading, 0.0, 120.0);
    }
    delayMs = 100;
    break;
  case GYRO:
    // scale the brightness to the motion being 
    // experienced, where X = R, Y = B, Z = G
    gyro.getEvent(&gyro_event);
    r = conditionGyros(gyro_event.gyro.x);
    g = conditionGyros(gyro_event.gyro.y);
    b = conditionGyros(gyro_event.gyro.z);
    delayMs = 5;
    break;
  default:
    r = g = b = 0;
    delayMs = 10;
    break;
  }
  if (buttonPressed()) { //(accel.detectedDoubleTap()) {
     Serial.print(F("Button pressed from "));
     Serial.print(gMode);
     gMode = changeMode(gMode);
     Serial.print(F(" to new mode "));
     Serial.println(gMode);
  }  
  BlinkM_setRGB(BLINKM_ADDR, r, g, b);
  delay(delayMs);
}


