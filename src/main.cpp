//! Line follower robot for ESP32: TB6612FNG motor driver, dual TCS34725 color sensors.

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>

// -------------------- Pin definitions --------------------

#define MTR_R_PWM 26
#define MTR_R_BWD 25
#define MTR_R_FWD 33

#define MTR_L_PWM 34
#define MTR_L_FWD 35
#define MTR_L_BWD 32

#define CLR_L_SDA 16
#define CLR_L_SCL 21

#define CLR_R_SDA 14
#define CLR_R_SCL 12

// -------------------- Tuning --------------------

const uint16_t LINE_THRESHOLD = 1000;
const int BASE = 150;
const int TURN = 200;
const int loopDelayMs = 20;

// -------------------- Motors --------------------

// Left motor is physically inverted, so speed is negated before driving.
void driveLeft(int speed) {
  int s = constrain(-speed, -255, 255);
  digitalWrite(MTR_L_FWD, s < 0);
  digitalWrite(MTR_L_BWD, s >= 0);
  analogWrite(MTR_L_PWM, abs(s));
}

void driveRight(int speed) {
  int s = constrain(speed, -255, 255);
  digitalWrite(MTR_R_FWD, s < 0);
  digitalWrite(MTR_R_BWD, s >= 0);
  analogWrite(MTR_R_PWM, abs(s));
}

void driveMotors(int leftSpeed, int rightSpeed) {
  driveLeft(leftSpeed);
  driveRight(rightSpeed);
}

void stopMotors() {
  digitalWrite(MTR_L_FWD, HIGH); digitalWrite(MTR_L_BWD, HIGH); analogWrite(MTR_L_PWM, 0);
  digitalWrite(MTR_R_FWD, HIGH); digitalWrite(MTR_R_BWD, HIGH); analogWrite(MTR_R_PWM, 0);
}

// -------------------- TCS34725 sensors --------------------

TwoWire WireLeft(0);
TwoWire WireRight(1);

Adafruit_TCS34725 tcsLeft(TCS34725_INTEGRATIONTIME_24MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcsRight(TCS34725_INTEGRATIONTIME_24MS, TCS34725_GAIN_4X);

uint16_t readClear(Adafruit_TCS34725 &sensor) {
  uint16_t r, g, b, c;
  sensor.getRawData(&r, &g, &b, &c);
  return c;
}

// -------------------- Setup --------------------

void setup() {
  Serial.begin(115200);

  pinMode(MTR_L_FWD, OUTPUT); pinMode(MTR_L_BWD, OUTPUT); pinMode(MTR_L_PWM, OUTPUT);
  pinMode(MTR_R_FWD, OUTPUT); pinMode(MTR_R_BWD, OUTPUT); pinMode(MTR_R_PWM, OUTPUT);
  stopMotors();

  WireLeft.begin(CLR_L_SDA, CLR_L_SCL);
  WireRight.begin(CLR_R_SDA, CLR_R_SCL);
  bool leftOk  = tcsLeft.begin(TCS34725_ADDRESS, &WireLeft);
  bool rightOk = tcsRight.begin(TCS34725_ADDRESS, &WireRight);
  Serial.printf("Left Sensor: %s\n", leftOk ? "OK" : "FAIL");
  Serial.printf("Right Sensor: %s\n", rightOk ? "OK" : "FAIL");

  if (!leftOk || !rightOk) {
    while (1) delay(100);
  }

  Serial.println("Line follower ready.");
  driveMotors(BASE, BASE);
}

// -------------------- Loop --------------------

void loop() {
  uint16_t clear1 = readClear(tcsLeft);
  uint16_t clear2 = readClear(tcsRight);

  bool leftOnLine  = clear1 < LINE_THRESHOLD;
  bool rightOnLine = clear2 < LINE_THRESHOLD;

  if (leftOnLine == rightOnLine) driveMotors(BASE, BASE);   // both or neither
  else if (leftOnLine)           driveMotors(-TURN, BASE);  // turn left
  else                           driveMotors(BASE, -TURN);  // turn right

  Serial.printf("C1:%u C2:%u L:%d R:%d\n", clear1, clear2, leftOnLine, rightOnLine);

  delay(loopDelayMs);
}
