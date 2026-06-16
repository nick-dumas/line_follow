//! Line follower robot for ESP32: TB6612FNG motor driver, TCA9548A I2C mux, dual TCS34725 color sensors.

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>

// -------------------- Motor --------------------

class Motor {
  public:
    Motor(int in1, int in2, int pwm, int offset)
      : in1_(in1), in2_(in2), pwm_(pwm), offset_(offset) {
      pinMode(in1_, OUTPUT);
      pinMode(in2_, OUTPUT);
      pinMode(pwm_, OUTPUT);
    }

    void drive(int speed) {
      speed = constrain(speed, -255, 255) * offset_;
      digitalWrite(in1_, speed < 0);
      digitalWrite(in2_, speed >= 0);
      analogWrite(pwm_, abs(speed));
    }

    void brake() {
      digitalWrite(in1_, HIGH);
      digitalWrite(in2_, HIGH);
      analogWrite(pwm_, 0);
    }

  private:
    int in1_, in2_, pwm_, offset_;
};

// -------------------- TB6612FNG pin map --------------------

#define AIN1 12
#define BIN1 9
#define AIN2 13
#define BIN2 8
#define PWMA 10
#define PWMB 11

const int offsetA = -1;
const int offsetB = 1;

Motor left_motor(AIN1, AIN2, PWMA, offsetA);
Motor right_motor(BIN1, BIN2, PWMB, offsetB);

// -------------------- TCA9548A mux + TCS34725 --------------------

#define TCA_ADDR 0x70
const uint8_t LEFT_SENSOR_CHANNEL = 3;
const uint8_t RIGHT_SENSOR_CHANNEL = 0;

Adafruit_TCS34725 tcs(TCS34725_INTEGRATIONTIME_24MS, TCS34725_GAIN_4X);

// -------------------- Tuning --------------------

const uint16_t LINE_THRESHOLD = 1000;
const int BASE = 150;
const int TURN = 200;
const int loopDelayMs = 20;

// -------------------- Helpers --------------------

void tcaSelect(uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

bool initSensor(uint8_t channel) {
  tcaSelect(channel);
  delay(10);
  return tcs.begin();
}

uint16_t readClear(uint8_t channel) {
  uint16_t r, g, b, c;
  tcaSelect(channel);
  delay(2);
  tcs.getRawData(&r, &g, &b, &c);
  return c;
}

void driveMotors(int leftSpeed, int rightSpeed) {
  left_motor.drive(leftSpeed);
  right_motor.drive(rightSpeed);
}

void stopMotors() {
  left_motor.brake();
  right_motor.brake();
}

// -------------------- Setup --------------------

void setup() {
  Serial.begin(115200);
  Wire.begin();
  stopMotors();

  bool leftOk = initSensor(LEFT_SENSOR_CHANNEL);
  bool rightOk = initSensor(RIGHT_SENSOR_CHANNEL);
  Serial.printf("Left Sensor: %s\n", leftOk ? "OK" : "FAIL");
  Serial.printf("Right Sensor: %s\n", rightOk ? "OK" : "FAIL");

  if (!leftOk || !rightOk) {
    while (1) {
      stopMotors();
      delay(100);
    }
  }

  Serial.println("Line follower ready.");
  driveMotors(BASE, BASE);
}

// -------------------- Loop --------------------

void loop() {
  uint16_t clear1 = readClear(LEFT_SENSOR_CHANNEL);
  uint16_t clear2 = readClear(RIGHT_SENSOR_CHANNEL);

  bool leftOnLine = clear1 < LINE_THRESHOLD;
  bool rightOnLine = clear2 < LINE_THRESHOLD;

  if (leftOnLine == rightOnLine) driveMotors(BASE, BASE);   // both or neither
  else if (leftOnLine)           driveMotors(-TURN, BASE);  // turn left
  else                           driveMotors(BASE, -TURN);  // turn right

  Serial.printf("C1:%u C2:%u L:%d R:%d\n", clear1, clear2, leftOnLine, rightOnLine);

  delay(loopDelayMs);
}
