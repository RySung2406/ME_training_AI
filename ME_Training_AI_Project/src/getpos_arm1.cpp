#include <Arduino.h>

// ============================================================================
//  GET POSITION - ARM 1 (Feeder)     (build env: getpos_arm1)
//
//  Self-contained — talks raw Dobot protocol on Serial1, no SDK, no queued
//  commands. Does NOT move the arm or interfere with hand-jogging.
//
//  Usage:
//    1. Hold the unlock key on the Dobot arm.
//    2. Move the arm by hand to the target spot.
//    3. Release the key so the arm holds position.
//    4. Type P in the Serial Monitor (115200, "No line ending" or "Newline").
//    5. Read the printed X / Y / Z / R.
//    6. Paste into the matching const in src/arm1_feeder.cpp.
//    7. Repeat for each spot, then re-flash the real firmware.
// ============================================================================

// Send the raw GetPose request and parse the response.
// Dobot packet: AA AA Len ID Ctrl [Params...] Checksum
// GetPose: ID=10, Ctrl=0x00 (read, not queued), no params.
// Response payload: 8 floats (x, y, z, r, j1, j2, j3, j4) = 32 bytes.

bool getPose(float &x, float &y, float &z, float &r) {
  // Drain any stale bytes sitting in the Serial1 buffer.
  while (Serial1.available()) Serial1.read();

  // Build and send the GetPose request.
  // AA AA 02 0A 00 F6
  const uint8_t req[] = {0xAA, 0xAA, 0x02, 0x0A, 0x00, 0xF6};
  Serial1.write(req, sizeof(req));
  Serial1.flush();   // wait until all bytes are physically sent

  // Wait for the response (timeout 1 second).
  // We need at least: 2 sync + 1 len + 34 payload + 1 checksum = 38 bytes.
  uint8_t buf[64];
  uint8_t idx = 0;
  unsigned long start = millis();
  while (millis() - start < 1000) {
    while (Serial1.available() && idx < sizeof(buf)) {
      buf[idx++] = Serial1.read();
    }
    // Check if we have a complete packet.
    if (idx >= 4) {
      // buf[0..1] = AA AA, buf[2] = payloadLen
      uint8_t payloadLen = buf[2];
      uint8_t totalLen = 2 + 1 + payloadLen + 1;  // sync(2) + len(1) + payload + checksum(1)
      if (idx >= totalLen) {
        // Verify it is a GetPose response (ID = 10).
        if (buf[0] == 0xAA && buf[1] == 0xAA && buf[3] == 0x0A && payloadLen >= 34) {
          // Params start at byte 5 (after sync, len, id, ctrl).
          memcpy(&x, &buf[5],  4);
          memcpy(&y, &buf[9],  4);
          memcpy(&z, &buf[13], 4);
          memcpy(&r, &buf[17], 4);
          return true;
        }
        return false;  // got a packet but wrong ID or too short
      }
    }
  }
  return false;  // timeout
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  delay(500);   // let the arm's controller finish booting

  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("   ARM 1 (Feeder) - Position Reader"));
  Serial.println(F("========================================"));
  Serial.println(F("1. Hold the unlock key + move the arm."));
  Serial.println(F("2. Release the key (arm holds position)."));
  Serial.println(F("3. Send 'P' to capture X / Y / Z / R."));
  Serial.println();
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'P' || c == 'p') {
      float x, y, z, r;
      if (getPose(x, y, z, r)) {
        Serial.print(F("  X = ")); Serial.print(x, 1);
        Serial.print(F("    Y = ")); Serial.print(y, 1);
        Serial.print(F("    Z = ")); Serial.print(z, 1);
        Serial.print(F("    R = ")); Serial.println(r, 1);
      } else {
        Serial.println(F("  (no reply - check Serial1 wiring 18/19 + arm power)"));
      }
    }
  }
}
