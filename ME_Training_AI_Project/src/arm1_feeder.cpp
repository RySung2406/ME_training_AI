#include <Arduino.h>
#include "DobotArm.h"
#include "Conveyor.h"
#include "Ultrasonic.h"

// ============================================================================
//  ARM 1 - FEEDER + CONVEYOR 1     (build env: arm1_feeder)
//
//  Board : Arduino Mega 2560 + Dobot Magician on Serial1 @ 115200
//          Conveyor 1 stepper on the STEP/DIR/EN pins below
//          HC-SR04 ultrasonic sensor aimed at the start point
//  Job   : when the ultrasonic sensor detects a cube at the start point, pick
//          it, drop it onto Conveyor 1, then run the belt until the cube
//          reaches Arm 2's detection point and stop.
// ============================================================================

const uint8_t PIN_PUMP      = 7;   // suction-cup air-pump relay (external 3.5 V)
const uint8_t PIN_BELT_STEP = 2;   // Conveyor 1 driver STEP
const uint8_t PIN_BELT_DIR  = 3;   // Conveyor 1 driver DIR
const uint8_t PIN_BELT_EN   = 4;   // Conveyor 1 driver EN (active LOW)
const uint8_t PIN_TRIG      = 5;   // HC-SR04 trigger
const uint8_t PIN_ECHO      = 6;   // HC-SR04 echo

// A cube is "present" at the start point when the ultrasonic reads closer than
// this. *** CALIBRATE ***: compare the empty distance with the with-cube
// distance and pick a threshold in between.
const long    BLOCK_DISTANCE_CM   = 10;
// Require this many consecutive close reads before acting (rejects noise).
const uint8_t BLOCK_CONFIRM_READS = 3;

// How far to run Conveyor 1 so a cube travels from the load point to Arm 2's
// detection point. *** CALIBRATE ***: jog the belt and count the steps.
const uint32_t BELT1_TRAVEL_STEPS = 4000;

// --- Arm coordinates (mm / deg) -- *** CALIBRATE THESE *** ---
const float START_X = 200, START_Y =    0, START_Z = -40, START_R = 0;  // pick-up point
const float BELT1_X = 100, BELT1_Y = -150, BELT1_Z = -40, BELT1_R = 0;  // drop onto conveyor 1
const float HOME_X  = 200, HOME_Y  =    0, HOME_Z  =  50, HOME_R  = 0;  // safe ready pose
const float LIFT_HEIGHT = 60;

const unsigned long MOVE_DELAY = 3000;  // settle time between moves (ms)
const unsigned long SUCTION_ON = 2000;  // hold suction before lifting (ms)

Conveyor   belt1;
Ultrasonic startSensor;

// Block (busy-wait) until the ultrasonic confirms a cube at the start point.
void waitForBlock() {
  Serial.println(F("Waiting for a block at the start point..."));
  uint8_t confirms = 0;
  while (confirms < BLOCK_CONFIRM_READS) {
    long d = startSensor.readCm();
    if (d > 0 && d <= BLOCK_DISTANCE_CM) {
      confirms++;            // one more close read
    } else {
      confirms = 0;          // not close (or no echo) -> reset
    }
    delay(50);
  }
  Serial.println(F("Block detected."));
}

void feedOneCube() {
  // Grab the cube at the start point.
  goTo(START_X, START_Y, START_Z, START_R);
  delay(MOVE_DELAY);
  digitalWrite(PIN_PUMP, HIGH);   // suction ON
  delay(SUCTION_ON);

  // Lift and carry it to the conveyor-1 load point.
  goTo(START_X, START_Y, START_Z + LIFT_HEIGHT, START_R);
  delay(MOVE_DELAY);
  goTo(BELT1_X, BELT1_Y, BELT1_Z, BELT1_R);
  delay(MOVE_DELAY);
  digitalWrite(PIN_PUMP, LOW);    // suction OFF -> cube is now ON the belt
  delay(MOVE_DELAY);

  // Lift clear of the belt so the moving cube won't hit the suction cup.
  goTo(BELT1_X, BELT1_Y, BELT1_Z + LIFT_HEIGHT, BELT1_R);
  delay(MOVE_DELAY);

  // Start the belt the moment the cube is placed; stop once it has travelled
  // to Arm 2's detection point.
  belt1.advance(BELT1_TRAVEL_STEPS);

  // Back to ready pose.
  goTo(HOME_X, HOME_Y, HOME_Z, HOME_R);
  delay(MOVE_DELAY);
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Arm 1 (feeder) booting..."));

  pinMode(PIN_PUMP, OUTPUT);
  digitalWrite(PIN_PUMP, LOW);

  belt1.begin(PIN_BELT_STEP, PIN_BELT_DIR, PIN_BELT_EN, true);
  startSensor.begin(PIN_TRIG, PIN_ECHO);

  dobotInit();
  goTo(HOME_X, HOME_Y, HOME_Z, HOME_R);
  delay(MOVE_DELAY);

  Serial.println(F("Arm 1 ready."));
}

void loop() {
  waitForBlock();   // ultrasonic detects a cube at the start point
  feedOneCube();    // pick it, drop on Conveyor 1, run the belt to Arm 2
}
