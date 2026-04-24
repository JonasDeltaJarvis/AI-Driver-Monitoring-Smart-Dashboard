# Raspberry Pi
 
This folder contains the full Python script that runs on the Raspberry Pi for the AI Driver Monitoring Smart Dashboard project.
 
---
 
## Contents
 
| File | Description |
|------|-------------|
| `driver_monitor.py` | Full production Python script — camera capture, drowsiness detection, data logging, and serial communication |
| `face_landmarker.task` | MediaPipe face landmark AI model file (download separately — see setup instructions below) |
| `session_log.csv` | Generated automatically when the script runs — frame-by-frame session data |
 
---
 
## What the Raspberry Pi Does
 
The Raspberry Pi is the AI processing brain of the system. It has four responsibilities:
 
**1. Camera capture** — reads live video frames from the Pi Camera Module via the CSI port using OpenCV.
 
**2. Drowsiness detection** — passes each frame through MediaPipe's face mesh neural network, which identifies 468 facial landmarks. Six landmarks around each eye are used to calculate the Eye Aspect Ratio (EAR). If EAR stays below 0.22 for 15 consecutive frames (~0.5 seconds), the driver is classified as drowsy.
 
**3. Serial communication** — sends commands to the Arduino over USB serial. Sends `DROWSY` when drowsiness is detected, `OK` when the driver recovers. Commands are only sent when the intervention level changes, not every frame.
 
**4. Data logging** — writes every frame's timestamp, EAR value, intervention level, and status to a CSV file for post-session analysis.
 
---
 
## Setup Instructions
 
### Step 1 — Install dependencies
Run this once in the Raspberry Pi terminal:
```
pip install mediapipe==0.10.33 opencv-python pyserial numpy
```
 
### Step 2 — Download the AI model
Run this once to download the face landmark model file:
```python
import urllib.request
urllib.request.urlretrieve(
    "https://storage.googleapis.com/mediapipe-models/face_landmarker/face_landmarker/float16/1/face_landmarker.task",
    "face_landmarker.task"
)
```
Or run it directly in the terminal:
```
wget -O face_landmarker.task https://storage.googleapis.com/mediapipe-models/face_landmarker/face_landmarker/float16/1/face_landmarker.task
```
 
### Step 3 — Enable the camera
If using the Pi Camera Module via CSI:
```
sudo raspi-config
```
Navigate to **Interface Options → Camera → Enable → Reboot**
 
### Step 4 — Connect the Arduino
Plug the Arduino into any USB port on the Pi using a USB A to USB B cable. Verify the port:
```
ls /dev/ttyUSB*
```
It should show `/dev/ttyUSB0`. If it shows a different port, update `SERIAL_PORT` in the configuration section of `driver_monitor.py`.
 
### Step 5 — Run the script
```
python3 driver_monitor.py
```
 
Press **Q** to quit, or **Ctrl+C** to stop from the terminal.
 
---
 
## Configuration
 
All tunable parameters are at the top of `driver_monitor.py`:
 
| Parameter | Default | Effect |
|-----------|---------|--------|
| `SERIAL_PORT` | `/dev/ttyUSB0` | USB port address of the Arduino |
| `BAUD_RATE` | `9600` | Serial communication speed — must match Arduino |
| `CAMERA_INDEX` | `0` | Camera index (0 = first camera) |
| `EAR_THRESHOLD` | `0.22` | EAR below this = eye closed |
| `WARNING_EAR` | `0.28` | EAR below this = early warning |
| `CLOSED_FRAMES` | `15` | Consecutive frames before drowsy alert triggers |
| `LOG_FILE` | `session_log.csv` | Output filename for session data |
| `MODEL_PATH` | `face_landmarker.task` | Path to MediaPipe model file |
 
---
 
## Eye Aspect Ratio (EAR)
 
The EAR formula measures how open the eye is using 6 landmark points:
 
```
EAR = (A + B) / (2 × C)
 
Where:
  A = vertical distance between landmarks 1 and 5
  B = vertical distance between landmarks 2 and 4  
  C = horizontal distance between landmarks 0 and 3
```
 
| Eye state | Typical EAR value |
|-----------|------------------|
| Fully open | ≈ 0.33 |
| Early closing | ≈ 0.28 |
| Closed | ≈ 0.07 |
 
---
 
## Intervention Levels
 
| Level | Condition | Serial command sent |
|-------|-----------|-------------------|
| 1 — OK | EAR above 0.28 | `OK` |
| 2 — Warning | EAR below 0.28 | `OK` (alert handled locally) |
| 3 — Drowsy | EAR below 0.22 for 15+ frames | `DROWSY` |
 
---
 
## Data Logging
 
Every frame is logged to `session_log.csv` with the following columns:
 
| Column | Description |
|--------|-------------|
| `timestamp` | Time of frame capture (HH:MM:SS.ms) |
| `frame` | Frame number |
| `ear` | Eye Aspect Ratio value |
| `level` | Intervention level (1, 2, or 3) |
| `status` | Status label (OK, LEVEL 2 - WARNING, LEVEL 3 - DROWSY) |
 
Use the Jupyter notebook in the `demo files` folder to generate graphs from this CSV.
 
---
 
## Notes
 
- If the Arduino is not connected, the script will print a warning and continue running without serial communication — useful for testing the detection algorithm alone
- The `cv2.imshow` line displays the annotated video feed on screen — remove this line if running the Pi headlessly (without a monitor)
- MediaPipe face detection works best in good lighting — performance degrades significantly in low light conditions
- The script logs every frame regardless of whether a face is detected — frames with no face detected will have empty EAR and level fields in the CSV
 
