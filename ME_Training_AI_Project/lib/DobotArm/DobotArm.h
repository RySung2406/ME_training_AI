#ifndef DOBOT_ARM_H
#define DOBOT_ARM_H

#include <Arduino.h>

// Shared helper around the Lesson 3 Dobot SDK (lib/DobotSDK).
//
// Every board in this project controls exactly ONE Dobot Magician on Serial1,
// so the SDK's single global protocol handler (hardwired to Serial1) is fine.
// All three arm sketches use these three calls instead of touching the SDK
// directly.

// Open Serial1, init the protocol stack and push default motion parameters.
void dobotInit();

// Pump the protocol stack: drain Serial1 RX, then flush queued commands out.
void dobotProcess();

// Queue a JUMP-mode move to (x,y,z,r) [mm / deg] and flush it to the arm.
void goTo(float x, float y, float z, float r);

// Read the arm's CURRENT pose (x,y,z,r in mm / deg) via the Dobot GetPose
// command. Returns false if the arm does not reply within ~500 ms. Use this to
// hand-teach coordinates: press the unlock key on the arm, move it by hand, and
// read off the printed values for calibration.
bool dobotGetPose(float &x, float &y, float &z, float &r);

#endif // DOBOT_ARM_H
