# Robocup Rescue robot

A two-sensor line-following robot for the ESP32, built with [PlatformIO](https://platformio.org/).

## Hardware

- ESP32 DevKit (WROOM-32)
- TB6612FNG Motor Driver
- Breadboard
- Hookup Wire
- 2× TCS34725 RGB Sensor
- 2× TT Motor (220:1) + Wheel
- HC-SR04 Ultrasonic Distance Sensor

## Tips

### Microcontroller

Microcontrollers all have different capabilities. You want to check your chosen microcontroller has enough pins to support your requirements. In most microcontrollers, not all pins have the same capabilities. You'll need to check that you have enough regular GPIO pins, enough I2C pins, enough PWM pins, and enough analog input pins for your peripherals. Some pins will also be reused internally by the board for boot settings, buttons, leds, etc and should be avoided to maintain functionality. I used the ESP32 because it has lots of pins, multiple I2C buses, fits onto a breadboard, has wifi & bluetooth, has USBC, and is cheap.

### I2C

I2C is a bus protocol that allows multiple devices and complex functionality to work with only an SDA pin, CLK pin, VCC and GND. However, most microcontrollers only support a single I2C bus with fixed pins. You can't connect multiple devices with the same address to the same bus. This usually causes problems with trying to connect multiple color sensors. Here's your options:
- Sensors with configurable address (eg OPT4048)
- I2C Multiplexer (eg TCA9548A)
- Microcontroller with multiple I2C buses (eg ESP32)
- Multiple microcontrollers ☹️

### Motor

TT motors have a different speed ranges depending on the gear ratio. The most common gear ratio is 50:1, which usually operates too fast for a line following robot. Use the minimum RPM with your wheel diameter to check that the minimum speed isn't too fast.

### Battery

Motors have a high current draw, which can cause 'brownout'. This is when the microcontroller resets from low voltage level. You need to ensure you supply enough power to keep the microcontroller running. A 9V 'smoke detector' battery can't deliver enough power to run motors. I used a 4xAA battery holder with a switch. If you are using a reasonable battery and still getting brownouts, capacitors can help stabilise the surges from motor direction changes. Capacitors should be as larges as possible, ideally 100uF or larger, and should be placed in parallel with the battery and the microcontroller. The more, the better. If capacitors and a decent battery still isn't enough, you could use a dedicated small battery for digital and a dedicated large battery for the motors.

### Connections

Reliable electrical connections are critical to a functioning robot. Ideally, every connection is made with soldering or screw terminals, but this is difficult without manufacturing a PCB. Breadboards offer a reasonable level of reliability and compactness for prototyping. I use 22 AWG solid-core wire, with a wire stripper to make custom lengths to connect components on a breadboard. I soldered some multicore cabling to the motors, and used 0.2" screw terminals to connect the motor to the breadboard. I also used a dupont jumper wire kit to connect the ultrasonic and color sensors.

### Multidirectional Wheel

In addition to two wheels, a third wheel contact point is needed. You can use a 'omniwheel', a caster wheel, or a caster ball. I used a caster ball. I tried a low-friction sliding 'foot', but it unfortunately hindered the robot turning. The chosen solution will need to be able to pass over debris and the bumps tile.

### Sensor positioning

The height and position relative to the wheels of the color sensors is really important. If the wheels are too far away from the sensors, the robot will not be able to adjust to sharp turns. If the sensors are too close together, the robot may not be able to keep the line between the sensors.

### Slipping

The weight distribution can also be important. If the robot is too front-heavy or back-heavy, it may roll over when finishing the seesaw. If there is not enough weight over the motor wheels, they may not have enough force to grip when going uphill. This is especially noticable when wheels pick up dust from the course. If this is a concern then the course and the wheels should be cleaned before starting.
