#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <Arduino.h>

// Minimal HC-SR04 ultrasonic distance sensor driver.
//
//   sensor.begin(TRIG_PIN, ECHO_PIN);
//   long cm = sensor.readCm();   // distance to nearest object, -1 if no echo
//
// On a 5 V board (Mega) the ECHO pin can connect directly. On a 3.3 V board
// you would need a voltage divider on ECHO.
class Ultrasonic {
public:
  void begin(uint8_t trigPin, uint8_t echoPin);

  // Distance to the nearest object in centimetres, or -1 on timeout (no echo).
  long readCm();

private:
  uint8_t _trig = 255;
  uint8_t _echo = 255;
};

#endif // ULTRASONIC_H
