# AI Driver Monitoring Smart Dashboard

## Overview
This project is an AI-powered smart car dashboard system designed to enhance road safety by detecting driver impairment (drowsiness and alcohol influence) and executing intelligent safety interventions.

The system integrates **computer vision**, **embedded systems**, and **multi-sensor data fusion** to monitor driver behavior in real time and respond accordingly.

---

## Objectives
- Detect driver drowsiness using computer vision
- Detect alcohol presence using gas sensors
- Monitor driving behavior using motion sensors
- Implement a real-time decision-making system
- Execute safe intervention (warnings, speed reduction, stop)

---

## System Architecture

### Input Layer
- Camera (eye tracking, head movement)
- MQ-3 Alcohol Sensor
- MPU6050 (motion detection)

### Processing Layer
- Raspberry Pi (AI + image processing)
- Arduino (real-time control system)

### Decision Layer
- Risk scoring algorithm based on sensor fusion

### Output Layer
- Motor control (speed reduction / stop)
- Buzzer & LED alerts
- LCD dashboard display

---

## Features

- Real-time drowsiness detection using OpenCV
- Alcohol detection using MQ-3 sensor
- Multi-sensor fusion for improved accuracy
- Tiered safety response system:
  - Level 1: Warning
  - Level 2: Speed reduction
  - Level 3: Full stop
- Live dashboard display of driver status
- Data logging for analysis (optional)

---

## Hardware Components

- Raspberry Pi 4
- Arduino Uno / Nano
- L298N Motor Driver
- DC Motors + Chassis
- MQ-3 Alcohol Sensor
- MPU6050 (Accelerometer + Gyroscope)
- Camera Module / USB Webcam
- LCD Display (16x2 with I2C)
- Buzzer & LEDs
- Power supply (batteries)

---

## Software Stack

- Python (OpenCV, NumPy)
- Arduino C/C++
- Serial Communication (Pi ↔ Arduino)

---

## System Workflow

1. Camera captures driver face
2. AI model detects eye closure and head position
3. Sensors collect alcohol and motion data
4. Data is processed and risk score calculated
5. Arduino executes appropriate action:
   - Warning
   - Slow down
   - Stop vehicle

---

## Testing & Evaluation

The system is tested under different scenarios:
- Normal driving
- Simulated drowsiness
- Alcohol detection
- Sudden movement / instability

Performance metrics:
- Detection accuracy
- Response time
- False positive rate

---

## Challenges & Limitations

- Lighting conditions affect camera accuracy
- Sensor calibration required
- Risk of false positives
- Real-world deployment requires higher reliability

---

## Future Improvements

- Mobile app integration
- GPS tracking and emergency alerts
- Machine learning model optimization
- Integration with real vehicle systems
- Cloud data analysis

---

## Learning Outcomes

This project demonstrates:
- Embedded systems design
- AI & computer vision
- Control systems engineering
- Sensor integration
- Real-time decision making

---

## Author

Jonas (Jarvis 2.0)  

---

## License

This project is open-source and available under the MIT License.
