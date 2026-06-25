#include <Arduino.h>
#include "DobotArm.h"

// ============================================================================
//  GET POSITION - ARM 3 (Inspect)     (build env: getpos_arm3)
//
//  Calibration tool. Upload this to the Arm 3 board, open the Serial Monitor
//  (115200), then:
//    1. Press and HOLD the unlock key on the Dobot arm.
//    2. Move the arm by hand to a target spot (conveyor-2 end / green area /
//       inspection area).
//    3. Send the letter 'P' from the Serial Monitor to capture the pose.
//    4. Paste the printed X/Y/Z/R into the matching const in arm3_inspect.cpp.
//  Re-flash the real firmware (arm3_inspect) when you are done.
// ============================================================================

void setup() {
  Serial.begin(115200);
  Serial.println(F("=== ARM 3 (Inspect) position reader ==="));
  Serial.println(F("Hold the unlock key + move the arm, then send 'P' to read X/Y/Z/R."));
  dobotInit();
}

void loop() {
  // Capture the current position only when 'P' is received over USB serial.
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'P' || c == 'p') {
      float x, y, z, r;
      if (dobotGetPose(x, y, z, r)) {
        Serial.print(F("X="));   Serial.print(x, 1);
        Serial.print(F("  Y=")); Serial.print(y, 1);
        Serial.print(F("  Z=")); Serial.print(z, 1);
        Serial.print(F("  R=")); Serial.println(r, 1);
      } else {
        Serial.println(F("(no reply - check Serial1 wiring / arm power)"));
      }
    }
  }
}
