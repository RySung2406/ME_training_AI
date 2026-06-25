#include <Arduino.h>
#include <Wire.h>
#include "HUSKYLENS.h"

HUSKYLENS huskylens;
//HUSKYLENS green line >> SDA; blue line >> SCL
void setup() {
    Serial.begin(115200);
    Wire.begin();
    while (!huskylens.begin(Wire))
    {
        delay(100);
    }
    huskylens.writeAlgorithm(ALGORITHM_FACE_RECOGNITION);
}

void loop() {
    if (huskylens.request()) {
        if (huskylens.available()) {
            HUSKYLENSResult result = huskylens.read();
            Serial.print("Face ID: ");
            Serial.println(result.ID);
        }
    }
    delay(100);
}
