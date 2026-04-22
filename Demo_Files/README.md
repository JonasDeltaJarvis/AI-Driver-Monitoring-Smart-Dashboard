# Demo Files
 
This folder contains all software, data, and demo outputs for the AI Driver Monitoring Smart Dashboard project.
 
---
 
## Contents
 
| File | Description |
|------|-------------|
| `drowsiness_detection.ipynb` | Main Python notebook, runs the full drowsiness detection pipeline |
| `drowsiness_log.csv` | Frame-by-frame data log from the test session (frame number, timestamp, EAR value, drowsy status) |
| `drowsiness_analysis.png` | Three analysis graphs generated from the session data |
| `drowsiness_output_h264.mp4` | Annotated output video with live EAR overlay and drowsy/OK status |
 
---
 
## How to Run the Notebook
 
1. Go to [colab.research.google.com](https://colab.research.google.com)
2. Click **File → Upload notebook** and select `drowsiness_detection.ipynb`
3. Run the first cell to install dependencies:
   ```
   pip install mediapipe==0.10.33 opencv-python-headless numpy
   ```
4. Download the face landmark model by running the second cell
5. Upload your own test video when prompted, or use the provided `drowsiness_output_h264.mp4` as reference
6. Run all remaining cells in order
---
 
## Dependencies
 
| Library | Version | Purpose |
|---------|---------|---------|
| MediaPipe | 0.10.33 | Face mesh landmark detection (468 points) |
| OpenCV | Latest | Video capture and frame processing |
| NumPy | Latest | EAR distance calculations |
| Matplotlib | Latest | Graph generation |
 
---
 
## What the Detection Does
 
Each video frame is passed through MediaPipe's face mesh neural network. Six landmark points around each eye are extracted and used to compute the **Eye Aspect Ratio (EAR)**:
 
```
EAR = (A + B) / (2 × C)
```
 
Where A and B are vertical eye distances and C is the horizontal eye width. An open eye produces EAR ≈ 0.33. When EAR drops below 0.22 for 15 consecutive frames (~0.5 seconds), the driver is classified as drowsy.
 
---
 
## Test Results
 
Results from the included test session:
 
| Metric | Value |
|--------|-------|
| EAR when eyes open | ≈ 0.33 |
| EAR when eyes closed | ≈ 0.07 |
| Threshold used | 0.22 |
| Frames to trigger alert | 15 (~0.5 seconds) |
| False positives from blinking | 0 |
 
The graphs in `drowsiness_analysis.png` show the full session breakdown — EAR over time, drowsy/OK status over time, and a pie chart summary of the session.
 
---
 
## Notes
 
- The notebook was developed and tested on Google Colab using a Chromebook
- When deployed on Raspberry Pi, the video input (`cv2.VideoCapture`) is replaced with a live camera feed, everything else remains identical
- The serial communication line (`arduino.write(b'DROWSY\n')`) is commented out in the notebook since no Arduino is connected during software development
