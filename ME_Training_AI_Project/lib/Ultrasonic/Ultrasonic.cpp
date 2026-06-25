#include "Ultrasonic.h"

void Ultrasonic::begin(uint8_t trigPin, uint8_t echoPin) {
  _trig = trigPin;
  _echo = echoPin;
  pinMode(_trig, OUTPUT);
  pinMode(_echo, INPUT);
  digitalWrite(_trig, LOW);
}

long Ultrasonic::readCm() {
  // Fire a 10 us trigger pulse.
  digitalWrite(_trig, LOW);
  delayMicroseconds(2);
  digitalWrite(_trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(_trig, LOW);

  // The ECHO pin stays HIGH for a time proportional to the distance.
  // Time out at ~30 ms (~5 m) so a missing echo doesn't block forever.
  unsigned long duration = pulseIn(_echo, HIGH, 30000UL);
  if (duration == 0) return -1;        // no echo within the timeout

  // Sound travels ~58 us per cm round-trip.
  return (long)(duration / 58);
}
