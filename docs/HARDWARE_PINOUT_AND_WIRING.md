# 🔌 Hardware Pinout & Wiring Guide (`HARDWARE_PINOUT_AND_WIRING.md`)

> 🌐 **Language**: **[🇬🇧 English](HARDWARE_PINOUT_AND_WIRING.md)** | **[🇹🇷 Türkçe](HARDWARE_PINOUT_AND_WIRING_TR.md)**

This document provides the complete Arduino pin mappings for all robotics sketches (`raw/` and `firmware/`) in **`WERRobotics`**. All WERR Edge AI firmware sketches (`firmware/`) are **100% pin-compatible** with the original hardware layouts—no rewiring is required.

---

## 1. Autonomous Obstacle Avoidance Robot (`engeldenkacan.ino` & `werr_engelden_kacan.ino`)

- **Target MCU**: Arduino Uno / Arduino Nano (`ATmega328P`)
- **Motor Driver**: L298N Dual H-Bridge DC Motor Driver
- **Range Sensor**: HC-SR04 Ultrasonic Distance Sensor

| Signal / Variable | Arduino Pin | Mode | Description |
| :--- | :---: | :---: | :--- |
| `ir_kumanda` | **D2** | `INPUT` | Infrared (IR) receiver signal pin |
| `OUT1` | **D5** | `OUTPUT` | Right motor forward direction (L298N IN1) |
| `OUT2` | **D6** | `OUTPUT` | Right motor reverse direction (L298N IN2) |
| `OUT3` | **D7** | `OUTPUT` | Left motor forward direction (L298N IN3) |
| `OUT4` | **D8** | `OUTPUT` | Left motor reverse direction (L298N IN4) |
| `motor1` (ENA) | **D9 (PWM)** | `OUTPUT` | Right motor PWM speed control (`0..255`) |
| `motor2` (ENB) | **D10 (PWM)** | `OUTPUT` | Left motor PWM speed control (`0..255`) |
| `echo` | **D11** | `INPUT` | HC-SR04 ultrasonic echo pulse input |
| `trig` | **D12** | `OUTPUT` | HC-SR04 ultrasonic trigger pulse output |

---

## 2. Biomimetic Bionic Butterfly (`kelebek.ino` & `werr_kelebek_biyonik.ino`)

- **Target MCU**: Arduino Uno / Arduino Nano (`ATmega328P`)
- **Sensor**: KY-037 / KY-038 High-Sensitivity Microphone Sound Module (Analog + Digital)
- **Actuators**: 2x SG90 / MG90S Micro Servos + 1x 5V Relay Module

| Signal / Variable | Arduino Pin | Mode | Description |
| :--- | :---: | :---: | :--- |
| `sesAnalogPin` | **A0** | `INPUT` | Sound module `AO` (Analog acoustic intensity) |
| `sesDijitalPin` | **D7** | `INPUT` | Sound module `DO` (Digital threshold trigger, active `LOW`) |
| `srv1` (Left Wing) | **D9 (PWM)** | `SERVO` | Left wing servo (`0° .. 140°`) |
| `srv2` (Right Wing) | **D10 (PWM)** | `SERVO` | Right wing servo (`180° .. 40°` counter-phase) |
| `rolePin` | **D13** | `OUTPUT` | Relay / lighting toggle output triggered on double-clap |

---

## 3. Autonomous Market / Line Robot (`alisveris_roboti.ino` & `werr_alisveris_robotu.ino`)

- **Target MCU**: Arduino Mega 2560 (`ATmega2560`)
- **Display**: 16x2 Parallel Character LCD (`LiquidCrystal(54, 55, 56, 57, 58, 59)`)

| Signal / Variable | Arduino Mega Pin | Mode | Description |
| :--- | :---: | :---: | :--- |
| `INA1`, `INA2` | **D38, D36** | `OUTPUT` | Motor A direction pins |
| `HIZA` | **D44 (PWM)** | `OUTPUT` | Motor A PWM speed pin |
| `INB1`, `INB2` | **D42, D40** | `OUTPUT` | Motor B direction pins |
| `HIZB` | **D46 (PWM)** | `OUTPUT` | Motor B PWM speed pin |
| `onFar`, `arkFar` | **D51, D48** | `OUTPUT` | Front headlights & rear brake lights |
| `solSin`, `sagSin` | **D53, D49** | `OUTPUT` | Left & right turn signal indicators |
| `solSen`, `ortSen`, `sagSen` | **A6, A7, A8** | `INPUT` | Left, center, and right IR obstacle/line sensors |
| `lcd` | **D54..D59 (A0..A5)** | `OUTPUT` | 16x2 character LCD interface pins |

---

## 4. Bipedal Humanoid Companion Robot: Teknobot (`TeknoBotv4_ino.ino` & `werr_teknobot.ino`)

- **Target MCU**: Arduino Uno / Arduino Nano (`ATmega328P`)
- **Original Hardware Design**: Burak Salar (`TeknoBotv4_ino.ino`)
- **Sensors**: LSM303 3-Axis Accelerometer (I2C), HC-SR04 Ultrasonic, LDR Light, Analog Sound Microphone, Buzzer
- **Actuators**: 6x SG90 / MG90S Micro Servos (2 Ankle Feet + 2 Hip Legs + 2 Arms)

| Signal / Variable | Arduino Pin | Mode | Description |
| :--- | :---: | :---: | :--- |
| `solAyak` | **D2 (PWM)** | `SERVO` | Left foot ankle tilt servo (Center: `90°`) |
| `sagAyak` | **D3 (PWM)** | `SERVO` | Right foot ankle tilt servo (Center: `90°`) |
| `solBacak` | **D4 (PWM)** | `SERVO` | Left leg/hip stride servo (Center: `90°`) |
| `sagBacak` | **D5 (PWM)** | `SERVO` | Right leg/hip stride servo (Center: `90°`) |
| `solKol` | **D6 (PWM)** | `SERVO` | Left arm servo (Neutral: `45°`, Folded: `15°`, Raised: `145°`) |
| `sagKol` | **D7 (PWM)** | `SERVO` | Right arm servo (Neutral: `135°`, Folded: `165°`, Raised: `35°`) |
| `trigPin` | **D8** | `OUTPUT` | HC-SR04 ultrasonic trigger pulse |
| `echoPin` | **D9** | `INPUT` | HC-SR04 ultrasonic echo pulse |
| `buzzerPin` | **D13** | `OUTPUT` | Piezo buzzer acoustic alarm & melody output |
| `ldrPin` | **A0** | `INPUT` | LDR ambient light analog sensor input |
| `sesPin` | **A1** | `INPUT` | Analog sound sensor intensity input |
| `LSM303 SDA` | **A4** | `I2C` | LSM303 accelerometer I2C data bus |
| `LSM303 SCL` | **A5** | `I2C` | LSM303 accelerometer I2C clock bus |

