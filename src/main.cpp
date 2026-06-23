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

struct ColorReading {
  uint16_t red;
  uint16_t green;
  uint16_t blue;
  uint16_t clear;
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

// Green-stop detection. Green is declared when the green channel dominates both
// red and blue by GREEN_RATIO, with enough light (clear) to trust the reading.
// These depend on your sensor/lighting — tune from the printed R/G/B values.
const uint16_t GREEN_MIN_CLEAR = 50;
const float    GREEN_RATIO     = 1.4f;

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

ColorReading readColor(Adafruit_TCS34725 &sensor) {
  ColorReading cr;
  sensor.getRawData(&cr.red, &cr.green, &cr.blue, &cr.clear);
  return cr;
}

bool isBlack(ColorReading cr) {
  return cr.clear < LINE_THRESHOLD;
}

// True when the sensor sees a predominantly green surface.
bool isGreen(ColorReading cr) {
  if (cr.clear < GREEN_MIN_CLEAR) return false;  // too dark to classify
  return cr.green > cr.red * GREEN_RATIO && cr.green > cr.blue * GREEN_RATIO;
}

void debugColor(ColorReading cr) {
  Serial.printf("R:%5u G:%5u B:%5u C:%5u, %s, %s\n",
    cr.red, cr.green, cr.blue, cr.clear,
    isBlack(cr) ? " BLK" : "!BLK",
    isGreen(cr) ? " GRN" : "!GRN");
}

// Stop the motors and halt permanently.
void haltForever(const char *reason) {
  stopMotors();
  Serial.printf("HALT: %s\n", reason);
  while (1) delay(1000);
}

void green(bool is_right) {
  driveMotors(BASE, BASE);
  while (1) {
    ColorReading cr = readColor(is_right ? TCS_R : TCS_L);
    if (isBlack(cr) && !isGreen(cr)) {
      break;
    }
  }
  delay(200);
  int green_side = BASE;
  int other_side = -BASE * 0.5;
  if (is_right) {
    driveMotors(green_side, other_side);
  } else {
    driveMotors(other_side, green_side);
  }
  delay(800);
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
    haltForever("Failed to initialize color sensors");
  }

  Serial.println("Color sensors initialized");
  Serial.println("Waiting for button...");
  // Boot button is GPIO0, fine to use after boot
  static uint32_t lastPrint = 0;
  while (digitalRead(0)) {
    uint32_t now = millis();
    if (now - lastPrint >= 1000) {
      lastPrint = now;
      ColorReading l_color = readColor(TCS_L);
      ColorReading r_color = readColor(TCS_R);
      Serial.println("Left:");
      debugColor(l_color);
      Serial.println("Right:");
      debugColor(r_color);
    }
  }
  Serial.println("Starting!");
  driveMotors(BASE, BASE);
}

// -------------------- Loop --------------------

void loop() {

  ColorReading color_l = readColor(TCS_L);
  ColorReading color_r = readColor(TCS_R);

  const char* dir = "";

  static uint32_t lGreenCount = 0;
  static uint32_t rGreenCount = 0;
  if (isGreen(color_l)) {
    lGreenCount++;
  } else if (lGreenCount != 0) {
    lGreenCount--;
  }
  if (isGreen(color_r)) {
    rGreenCount++;
  } else if (rGreenCount != 0) {
    rGreenCount--;
  }

  if (lGreenCount > 3 && rGreenCount > 3) {
    haltForever("GREEN DETECTED");
  }

  if (lGreenCount > 5) {
    Serial.println("GREEN LEFT");
    green(false);
    lGreenCount = 0;
  }

  if (rGreenCount > 5) {
    Serial.println("GREEN RIGHT");
    green(true);
    rGreenCount = 0;
  }

  if (isBlack(color_l) && !isBlack(color_r)) {
    dir = "L";
    driveMotors(TURN1, TURN2);
    delay(TURN_DELAY_MS);
  } else if (isBlack(color_r) && !isBlack(color_l)) {
    dir = "R";
    driveMotors(TURN2, TURN1);
    delay(TURN_DELAY_MS);
  } else {
    dir = "F";
    driveMotors(BASE, BASE);
  }

  // static uint32_t lastLoopUs = 0;
  // static uint32_t maxLatencyUs = 0;
  // static uint32_t sumLatencyUs = 0;
  // static uint32_t loopCount = 0;
  // uint32_t loopStartUs = micros();
  // if (lastLoopUs) {
  //   uint32_t dt = loopStartUs - lastLoopUs;
  //   if (dt > maxLatencyUs) maxLatencyUs = dt;
  //   sumLatencyUs += dt;
  //   loopCount++;
  // }
  // lastLoopUs = loopStartUs;
  // static uint32_t lastPrint = 0;
  // uint32_t now = millis();
  // if (now - lastPrint >= 1000) {
  //   lastPrint = now;
  //   float avgMs = loopCount ? (sumLatencyUs / 1000.0f) / loopCount : 0.0f;
  //   float maxMs = maxLatencyUs / 1000.0f;
  //   Serial.printf("T:%8u\tavg:%6.2fms\tmax:%6.2fms\n",
  //                 now, avgMs, maxMs);
  //   maxLatencyUs = 0;
  //   sumLatencyUs = 0;
  //   loopCount = 0;
  // }
}
