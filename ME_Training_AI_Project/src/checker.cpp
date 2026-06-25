#include <Arduino.h>
#include <Wire.h>
#include "HUSKYLENS.h"

// ============================================================================
//  CHECKER — Mission 4 checking device     (build env: checker)
//
//  Board : Arduino Mega 2560 (or Uno — no Dobot, no conveyor)
//          HuskyLens on I2C (pin 20 SDA / 21 SCL on Mega,
//                            pin A4 SDA / A5 SCL on Uno)
//  Place : dedicated camera aimed at the INSPECTION AREA, running full-time.
//  Job   : continuously monitor the inspection area and drive the LEDs,
//          buzzer, and silence button.
//
//  Rules (from Lesson 4 spec):
//    1. >= 1 blue AND >= 1 yellow block  ->  green LED on, else red LED on.
//    2. Any red block OR April Tag TAG36H11-0 detected  ->  buzzer on (500 Hz).
//    3. Press button  ->  buzzer off (re-arms on the next scan if cause remains).
// ============================================================================

const uint8_t PIN_GREEN_LED = 8;
const uint8_t PIN_RED_LED   = 9;
const uint8_t PIN_BUZZER    = 10;
const uint8_t PIN_BUTTON    = 11;   // active HIGH, wire to 5 V with pull-down

// HuskyLens learned colour IDs (Colour Recognition mode).
const int ID_RED = 1, ID_GREEN = 2, ID_BLUE = 3, ID_YELLOW = 4;
// Tag Recognition mode: learn April-tag TAG36H11-0 as this ID.
const int TRIGGER_TAG_ID = 1;

const unsigned long ALGO_SWITCH = 500;   // let HuskyLens change algorithm (ms)

HUSKYLENS huskylens;

void serviceButton() {
  if (digitalRead(PIN_BUTTON) == HIGH) {
    noTone(PIN_BUZZER);
  }
}

void checkInspectionArea() {
  // --- Phase 1: colours ---
  huskylens.writeAlgorithm(ALGORITHM_COLOR_RECOGNITION);
  delay(ALGO_SWITCH);
  huskylens.request();
  int blue   = huskylens.countBlocks(ID_BLUE);
  int yellow = huskylens.countBlocks(ID_YELLOW);
  int red    = huskylens.countBlocks(ID_RED);

  if (blue >= 1 && yellow >= 1) {
    digitalWrite(PIN_GREEN_LED, HIGH);
    digitalWrite(PIN_RED_LED,   LOW);
  } else {
    digitalWrite(PIN_RED_LED,   HIGH);
    digitalWrite(PIN_GREEN_LED, LOW);
  }

  // --- Phase 2: tags ---
  huskylens.writeAlgorithm(ALGORITHM_TAG_RECOGNITION);
  delay(ALGO_SWITCH);
  huskylens.request();
  bool alarmTag = (huskylens.countBlocks(TRIGGER_TAG_ID) >= 1);

  if (red >= 1 || alarmTag) {
    tone(PIN_BUZZER, 500);
  }

  serviceButton();
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Checker (Mission 4) booting..."));

  pinMode(PIN_GREEN_LED, OUTPUT);
  pinMode(PIN_RED_LED,   OUTPUT);
  pinMode(PIN_BUZZER,    OUTPUT);
  pinMode(PIN_BUTTON,    INPUT);
  digitalWrite(PIN_GREEN_LED, LOW);
  digitalWrite(PIN_RED_LED,   LOW);

  Wire.begin();
  while (!huskylens.begin(Wire)) {
    Serial.println(F("HuskyLens not found - check I2C wiring + set Protocol=I2C"));
    delay(1000);
  }

  Serial.println(F("Checker ready — monitoring inspection area."));
}

void loop() {
  checkInspectionArea();
  serviceButton();
  delay(200);
}
