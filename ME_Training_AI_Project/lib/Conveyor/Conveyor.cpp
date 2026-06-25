#include "Conveyor.h"

// Full-step sequence for a bipolar stepper through an H-bridge.
// Each row is {1A, 2A, 3A, 4A}. The two coils are driven in four phases:
//
//   Step  Coil A (1A,2A)  Coil B (3A,4A)
//    0       H    L          H    L       (A+  B+)
//    1       L    H          H    L       (A-  B+)
//    2       L    H          L    H       (A-  B-)
//    3       H    L          L    H       (A+  B-)
static const uint8_t STEP_SEQ[4][4] = {
  {HIGH, LOW,  HIGH, LOW },  // step 0
  {LOW,  HIGH, HIGH, LOW },  // step 1
  {LOW,  HIGH, LOW,  HIGH},  // step 2
  {HIGH, LOW,  LOW,  HIGH},  // step 3
};

void Conveyor::begin(uint8_t pin1A, uint8_t pin2A, uint8_t pin3A, uint8_t pin4A,
                     bool forward) {
  _pins[0] = pin1A;
  _pins[1] = pin2A;
  _pins[2] = pin3A;
  _pins[3] = pin4A;

  for (uint8_t i = 0; i < 4; i++) {
    pinMode(_pins[i], OUTPUT);
    digitalWrite(_pins[i], LOW);
  }

  _stepIndex = 0;
  setForward(forward);
}

void Conveyor::setForward(bool forward) {
  _dir = forward ? 1 : -1;
}

void Conveyor::applyStep() {
  for (uint8_t i = 0; i < 4; i++) {
    digitalWrite(_pins[i], STEP_SEQ[_stepIndex][i]);
  }
}

void Conveyor::advance(uint16_t steps) {
  for (uint16_t i = 0; i < steps; i++) {
    _stepIndex = (_stepIndex + _dir + 4) % 4;
    applyStep();
    delayMicroseconds(_stepDelayUs);
  }
}

void Conveyor::stop() {
  for (uint8_t i = 0; i < 4; i++) {
    digitalWrite(_pins[i], LOW);
  }
}
