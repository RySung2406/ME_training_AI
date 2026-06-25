#ifndef CONVEYOR_H
#define CONVEYOR_H

#include <Arduino.h>

// Minimal step/dir stepper driver for a conveyor belt (A4988 / DRV8825 style).
//
// Wire STEP and DIR to two digital pins; ENABLE is optional (tie the driver's
// EN to GND and pass 255 to skip it). The belt is advanced in blocking bursts
// so a sketch can interleave belt motion with camera polling:
//
//     belt.enable();
//     while (!cubeSeen) belt.advance(50);   // keep nudging the belt along
//     belt.disable();
class Conveyor {
public:
  // enablePin = 255 means "no enable pin" (driver always on).
  void begin(uint8_t stepPin, uint8_t dirPin, uint8_t enablePin = 255, bool forward = true);

  void advance(uint16_t steps);   // pulse STEP `steps` times (blocking)
  void setForward(bool forward);  // belt direction
  void enable();                  // energise the motor (A4988 EN is active LOW)
  void disable();                 // release the motor

private:
  uint8_t  _step    = 255;
  uint8_t  _dir     = 255;
  uint8_t  _en      = 255;
  uint16_t _pulseUs = 800;        // half-period of each STEP pulse
};

#endif // CONVEYOR_H
