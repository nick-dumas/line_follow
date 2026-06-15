//! Line follower robot code for ESP32 using TB6612FNG motor driver, TCA9548A multiplexer, and TCS34725 color sensors.
//!

#include <Wire.h>
#include <Adafruit_TCS34725.h>

// -------------------- Motor class --------------------

#define DEFAULTSPEED 150

class Motor
{
  public:
    Motor(int In1pin, int In2pin, int PWMpin, int offset) {
      In1 = In1pin;
      In2 = In2pin;
      PWM = PWMpin;
      Offset = offset;

      pinMode(In1, OUTPUT);
      pinMode(In2, OUTPUT);
      pinMode(PWM, OUTPUT);
    }

    void drive(int speed) {
      speed = constrain(speed, -255, 255);
      speed = speed * Offset;

      if (speed >= 0) {
        fwd(speed);
      } else {
        rev(-speed);
      }
    }

    void drive(int speed, int duration) {
      drive(speed);
      delay(duration);
    }

    void brake() {
      digitalWrite(In1, HIGH);
      digitalWrite(In2, HIGH);
      analogWrite(PWM, 0);
    }

  private:
    int In1, In2, PWM, Offset;

    void rev(int speed) {
      speed = constrain(speed, 0, 255);
      digitalWrite(In1, HIGH);
      digitalWrite(In2, LOW);
      analogWrite(PWM, speed);
    }

    void fwd(int speed) {
      speed = constrain(speed, 0, 255);
      digitalWrite(In1, LOW);
      digitalWrite(In2, HIGH);
      analogWrite(PWM, speed);
    }
};

void brake(Motor motor1, Motor motor2) {
  motor1.brake();
  motor2.brake();
}

// -------------------- Known-working TB6612FNG pin map --------------------

#define AIN1 12
#define BIN1 9
#define AIN2 13
#define BIN2 8
#define PWMA 10
#define PWMB 11

const int offsetA = -1;
const int offsetB = 1;

Motor left_motor = Motor(AIN1, AIN2, PWMA, offsetA);  // Left motor, assumed
Motor right_motor = Motor(BIN1, BIN2, PWMB, offsetB);  // Right motor, assumed

// -------------------- TCA9548A multiplexer --------------------

#define TCA_ADDR 0x70

const uint8_t LEFT_SENSOR_CHANNEL = 3;
const uint8_t RIGHT_SENSOR_CHANNEL = 0;

// -------------------- TCS34725 --------------------

Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_24MS,
  TCS34725_GAIN_4X
);

// -------------------- Line follower tuning --------------------

const uint16_t LINE_THRESHOLD = 1000;

// Loop delay
const int loopDelayMs = 20;

// -------------------- TCA helper --------------------

void tcaSelect(uint8_t channel) {
  if (channel > 7) return;

  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

// -------------------- Sensor helper --------------------

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

// -------------------- Motor helper --------------------

void driveMotors(int leftSpeed, int rightSpeed) {
  leftSpeed = constrain(leftSpeed, -255, 255);
  rightSpeed = constrain(rightSpeed, -255, 255);

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

  bool leftSensorOk = initSensor(LEFT_SENSOR_CHANNEL);
  bool rightsensorOk = initSensor(RIGHT_SENSOR_CHANNEL);

  Serial.print("Left Sensor: ");
  Serial.println(leftSensorOk ? "OK" : "FAIL");

  Serial.print("Right Sensor: ");
  Serial.println(rightsensorOk ? "OK" : "FAIL");

  if (!leftSensorOk || !rightsensorOk) {
    while (1) {
      stopMotors();
      delay(100);
    }
  }

  Serial.println("Line follower ready.");
  driveMotors(150, 150);
}

// -------------------- Main loop --------------------

void loop() {
  uint16_t clear1 = readClear(LEFT_SENSOR_CHANNEL);
  uint16_t clear2 = readClear(RIGHT_SENSOR_CHANNEL);

  bool leftOnLine = clear1 < LINE_THRESHOLD;
  bool rightOnLine = clear2 < LINE_THRESHOLD;

  // --------------------------------------------------
  // Incremental, non-PID line following logic
  // --------------------------------------------------


  if (leftOnLine) {
    if (rightOnLine) {
      driveMotors(150, 150);
    } else {
      driveMotors(-200, 150);
    }
  } else {
    if (rightOnLine) {
      driveMotors(150, -200);
    } else {
      driveMotors(150, 150);
    }
  }
  // driveMotors(100, 100);

  Serial.print("C1: ");
  Serial.println(clear1);
  Serial.print("C2: ");
  Serial.println(clear2);
  Serial.print("L: ");
  Serial.println(leftOnLine);
  Serial.print("R: ");
  Serial.println(rightOnLine);


  delay(loopDelayMs);
}