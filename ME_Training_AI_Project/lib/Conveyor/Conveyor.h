#ifndef CONVEYOR_H
#define CONVEYOR_H

#include <Arduino.h>

// Bipolar stepper conveyor driven through an SN754410NE H-Bridge.
//
// The H-bridge has 4 inputs (1A, 2A, 3A, 4A) that directly control the two
// motor coils. The stepping sequence is generated in software (full-step mode).
//
// Wiring (SN754410NE):
//   pin1A -> H-bridge 1A (pin 2)  -> output 1Y (pin 3)  -> motor coil A1
//   pin2A -> H-bridge 2A (pin 7)  -> output 2Y (pin 6)  -> motor coil A2
//   pin3A -> H-bridge 3A (pin 10) -> output 3Y (pin 11) -> motor coil B1
//   pin4A -> H-bridge 4A (pin 15) -> output 4Y (pin 14) -> motor coil B2
//   1,2EN and 3,4EN tied to 5 V (always enabled).
//   VCC1 = 5 V (logic), VCC2 = 3.5 V external (motor power).

class Conveyor {
public:
  void begin(uint8_t pin1A, uint8_t pin2A, uint8_t pin3A, uint8_t pin4A,
             bool forward = true);

  void advance(uint16_t steps);   // step the motor (blocking)
  void setForward(bool forward);  // belt direction
  void stop();                    // de-energise all coils

  // Adjust speed: lower = faster. Default 2000 us between steps.
  void setStepDelayUs(uint16_t us) { _stepDelayUs = us; }

private:
  uint8_t  _pins[4];             // 1A, 2A, 3A, 4A
  int8_t   _dir         = 1;     // +1 forward, -1 reverse
  uint8_t  _stepIndex   = 0;     // current position in the 4-step sequence
  uint16_t _stepDelayUs = 2000;  // microseconds between steps
  void applyStep();              // drive the pins for the current step index
};

#endif // CONVEYOR_H
