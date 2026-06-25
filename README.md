# RoboCup Rescue Robot

A two-sensor line-following robot with an ESP32 for [RoboCup Rescue Australia Junior](https://www.robocupjunior.org.au/wp-content/uploads/2026/02/RCJA-Rescue-Line-Rules-2026.pdf). This is intended to guide learning, the functionality is intentionally incomplete.

![The assembled robot, top view](images/robot-top.jpg)

![The assembled robot, side view](images/robot-side.jpg)

## Hardware

- ESP32 DevKit (WROOM-32)
- TB6612FNG motor driver
- 2× TCS34725 RGB colour sensor
- 2× TT motor (220:1) + wheel
- HC-SR04 ultrasonic distance sensor
- Caster ball
- 4×AA battery holder with switch
- Breadboard and 22 AWG hookup wire

![Underside of the robot showing motors, colour sensors, caster ball and battery](images/robot-underside.jpg)

![Front view of the robot showing the HC-SR04 ultrasonic sensor](images/ultrasonic-front.jpg)

## Bill of Materials

Indicative prices in AUD, ex-shipping, with the cheapest viable option chosen.
The AU column links directly to Australian-supplier products (Core Electronics,
Zaitronics, Jaycar); the "any supplier"
column links to AliExpress/Amazon searches (individual listings expire too fast to
link directly) and is cheaper but slower to ship.

| Item                          | Qty | AU supplier (AUD) | Any supplier (AUD) |
| ----------------------------- | --- | ----------------: | -----------------: |
| ESP32 DevKit (WROOM-32)       | 1   | [$13](https://zaitronics.com.au/products/esp32-wifi-bluetooth-development-board) | [$6](https://www.aliexpress.com/wholesale?SearchText=ESP32+WROOM-32+devkit) |
| TB6612FNG motor driver        | 1   | [$13](https://core-electronics.com.au/tb6612fng-dual-motor-driver-carrier.html) | [$2](https://www.aliexpress.com/wholesale?SearchText=TB6612FNG) |
| TCS34725 RGB colour sensor    | 2   | [$13](https://core-electronics.com.au/rgb-colour-sensor-tcs34725.html) | [$7](https://www.aliexpress.com/wholesale?SearchText=TCS34725) |
| TT motor + wheel              | 2   | [$16](https://core-electronics.com.au/dc-gearbox-motor-tt-motor-200rpm-3-to-6vdc.html) | [$5](https://www.aliexpress.com/wholesale?SearchText=TT+motor+gearbox+wheel) |
| HC-SR04 ultrasonic sensor     | 1   | [$5](https://core-electronics.com.au/hc-sr04-ultrasonic-module-distance-measuring-sensor.html) | [$2](https://www.aliexpress.com/wholesale?SearchText=HC-SR04) |
| Caster ball                   | 1   | [$3](https://core-electronics.com.au/pololu-ball-caster-with-3-8-plastic-ball.html) | [$1](https://www.aliexpress.com/wholesale?SearchText=ball+caster+robot) |
| 4×AA battery holder w/ switch | 1   | [$3](https://core-electronics.com.au/4xaa-battery-holder-square-with-cover.html) | [$1](https://www.aliexpress.com/wholesale?SearchText=4xAA+battery+holder+switch) |
| Breadboard + jumper wire      | 1   | [$10](https://core-electronics.com.au/jumper-wire-kit-for-solderless-breadboard-140-pcs.html) | [$4](https://www.aliexpress.com/wholesale?SearchText=breadboard+jumper+wire+kit) |
| **Total**                     |     |           **~$76** |             **~$28** |

Core stocks the TT motor at 1:48 (linked) and 1:90 rather than the 220:1 used here;
a slower ratio tracks lines more easily, but check minimum RPM (see [Motors](#motors)).
The ESP32 is the cheapest 38-pin board from Zaitronics (micro-USB) — match your USB cable to it.

## Equipment

Reusable tools that aren't consumed by a single build — buy once, share across robots.

| Tool                                | Approx. (AUD) |
| ----------------------------------- | ------------: |
| [Low Cost Digital Multimeter (QM1500)](https://www.jaycar.com.au/low-cost-digital-multimeter-dmm/p/QM1500) | $15 |
| [Duratech 48W Soldering Station (TS1620)](https://www.jaycar.com.au/duratech-48w-temperature-controlled-soldering-station/p/TS1620) | $60 |
| [Wire Stripper / Cutter / Pliers (TH1841)](https://www.jaycar.com.au/stainless-steel-wire-stripper-cutter-pliers/p/TH1841) | $25 |
| [DuPont jumper wires, M/F 40pc (CE09606)](https://core-electronics.com.au/male-to-female-dupont-line-40-pin-10cm-24awg.html) | $5 |
| [22 AWG hookup wire spool set (ADA1311)](https://core-electronics.com.au/hook-up-wire-spool-set-22awg-solid-core-6-x-25-ft.html) | $30 |
| [Third Hand PCB Holder (TH1982)](https://www.jaycar.com.au/third-hand-pcb-holder-tool-with-2-clips-and-heavy-base/p/TH1982) | $18 |
| [10W Hot Glue Gun (TH2050)](https://www.jaycar.com.au/10w-hot-glue-gun-suits-7mm-glue-sticks/p/TH2050) | $10 |
| [15 Piece Micro Driver Set (TD2069)](https://www.jaycar.com.au/15-piece-micro-driver-set/p/TD2069) | $15 |
| [USB-A to Micro-B Cable 1.8m (WC7724)](https://www.jaycar.com.au/usb-a-to-usb-micro-b-cable-1-8m/p/WC7724) | $17 |

## Wiring

| Peripheral            | ESP32 pins                          |
| --------------------- | ----------------------------------- |
| Right motor (TB6612)  | FWD 25, BWD 33, PWM 32              |
| Left motor (TB6612)   | FWD 26, BWD 27, PWM 14              |
| Left colour sensor    | I2C bus 0 - SDA 18, SCL 5          |
| Right colour sensor   | I2C bus 1 - SDA 17, SCL 16         |
| Ultrasonic (HC-SR04)  | TRIG 2, ECHO 4                      |
| Start button          | GPIO 0 (on-board BOOT button)      |

The two colour sensors share the same I2C address, so each sits on a separate ESP32
hardware I2C bus (see [I2C](#i2c) below).

![Breadboard wiring detail](images/wiring-detail.jpg)

## Getting Started

I used visual studio code with PlatformIO, but the code can also run with the Arduino IDE.

1. **Install Visual Studio Code** — download from
   [code.visualstudio.com](https://code.visualstudio.com/) and run the installer.

2. **Install the PlatformIO IDE extension** — in VS Code open the Extensions panel
   (`Ctrl+Shift+X`), search for *PlatformIO IDE*, and click Install. It bundles
   PlatformIO Core and the toolchains, so no separate Python or compiler setup is
   needed. Wait for the one-time install to finish and reload when prompted.

3. **USB-to-serial driver**
   - **Windows** — open Device Manager and look under *Ports (COM & LPT)* for an
     entry like `Silicon Labs CP210x ... (COM3)` or `USB-SERIAL CH340 (COM5)`. A
     device under *Other devices* with a yellow warning triangle means the driver
     is missing, and will need to be installed
   - **macOS / Linux** - Should work by default

4. **Open the project** — clone or download this repository, then in VS Code choose
   *File → Open Folder* and select the `line_follow_esp32` folder. PlatformIO reads
   [platformio.ini](platformio.ini) and downloads the ESP32 platform and the
   Adafruit TCS34725 library automatically on the first build.

5. **Connect the ESP32** over USB and use the PlatformIO toolbar at the bottom of
   VS Code, or the terminal:

   ```sh
   pio run                 # compile
   pio run --target upload # flash over USB
   pio device monitor      # serial monitor @ 115200 baud
   ```

## Behaviour

On startup, print sensor information until button pressed. Then, obot moves forward until one sensor sees black. The robot will turns towards the black to keep straddling the line. When green shortcut is detected, it will make a turn in that direction. If both sensors see green, it will halt.

## Tips

### Complexity

Reducing complexity means more reliable, cleaper, and less time wasted. I recommend avoid adding hardware that isn't needed, such as additional microcontrollers, batteries, or sensors. The most important functionality is the line follow and shortcuts, these must be thoroughly tested for reliability because if you don't make it to the obstacles and oil spill, then you can't earn any points for them.

### Microcontroller

Microcontrollers differ in their capabilities, so check that your chosen board has
enough pins of the right type for your peripherals: regular GPIO, I2C, PWM, and
analog input. Some pins are reserved for internal purposes, boot settings, buttons, LEDs which should be avoided. I chose the ESP32 because it has plenty of pins, multiple I2C buses, fits
a breadboard, includes WiFi and Bluetooth, has USB-C, and is cheap. The microbit is another compelling option which includes a speaker, LED matrix, microphone, accelerometer, bluetooth, visual programming, and buttons but only had 1 I2C and less GPIO pins.

### I2C

I2C is a bus protocol that drives multiple devices over SDA, SCL, VCC, and GND.
However, most microcontrollers expose a single I2C bus on fixed pins, and you can't
put two devices with the same address on one bus. This causes problems when connecting
multiple colour sensors. Your options:

- Sensors with a configurable address (e.g. OPT4048)
- An I2C multiplexer (e.g. TCA9548A)
- A microcontroller with multiple I2C buses (e.g. ESP32)
- "Bit-banging"
- Multiple microcontrollers ☹️

### Claw mechanism

Lego can be attached to a any motor with bolts or super glue. For the claw mechanism, I would recommend a servo motor because they are small, powerful, and have inbuilt position knowledge.

### Motors

TT motors have different speed ranges depending on gear ratio. The common 50:1 ratio
usually runs too fast for line following. Check the minimum RPM against your wheel
diameter to confirm the slowest speed isn't too fast.
Metal gear motors are much better than plastic ones. You should ensure your motor comes with the right axle for your wheel, it is difficult to adapt the axles. Motor encoders provide much more precise control, but are more expensive. Also, externally fitted encoders are cumbersome.

### Battery

Motors draw high current, which can trigger a 'brownout'. A brownout is a microcontroller reset from low voltage. A 9V "smoke detector" battery can't deliver enough power to run
motors; I used a 4×AA holder with a switch. If brownouts persist on a decent battery,
capacitors help absorb the surges from motor direction changes. Use the largest you
can (100 µF or more) in parallel with the battery and the microcontroller - the more
the better. If that still isn't enough, run a separate small battery for the digital
electronics and a larger one for the motors.

### Connections

Reliable connections are critical. Soldered joints or screw terminals are the most reliable; breadboards offer convenience, reasonable reliability and compactness for
prototyping. I use 22 AWG solid-core wire cut to length on the breadboard, soldered multicore cable with 0.2" screw terminals to
the motors, and DuPont jumpers for the
ultrasonic and colour sensors.

### Multidirectional wheel

Two driven wheels need a third contact point - an omniwheel, caster wheel, or caster
ball. I used a caster ball. I tried a low-friction sliding "foot" but it hindered turning. Whatever
you pick must clear debris and the bumpy tile.

### Sensor positioning

The height and spacing of the colour sensors matters. Sensors too far ahead of the
wheels can't react to sharp turns; sensors too close together can't keep the line
between them. The sensor ahould be about 0.5 cm above the ground for best color detection.

### Slipping

Weight distribution matters too. Too front- or back-heavy and the robot may roll over
finishing the seesaw. Too little weight over the driven wheels and they lose grip
uphill - especially as they pick up dust. Clean the course and wheels before starting.
