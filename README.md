# line_follow_esp32

A two-sensor line-following robot for the ESP32, built with [PlatformIO](https://platformio.org/).

## Hardware

| Component | Role |
|-----------|------|
| ESP32 DevKit (WROOM-32) | Controller (`esp32dev`) |
| TB6612FNG | Dual DC motor driver |
| TCA9548A | I²C multiplexer (lets two identical sensors share one bus) |
| 2 × TCS34725 | RGB/clear color sensors (left & right) |
| HC-SR04 | Ultrasonic distance sensor (pre-start telemetry) |

### Pin map (TB6612FNG)

| Signal | GPIO |
|--------|------|
| AIN1 / AIN2 / PWMA (left motor)  | 12 / 13 / 10 |
| BIN1 / BIN2 / PWMB (right motor) | 9 / 8 / 11 |

`offsetA = -1`, `offsetB = 1` flip motor direction so positive speed drives the robot forward.

### Sensors

Both TCS34725 sensors sit behind the TCA9548A at address `0x70`:

- Left sensor  → mux channel **3**
- Right sensor → mux channel **0**

### HC-SR04 pin map

| Signal | GPIO |
|--------|------|
| TRIG   | 2    |
| ECHO   | 4    |

Wire TRIG → GPIO 2, ECHO → GPIO 4, VCC → 5 V, GND → GND.

## How it works

Each loop reads the *clear* channel of both sensors. A reading below
`LINE_THRESHOLD` (1000) means that sensor is over the (dark) line.

| Left on line | Right on line | Action |
|:---:|:---:|---|
| ✓ | ✓ | Drive straight (`BASE`, `BASE`) |
| ✓ | ✗ | Pivot left (`-TURN`, `BASE`) |
| ✗ | ✓ | Pivot right (`BASE`, `-TURN`) |
| ✗ | ✗ | Drive straight (`BASE`, `BASE`) |

Tuning constants live at the top of [src/main.cpp](src/main.cpp): `LINE_THRESHOLD`,
`BASE` (150), `TURN` (200), `loopDelayMs` (20).

## Build & flash

Requires the [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/) CLI
or the PlatformIO VS Code extension.

```sh
pio run                # compile
pio run --target upload # flash over USB
pio device monitor     # serial console @ 115200
```

On boot the monitor prints sensor init status. **Before the start button (GPIO 0) is pressed**,
a status block prints once per second:

```
Left:
R:  842 G:  901 B:  800 C:  900, !BLK, !GRN
Right:
R: 1400 G: 1500 B: 1300 C: 1503, !BLK, !GRN
DIST: 23.4 cm
```

If the HC-SR04 echo times out (nothing in range / sensor absent) it prints `DIST: timeout`.

After the button is pressed, per-loop telemetry resumes:

```
C1:842 C2:1503 L:1 R:0
```

## Notes

- `analogWrite()` requires the **arduino-esp32 core 3.x**, which the current
  `platform = espressif32` pulls in by default. If you pin an older platform
  version, switch the PWM calls to the LEDC API.
- The `Adafruit TCS34725` dependency is declared in [platformio.ini](platformio.ini);
  PlatformIO resolves Adafruit BusIO and Adafruit Unified Sensor automatically.
