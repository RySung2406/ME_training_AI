#include <Arduino.h>
#include <Wire.h>
#include "HUSKYLENS.h"
#include "DobotArm.h"
#include "Conveyor.h"

// ============================================================================
//  ARM 2 - SORTER + CONVEYOR 2     (build env: arm2_sorter)
//
//  Board : Arduino Mega 2560 + Dobot Magician on Serial1 @ 115200
//          HuskyLens on I2C (pin 20 SDA / 21 SCL)
//          Conveyor 2 stepper via SN754410NE H-bridge
//  Place : between Conveyor 1 and Conveyor 2, next to the RED area.
//  Job   : a cube is delivered (already stopped) at the detection point by
//          Conveyor 1. Read its colour, then
//             RED            -> Red area
//             anything else  -> load Conveyor 2, run it to Arm 3's detection
//                               point, and stop.
// ============================================================================

const uint8_t PIN_PUMP = 7;   // suction-cup air-pump relay
const uint8_t PIN_1A  = 2;   // SN754410NE input 1A (coil A+)
const uint8_t PIN_2A  = 3;   // SN754410NE input 2A (coil A-)
const uint8_t PIN_3A  = 4;   // SN754410NE input 3A (coil B+)
const uint8_t PIN_4A  = 8;   // SN754410NE input 4A (coil B-)

// HuskyLens learned colour IDs (train in Colour Recognition mode).
const int ID_RED = 1, ID_GREEN = 2, ID_BLUE = 3, ID_YELLOW = 4;

// How far to run Conveyor 2 so a cube travels from its load point to Arm 3's
// detection point. *** CALIBRATE ***: jog the belt and count the steps.
const uint32_t BELT2_TRAVEL_STEPS = 4000;

// --- Arm coordinates (mm / deg) -- *** CALIBRATE THESE *** ---
const float PICK_X  = 200, PICK_Y  =    0, PICK_Z  = -40, PICK_R  = 0;  // end of conveyor 1
const float RED_X   = 150, RED_Y   =  150, RED_Z   = -40, RED_R   = 0;  // red area
const float BELT2_X = 100, BELT2_Y = -150, BELT2_Z = -40, BELT2_R = 0;  // load point of conveyor 2
const float HOME_X  = 200, HOME_Y  =    0, HOME_Z  =  50, HOME_R  = 0;  // ready pose
const float LIFT_HEIGHT = 60;

const unsigned long MOVE_DELAY  = 3000;  // settle time between moves (ms)
const unsigned long SUCTION_ON  = 2000;  // hold suction before lifting (ms)
const unsigned long ALGO_SWITCH = 500;   // let HuskyLens change algorithm (ms)

HUSKYLENS huskylens;
Conveyor  belt2;

// Wait (watching, belt idle) until a cube is sitting at the detection point,
// then return its learned colour ID (1..4). Conveyor 1 already delivered and
// stopped it -- this board only watches.
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

// Grab the cube at the conveyor-1 end and lift it clear.
void pickFromConveyor() {
  goTo(PICK_X, PICK_Y, PICK_Z, PICK_R);
  delay(MOVE_DELAY);
  digitalWrite(PIN_PUMP, HIGH);   // suction ON
  delay(SUCTION_ON);
  goTo(PICK_X, PICK_Y, PICK_Z + LIFT_HEIGHT, PICK_R);
  delay(MOVE_DELAY);
}

// RED cube -> drop in the red area, return home.
void placeInRedArea() {
  pickFromConveyor();
  goTo(RED_X, RED_Y, RED_Z, RED_R);
  delay(MOVE_DELAY);
  digitalWrite(PIN_PUMP, LOW);    // suction OFF
  delay(MOVE_DELAY);
  goTo(HOME_X, HOME_Y, HOME_Z, HOME_R);
  delay(MOVE_DELAY);
}

// Other cube -> drop on Conveyor 2, run the belt to Arm 3, return home.
void deliverOnConveyor2() {
  pickFromConveyor();
  goTo(BELT2_X, BELT2_Y, BELT2_Z, BELT2_R);
  delay(MOVE_DELAY);
  digitalWrite(PIN_PUMP, LOW);    // suction OFF -> cube is now ON the belt
  delay(MOVE_DELAY);

  // Lift clear, then start the belt and stop it at Arm 3's detection point.
  goTo(BELT2_X, BELT2_Y, BELT2_Z + LIFT_HEIGHT, BELT2_R);
  delay(MOVE_DELAY);
  belt2.advance(BELT2_TRAVEL_STEPS);
  belt2.stop();   // de-energise coils

  goTo(HOME_X, HOME_Y, HOME_Z, HOME_R);
  delay(MOVE_DELAY);
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Arm 2 (sorter) booting..."));

  pinMode(PIN_PUMP, OUTPUT);
  digitalWrite(PIN_PUMP, LOW);

  Wire.begin();
  while (!huskylens.begin(Wire)) {
    Serial.println(F("HuskyLens not found - check I2C wiring + set Protocol=I2C"));
    delay(1000);
  }

  belt2.begin(PIN_1A, PIN_2A, PIN_3A, PIN_4A, true);

  dobotInit();
  goTo(HOME_X, HOME_Y, HOME_Z, HOME_R);
  delay(MOVE_DELAY);

  Serial.println(F("Arm 2 ready."));
}

void loop() {
  int colorID = waitForCube();
  Serial.print(F("Arm 2 cube colour ID = "));
  Serial.println(colorID);

  if (colorID == ID_RED) {
    placeInRedArea();       // red -> red area
  } else {
    deliverOnConveyor2();   // else -> on to conveyor 2
  }
}
