#!/usr/bin/env python3
# =============================================================
# AI Driver Monitoring Smart Dashboard
# Full Raspberry Pi Script — Production Version
# =============================================================
# Author: [Your Name]
# Hardware: Raspberry Pi 4 + Arduino Uno (USB serial)
# Camera:   Pi Camera Module (CSI) or USB webcam
# =============================================================

import cv2
import csv
import time
import serial
import numpy as np
import mediapipe as mp
from datetime import datetime
from mediapipe.tasks import python
from mediapipe.tasks.python import vision

# =============================================================
# SECTION 1: CONFIGURATION
# =============================================================

SERIAL_PORT     = '/dev/ttyUSB0'
BAUD_RATE       = 9600
CAMERA_INDEX    = 0
EAR_THRESHOLD   = 0.22
WARNING_EAR     = 0.28
CLOSED_FRAMES   = 15
LOG_FILE        = 'session_log.csv'
MODEL_PATH      = 'face_landmarker.task'

# =============================================================
# SECTION 2: EYE LANDMARK INDICES
# =============================================================

LEFT_EYE  = [362, 385, 387, 263, 373, 380]
RIGHT_EYE = [33,  160, 158, 133, 153, 144]

# =============================================================
# SECTION 3: EAR FORMULA
# =============================================================

def eye_aspect_ratio(landmarks, eye_indices, w, h):
    pts = [(landmarks[i].x * w, landmarks[i].y * h)
           for i in eye_indices]

    A = np.linalg.norm(np.array(pts[1]) - np.array(pts[5]))
    B = np.linalg.norm(np.array(pts[2]) - np.array(pts[4]))
    C = np.linalg.norm(np.array(pts[0]) - np.array(pts[3]))

    return (A + B) / (2.0 * C)

# =============================================================
# SECTION 4: INTERVENTION LEVEL LOGIC
# =============================================================

def get_intervention_level(ear, frame_count):
    if frame_count >= CLOSED_FRAMES:
        return 3, "LEVEL 3 - DROWSY"
    elif ear < EAR_THRESHOLD:
        return 2, "LEVEL 2 - WARNING"
    elif ear < WARNING_EAR:
        return 2, "LEVEL 2 - WARNING"
    else:
        return 1, "OK"

# =============================================================
# SECTION 5: SERIAL COMMUNICATION
# Updated to send EAR value to Arduino for LCD display
# =============================================================

def send_command(arduino, level, ear=None):
    try:
        if level == 3:
            arduino.write(b'DROWSY\n')
        elif level == 2:
            arduino.write(b'WARNING\n')
        elif ear is not None:
            # Send EAR value so Arduino can display it on LCD
            msg = f'EAR:{ear:.2f}\n'
            arduino.write(msg.encode())
        else:
            arduino.write(b'OK\n')
    except serial.SerialException as e:
        print(f"Serial error: {e}")

# =============================================================
# SECTION 6: DATA LOGGING
# =============================================================

def setup_logger(filepath):
    f = open(filepath, 'w', newline='')
    writer = csv.DictWriter(
        f, fieldnames=['timestamp', 'frame', 'ear', 'level', 'status'])
    writer.writeheader()
    return f, writer

def log_frame(writer, frame_num, ear, level, status):
    writer.writerow({
        'timestamp': datetime.now().strftime('%H:%M:%S.%f')[:-3],
        'frame':     frame_num,
        'ear':       round(ear, 4) if ear is not None else '',
        'level':     level,
        'status':    status
    })

# =============================================================
# SECTION 7: DRAW OVERLAY
# =============================================================

COLOURS = {
    1: (0, 255, 0),    # green
    2: (0, 165, 255),  # orange
    3: (0, 0, 255)     # red
}

def draw_overlay(frame, ear, label, level, w):
    colour = COLOURS[level]

    cv2.putText(frame, f"EAR: {ear:.3f}",
                (30, 50), cv2.FONT_HERSHEY_SIMPLEX, 1.0, colour, 2)
    cv2.putText(frame, f"Status: {label}",
                (30, 100), cv2.FONT_HERSHEY_SIMPLEX, 1.0, colour, 2)
    cv2.putText(frame, datetime.now().strftime('%H:%M:%S'),
                (30, 150), cv2.FONT_HERSHEY_SIMPLEX, 0.7, colour, 2)

    if level >= 2:
        cv2.rectangle(frame, (0, 0), (w, 120), colour, 3)

    return frame

# =============================================================
# SECTION 8: SETUP
# =============================================================

def setup():
    print("Starting AI Driver Monitoring Smart Dashboard...")

    # Load MediaPipe face landmarker model
    base_options = python.BaseOptions(model_asset_path=MODEL_PATH)
    options = vision.FaceLandmarkerOptions(
        base_options=base_options,
        num_faces=1,
        min_face_detection_confidence=0.5
    )
    detector = vision.FaceLandmarker.create_from_options(options)
    print("Face detector loaded.")

    # Open serial connection to Arduino
    try:
        arduino = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2)
        print(f"Arduino connected on {SERIAL_PORT}.")
    except serial.SerialException:
        print("WARNING: Arduino not found. Running without serial.")
        arduino = None

    # Open camera
    cap = cv2.VideoCapture(CAMERA_INDEX)
    if not cap.isOpened():
        raise RuntimeError("Could not open camera.")
    print("Camera opened.")

    return detector, arduino, cap

# =============================================================
# SECTION 9: MAIN LOOP
# =============================================================

def main():
    detector, arduino, cap = setup()
    log_file, writer = setup_logger(LOG_FILE)

    frame_num   = 0
    frame_count = 0
    last_level  = 1

    print("System running. Press Ctrl+C to stop.\n")

    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                print("Camera read failed.")
                break

            frame_num += 1
            h, w = frame.shape[:2]

            rgb      = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb)
            result   = detector.detect(mp_image)

            ear   = None
            level = 1
            label = "OK"

            if result.face_landmarks:
                lm        = result.face_landmarks[0]
                left_ear  = eye_aspect_ratio(lm, LEFT_EYE,  w, h)
                right_ear = eye_aspect_ratio(lm, RIGHT_EYE, w, h)
                ear       = (left_ear + right_ear) / 2.0

                if ear < EAR_THRESHOLD:
                    frame_count += 1
                else:
                    frame_count = 0

                level, label = get_intervention_level(ear, frame_count)

                # Send command to Arduino when level changes
                # Also send EAR value every 30 frames for LCD display
                if level != last_level and arduino:
                    send_command(arduino, level, ear)
                    last_level = level
                elif frame_num % 30 == 0 and arduino and level == 1:
                    send_command(arduino, level, ear)

                frame = draw_overlay(frame, ear, label, level, w)

            log_frame(writer, frame_num, ear, level, label)

            # Show frame (remove when running headless on Pi)
            cv2.imshow('AI Driver Monitor', frame)

            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    except KeyboardInterrupt:
        print("\nStopped by user.")

    finally:
        cap.release()
        cv2.destroyAllWindows()
        log_file.close()
        if arduino:
            arduino.close()
        print(f"Session log saved to {LOG_FILE}")
        print("System shutdown complete.")

# =============================================================
# ENTRY POINT
# =============================================================

if __name__ == '__main__':
    main()
