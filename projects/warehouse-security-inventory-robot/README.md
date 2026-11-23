# Warehouse Security and Inventory Robot

**Controller:** Arduino Uno  
**Status:** Source code available; hardware testing and final validation still required.

## Purpose
A mobile robot concept for warehouse patrol and basic security monitoring. It uses a PIR sensor to detect motion, an HC-SR04 ultrasonic sensor to measure obstacles, a servo to scan different directions, an LCD for messages and a buzzer for alerts.

## Hardware represented in this sketch
- Arduino Uno
- L298N motor driver
- HC-SR04 ultrasonic sensor
- PIR motion sensor
- Servo scanner
- 16×2 I2C LCD (address 0x27)
- Buzzer
- Two motor channels (left and right)

## Pin allocation
| Arduino pin | Connection |
|---|---|
| D2 | HC-SR04 TRIG |
| D3 | HC-SR04 ECHO |
| D4 | PIR output |
| D5 | L298N ENA (left PWM) |
| D6 | L298N ENB (right PWM) |
| A0, A1 | L298N IN1, IN2 |
| D7, D8 | L298N IN3, IN4 |
| D9 | Servo signal |
| D11 | Buzzer |

## Files
- [warehouse_security_robot.ino](warehouse_security_robot.ino) — Arduino sketch.

## Before running
1. Confirm the actual wiring matches the pin table.
2. Use a separate suitable motor supply and connect grounds correctly.
3. Check the L298N enable-jumper arrangement before using PWM on ENA/ENB.
4. Keep the robot's wheels off the floor during initial motor tests.
5. Test the emergency stop / power isolation method before any autonomous movement.
6. Calibrate motor direction, speed, turning time and sensor mounting.
7. Verify the LCD library and I2C address used by your hardware.

## Current limitations
- The sketch uses timed motor movement; it does not use wheel encoder feedback.
- The sketch does not include ESP8266/ESP32 cloud telemetry or HC-05 remote control.
- A PIR sensor detects changes in infrared energy; it does not identify a person.
- The source has not been confirmed as compiled or successfully tested on the final robot.

Do not describe the robot as fully completed until the hardware tests and demonstration evidence are added.
