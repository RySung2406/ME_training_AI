#include <Arduino.h>
#include <Wire.h>
#include "HUSKYLENS.h"
#include "DobotArm.h"

// ============================================================================
//  ARM 3 - SORTER (end of line)     (build env: arm3_inspect)
//
//  Board : Arduino Mega 2560 + Dobot Magician on Serial1 @ 115200
//          HuskyLens on I2C (pin 20 SDA / 21 SCL)
//  Place : near the end of the line, next to the GREEN area.
//  Job   : a cube is delivered (already stopped) at the detection point by
//          Conveyor 2. Read its colour, then
//             GREEN          -> Green area
//             blue / yellow  -> Inspection area
//
//  The checking device (LEDs / buzzer / button / tag detection) runs on a
//  SEPARATE board with its own HuskyLens — see src/checker.cpp.
// ============================================================================

const uint8_t PIN_PUMP = 7;   // suction-cup air-pump relay

// HuskyLens learned colour IDs (Colour Recognition mode).
const int ID_RED = 1, ID_GREEN = 2, ID_BLUE = 3, ID_YELLOW = 4;

// --- Arm coordinates (mm / deg) -- *** CALIBRATE THESE *** ---
const float PICK_X    = 200, PICK_Y    =    0, PICK_Z    = -40, PICK_R    = 0;  // end of conveyor 2
const float GREEN_X   = 150, GREEN_Y   =  150, GREEN_Z   = -40, GREEN_R   = 0;  // green area
const float INSPECT_X = 250, INSPECT_Y = -120, INSPECT_Z = -40, INSPECT_R = 0;  // inspection area
const float HOME_X    = 200, HOME_Y    =    0, HOME_Z    =  50, HOME_R    = 0;  // ready pose
const float LIFT_HEIGHT = 60;

const unsigned long MOVE_DELAY  = 3000;
const unsigned long SUCTION_ON  = 2000;
const unsigned long ALGO_SWITCH = 500;

HUSKYLENS huskylens;

// Wait until a cube is sitting at the detection point, then return its colour.
int waitForCube() {
  huskylens.writeAlgorithm(ALGORITHM_COLOR_RECOGNITION);
  delay(ALGO_SWITCH);
  while (true) {
    if (huskylens.request() && huskylens.available()) {
      HUSKYLENSResult result = huskylens.read();
      if (result.ID >= 1 && result.ID <= 4) {
        return result.ID;
      }
    }
  }
}

// Pick the cube at the conveyor-2 end and place it at (x,y,z,r).
void pickTo(float x, float y, float z, float r) {
  goTo(PICK_X, PICK_Y, PICK_Z, PICK_R);
  delay(MOVE_DELAY);
  digitalWrite(PIN_PUMP, HIGH);
  delay(SUCTION_ON);

  goTo(PICK_X, PICK_Y, PICK_Z + LIFT_HEIGHT, PICK_R);
  delay(MOVE_DELAY);
  goTo(x, y, z, r);
  delay(MOVE_DELAY);
  digitalWrite(PIN_PUMP, LOW);
  delay(MOVE_DELAY);

  goTo(HOME_X, HOME_Y, HOME_Z, HOME_R);
  delay(MOVE_DELAY);
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Arm 3 (sorter) booting..."));

  pinMode(PIN_PUMP, OUTPUT);
  digitalWrite(PIN_PUMP, LOW);

  Wire.begin();
  while (!huskylens.begin(Wire)) {
    Serial.println(F("HuskyLens not found - check I2C wiring + set Protocol=I2C"));
    delay(1000);
  }

  dobotInit();
  goTo(HOME_X, HOME_Y, HOME_Z, HOME_R);
  delay(MOVE_DELAY);

  Serial.println(F("Arm 3 ready."));
}

void loop() {
  int colorID = waitForCube();
  Serial.print(F("Arm 3 cube colour ID = "));
  Serial.println(colorID);

  if (colorID == ID_GREEN) {
    pickTo(GREEN_X, GREEN_Y, GREEN_Z, GREEN_R);
  } else {
    pickTo(INSPECT_X, INSPECT_Y, INSPECT_Z, INSPECT_R);
  }
}
