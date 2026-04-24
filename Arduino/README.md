# Arduino
 
This folder contains the Arduino control code for the AI Driver Monitoring Smart Dashboard project.
 
---
 
## Contents
 
| File | Description |
|------|-------------|
| `main.ino` | Full Arduino control code — motor control, alcohol detection, alert levels, and serial communication |
 
---
 
## What the Arduino Does
 
The Arduino is the hardware control brain of the system. It has three responsibilities:
 
**1. Motor control** — drives two DC motors via an L293D motor driver using PWM speed control. Under normal operation the car moves forward at a set speed. When impairment is detected, the Arduino executes a smooth parking stop that curves the vehicle to the left by ramping the left motor down faster than the right.
 
**2. Alcohol detection** — reads the MQ-3 alcohol sensor on pin A0 continuously. If the analog value exceeds the threshold (800/1023), it immediately triggers the parking stop and alert sequence independently of the Raspberry Pi. This independence is a deliberate safety design — alcohol response works even if the Pi crashes or the serial connection drops.
 
**3. Serial communication** — listens on the USB serial port (9600 baud) for commands from the Raspberry Pi. It responds to two commands:
- `DROWSY` — triggers parking stop and alerts
- `OK` — resumes normal forward motion and silences alerts
---
 
## Pin Connections
 
| Pin | Component | Role |
|-----|-----------|------|
| D2 | LED | Visual alert |
| D3 | Buzzer | Audible alert (2000Hz tone) |
| D5 | L293D ENA | Motor A speed (PWM) |
| D6 | L293D ENB | Motor B speed (PWM) |
| D8 | L293D IN1 | Motor A direction |
| D9 | L293D IN2 | Motor A direction |
| D10 | L293D IN3 | Motor B direction |
| D11 | L293D IN4 | Motor B direction |
| A0 | MQ-3 sensor | Alcohol level (analog read) |
| A4 | MPU6050 + LCD | I2C SDA (shared bus) |
| A5 | MPU6050 + LCD | I2C SCL (shared bus) |
 
---
 
## How to Upload
 
1. Open `main.ino` in the Arduino IDE
2. Select **Board → Arduino Uno**
3. Select the correct **Port** (the USB port your Arduino is connected to)
4. Click **Upload**
The Arduino will reset after upload and begin running immediately. Open the Serial Monitor at 9600 baud to see status messages.
 
---
 
## Key Functions
 
| Function | What it does |
|----------|-------------|
| `moveForward(speed)` | Drives both motors forward at the given PWM speed (0–255) |
| `parkingStop()` | Ramps left motor down faster than right, curving the vehicle left, then stops completely |
| `stopMotors()` | Cuts power to all motors immediately |
| `soundAlert()` | Activates buzzer at 2000Hz and turns on LED |
| `silenceAlert()` | Turns off buzzer and LED |
 
---
 
## Tunable Parameters
 
These values at the top of `main.ino` can be adjusted to tune system behaviour:
 
| Parameter | Default | Effect |
|-----------|---------|--------|
| `ALCOHOL_THRESHOLD` | 800 | Raise to make alcohol detection less sensitive, lower to make it more sensitive |
| `NORMAL_SPEED` | 180 | Normal driving speed (0–255 PWM) |
| `BUZZER_FREQUENCY` | 2000 | Alert tone pitch in Hz — higher = higher pitched beep |
 
---
 
## Serial Commands Reference
 
The Arduino listens for these commands from the Raspberry Pi over USB serial at 9600 baud:
 
| Command | Arduino Response |
|---------|-----------------|
| `DROWSY\n` | Executes parking stop, activates buzzer and LED |
| `OK\n` | Resumes forward motion, silences alerts |
 
Note: Commands must end with a newline character (`\n`) — the Arduino uses `readStringUntil('\n')` to know when a message is complete.
 
---
 
## Notes
 
- ENA and ENB on the L293D must be connected to PWM-capable pins (D5 and D6) for speed control to work
- Remove the default jumper caps from ENA and ENB on the L293D before connecting to Arduino pins
- MQ-3 sensor requires 20–30 seconds warm-up time after power on before readings are reliable
- The parking stop lockout keeps the vehicle stopped until alcohol level drops below threshold — this prevents the car from immediately resuming after a brief dip in sensor reading

