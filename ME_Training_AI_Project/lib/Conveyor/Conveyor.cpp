#include "Conveyor.h"

void Conveyor::begin(uint8_t stepPin, uint8_t dirPin, uint8_t enablePin, bool forward) {
  _step = stepPin;
  _dir  = dirPin;
  _en   = enablePin;

  pinMode(_step, OUTPUT);
  pinMode(_dir,  OUTPUT);
  if (_en != 255) pinMode(_en, OUTPUT);

  setForward(forward);
  enable();
}

void Conveyor::setForward(bool forward) {
  digitalWrite(_dir, forward ? HIGH : LOW);
}

void Conveyor::enable() {
  if (_en != 255) digitalWrite(_en, LOW);    // A4988/DRV8825 EN is active LOW
}

void Conveyor::disable() {
  if (_en != 255) digitalWrite(_en, HIGH);
}

void Conveyor::advance(uint16_t steps) {
  for (uint16_t i = 0; i < steps; i++) {
    digitalWrite(_step, HIGH);
    delayMicroseconds(_pulseUs);
    digitalWrite(_step, LOW);
    delayMicroseconds(_pulseUs);
  }
}
