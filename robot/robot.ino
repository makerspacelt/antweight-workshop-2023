/*
   Docs: https://thestempedia.com/docs/dabble/game-pad-module/
   Android app: https://play.google.com/store/apps/details?id=io.dabbleapp
   iOS app: https://apps.apple.com/us/app/dabble-bluetooth-controller/id1472734455?ls=1

   Arduino Libraries (install using Library Manager):
   * DabbleESP32
   * Cdrv8833
   Board (install ESP32 from espressif using Board Manager):
   * ESP32 Dev Module
*/

#define ROBOT_NAME "Zolinis stumbras"

#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>
#include "Cdrv8833.h"

// Uncomment this if the red motor wire is on the left
// when looking from the back of the motor.
#define M1_M2_PINS_INVERT 1

#ifndef M1_M2_PINS_INVERT
// M1 & M2 connected correctly (plus on PCB is red wire)
#define PIN_IN1 26
#define PIN_IN2 25
#define PIN_IN3 13
#define PIN_IN4 27
#else
// M1 & M2 connected incorrectly (plus on PCB is black wire)
#define PIN_IN1 25
#define PIN_IN2 26
#define PIN_IN3 27
#define PIN_IN4 13
#endif
#define CHANNEL 0 // PWM channel (0..15)
#define SWAP    false // swap motor rotation direction 

Cdrv8833 M1;
Cdrv8833 M2;

const float pi180 = 0.017453292519943295;
const float mag_min = 0.0;
const float mag_max = 6.0;
const float mag_scale = 1.0 / 6.0;
const float mag_scaled_start = 0.5;

float m1_spd_prev = 0.0;
float m2_spd_prev = 0.0;
float m1_spd = 0.0;
float m2_spd = 0.0;

float min_spd = -100.0;
float max_spd = 100.0;

unsigned long ms;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);      // make sure your Serial Monitor is also set at this baud rate.
  Dabble.begin(ROBOT_NAME);  // set bluetooth name of your device

  M1.init(PIN_IN1, PIN_IN2, CHANNEL, SWAP);
  M1.setDecayMode(drv8833DecayFast);
  M2.init(PIN_IN3, PIN_IN4, CHANNEL, SWAP);
  M2.setDecayMode(drv8833DecayFast);
}

void loop() {
  // Refresh data obtained from smartphone.
  Dabble.processInput();
  
  ms = millis();

  // The distance from the center of the joystick.
  // This value varies from 0 to 7. At the center, the value is 0 and on the perimeter of the joystick, the value is 7.
  float mag = GamePad.getRadius();

  // Angle between the positive x-axis and the line joining the center and the current position of the dot.
  // It varies from 0 to 360 at a step of 15 degrees.
  float a = GamePad.getAngle();

  if (mag > mag_max) mag = mag_max;
  mag = mag * mag_scale;
  if (mag > 0) {
    if (mag < mag_scaled_start) mag = mag_scaled_start;
  }

  float sina = sin(a * pi180);
  float cosa = cos(a * pi180);
  
  // calculate target speed
  m1_spd = mag * (sina - cosa) * max_spd;
  m2_spd = mag * (sina + cosa) * max_spd;

  // "Digital Mode" up/down/left/right buttons
  if (GamePad.isUpPressed()) {
    m1_spd = 100;
    m2_spd = 100;
  }
  else if (GamePad.isDownPressed()) {
    m1_spd = -100;
    m2_spd = -100;
  }
  else if (GamePad.isLeftPressed()) {
    m1_spd = 100;
    m2_spd = -100;
  }
  else if (GamePad.isRightPressed()) {
    m1_spd = -100;
    m2_spd = 100;
  }

  
  m1_spd = constrain(m1_spd, min_spd, max_spd);
  m2_spd = constrain(m2_spd, min_spd, max_spd);

  // motor needs a lot of start current to get running.
  // if it wasn't running previously, then kick it hard
  // (though one cycle is probably not enough).
  if (m1_spd_prev == 0.0) {
    if (m1_spd > 0) m1_spd = 100;
    else if (m1_spd < 0) m1_spd = -100;
  }
  if (m2_spd_prev == 0.0) {
    if (m2_spd > 0) m2_spd = 100;
    else if (m2_spd < 0) m2_spd = -100;
  }

  m1_spd_prev = m1_spd;
  m2_spd_prev = m2_spd;

  M1.move(m1_spd);
  M2.move(m2_spd);

  if (ms % 100 == 0) {
    Serial.print("mag=");
    Serial.print(mag);
    Serial.print(" a=");
    Serial.print(a);
    Serial.print(" m1=");
    Serial.print(m1_spd);
    Serial.print(" m2=");
    Serial.print(m2_spd);
    Serial.println();
  }

  // All modes - buttons
  //if (GamePad.isCrossPressed())
  //if (GamePad.isTrianglePressed())
  // if (GamePad.isCirclePressed())
  // if (GamePad.isSquarePressed())
  //if (GamePad.isStartPressed())
  //if (GamePad.isSelectPressed())
}
