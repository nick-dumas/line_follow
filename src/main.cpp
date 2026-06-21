//! Line follower robot for ESP32: TB6612FNG motor driver, dual TCS34725 color sensors.

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>

// Types

struct MotorPins {
  uint8_t fwd;
  uint8_t bwd;
  uint8_t pwm;
};

// -------------------- Pins --------------------

MotorPins MOTOR_R = {25, 33, 32};
MotorPins MOTOR_L = {26, 27, 14};

#define CLR_L_SDA 18
#define CLR_L_SCL 5

#define CLR_R_SDA 17
#define CLR_R_SCL 16

TwoWire I2C_L(0);
TwoWire I2C_R(1);
  
Adafruit_TCS34725 TCS_L(TCS34725_INTEGRATIONTIME_2_4MS, TCS34725_GAIN_16X);
Adafruit_TCS34725 TCS_R(TCS34725_INTEGRATIONTIME_2_4MS, TCS34725_GAIN_16X);

// -------------------- Tuning --------------------

const uint16_t LINE_THRESHOLD = 150;
const int BASE = 150;
const int TURN1 = -150;
const int TURN2 = 50;
const int TURN_DELAY_MS = 50;

// -------------------- Functions --------------------

// Left motor is physically inverted, so speed is negated before driving.
void driveMotor(MotorPins pins, int speed) {
  int s = constrain(-speed, -255, 255);
  digitalWrite(pins.fwd, s < 0);
  digitalWrite(pins.bwd, s >= 0);
  analogWrite(pins.pwm, abs(s));
}

void driveMotors(int leftSpeed, int rightSpeed) {
  driveMotor(MOTOR_L, leftSpeed);
  driveMotor(MOTOR_R, rightSpeed);
}

void stopMotors() {
  driveMotors(0, 0);
}

uint16_t readClear(Adafruit_TCS34725 &sensor) {
  uint16_t r, g, b, c;
  sensor.getRawData(&r, &g, &b, &c);
  return c;
}

// -------------------- Setup --------------------

void setup() {
  Serial.begin(115200);

  pinMode(MOTOR_L.fwd, OUTPUT); pinMode(MOTOR_L.bwd, OUTPUT); pinMode(MOTOR_L.pwm, OUTPUT);
  pinMode(MOTOR_R.fwd, OUTPUT); pinMode(MOTOR_R.bwd, OUTPUT); pinMode(MOTOR_R.pwm, OUTPUT);
  stopMotors();

  I2C_L.begin(CLR_L_SDA, CLR_L_SCL);
  I2C_R.begin(CLR_R_SDA, CLR_R_SCL);
  bool leftOk  = TCS_L.begin(TCS34725_ADDRESS, &I2C_L);
  bool rightOk = TCS_R.begin(TCS34725_ADDRESS, &I2C_R);
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
  static uint32_t lastLoopUs = 0;
  static uint32_t maxLatencyUs = 0;
  static uint32_t sumLatencyUs = 0;
  static uint32_t loopCount = 0;
  uint32_t loopStartUs = micros();
  if (lastLoopUs) {
    uint32_t dt = loopStartUs - lastLoopUs;
    if (dt > maxLatencyUs) maxLatencyUs = dt;
    sumLatencyUs += dt;
    loopCount++;
  }
  lastLoopUs = loopStartUs;

  uint16_t clear_l = readClear(TCS_L);
  uint16_t clear_r = readClear(TCS_R);

  bool leftOnLine  = clear_l < LINE_THRESHOLD;
  bool rightOnLine = clear_r < LINE_THRESHOLD;

  const char* dir = "";

  if (leftOnLine == rightOnLine) {
    dir = "F";
    driveMotors(BASE, BASE);
  } else if (leftOnLine) {
    dir = "L";
    driveMotors(TURN1, TURN2);
    delay(TURN_DELAY_MS);
  } else {
    dir = "R";
    driveMotors(TURN2, TURN1);
    delay(TURN_DELAY_MS);
  }

  static uint32_t lastPrint = 0;
  uint32_t now = millis();
  if (now - lastPrint >= 1000) {
    lastPrint = now;
    float avgMs = loopCount ? (sumLatencyUs / 1000.0f) / loopCount : 0.0f;
    float maxMs = maxLatencyUs / 1000.0f;
    Serial.printf("T:%8u\tavg:%6.2fms\tmax:%6.2fms\tCL:%5u\tCR:%5u\tDIR:%s\n",
                  now, avgMs, maxMs, clear_l, clear_r, dir);
    maxLatencyUs = 0;
    sumLatencyUs = 0;
    loopCount = 0;
  }

  static bool lastLeftOnLine = false;
  static bool lastRightOnLine = false;
  if (leftOnLine != lastLeftOnLine || rightOnLine != lastRightOnLine) {
    Serial.printf("T:%8u\tCL:%5u\tCR:%5u\tL:%d\tR:%d\tDIR:%s\n",
                  now, clear_l, clear_r, leftOnLine, rightOnLine, dir);
    lastLeftOnLine = leftOnLine;
    lastRightOnLine = rightOnLine;
  }
}
