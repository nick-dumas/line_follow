# line_follow_esp32

A two-sensor line-following robot for the ESP32, built with [PlatformIO](https://platformio.org/).

## Hardware

| Component | Role |
|-----------|------|
| ESP32 DevKit (WROOM-32) | Controller (`esp32dev`) |
| TB6612FNG | Dual DC motor driver |
| TCA9548A | I²C multiplexer (lets two identical sensors share one bus) |
| 2 × TCS34725 | RGB/clear color sensors (left & right) |

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

On boot the monitor prints sensor init status, then a per-loop telemetry line:

```
C1:842 C2:1503 L:1 R:0
```

(`C1`/`C2` = clear readings, `L`/`R` = on-line flags.)

## Notes

- `analogWrite()` requires the **arduino-esp32 core 3.x**, which the current
  `platform = espressif32` pulls in by default. If you pin an older platform
  version, switch the PWM calls to the LEDC API.
- The `Adafruit TCS34725` dependency is declared in [platformio.ini](platformio.ini);
  PlatformIO resolves Adafruit BusIO and Adafruit Unified Sensor automatically.
