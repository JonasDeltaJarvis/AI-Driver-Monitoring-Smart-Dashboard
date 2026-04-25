// =============================================
// MPU6050 Motion Sensor — Standalone Test
// AI Driver Monitoring Smart Dashboard
// Merge into main.ino when ready
// =============================================
// Detects two things:
// 1. Sudden jolts (accelerometer)
// 2. Abnormal tilt (gyroscope)
// Both can indicate erratic/impaired driving
// =============================================

#include <Wire.h>

// MPU6050 I2C address
#define MPU6050_ADDR 0x68

// Thresholds — tune these after real-world testing
#define JOLT_THRESHOLD  3.0   // g-force — sudden jolt detection
#define TILT_THRESHOLD  30.0  // degrees — abnormal tilt detection

// Alert pin (same as main.ino)
#define BUZZER 3
#define LED    2

// Raw sensor values
int16_t ax, ay, az;  // accelerometer
int16_t gx, gy, gz;  // gyroscope

// Converted values
float accel_x, accel_y, accel_z;
float tilt_x, tilt_y;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Wake up MPU6050 (it starts in sleep mode)
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x6B);  // power management register
  Wire.write(0x00);  // write 0 to wake it up
  Wire.endTransmission(true);

  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);

  Serial.println("MPU6050 ready");
}

void loop() {
  readMPU6050();
  detectMotionEvents();
  printData();
  delay(100);  // read 10 times per second
}

// ── Read raw data from MPU6050 ─────────────

void readMPU6050() {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x3B);  // starting register for accelerometer data
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 14, true);  // request 14 bytes

  // Accelerometer (X, Y, Z)
  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();

  // Temperature (we read but don't use it)
  Wire.read(); Wire.read();

  // Gyroscope (X, Y, Z)
  gx = Wire.read() << 8 | Wire.read();
  gy = Wire.read() << 8 | Wire.read();
  gz = Wire.read() << 8 | Wire.read();

  // Convert raw values to meaningful units
  // 16384 = sensitivity at ±2g range
  accel_x = ax / 16384.0;
  accel_y = ay / 16384.0;
  accel_z = az / 16384.0;

  // Convert gyroscope to degrees per second
  // 131 = sensitivity at ±250°/s range
  tilt_x = gx / 131.0;
  tilt_y = gy / 131.0;
}

// ── Detect dangerous motion events ─────────

void detectMotionEvents() {
  bool alert = false;

  // Check for sudden jolt (e.g. swerving, collision)
  if (abs(accel_x) > JOLT_THRESHOLD ||
      abs(accel_y) > JOLT_THRESHOLD) {
    Serial.println("JOLT_DETECTED");
    alert = true;
  }

  // Check for abnormal tilt (e.g. vehicle rolling, road edge)
  if (abs(tilt_x) > TILT_THRESHOLD ||
      abs(tilt_y) > TILT_THRESHOLD) {
    Serial.println("TILT_DETECTED");
    alert = true;
  }

  if (alert) {
    digitalWrite(BUZZER, HIGH);
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(BUZZER, LOW);
    digitalWrite(LED, LOW);
  }
}

// ── Print data to Serial Monitor ───────────

void printData() {
  Serial.print("Accel (g): ");
  Serial.print(accel_x); Serial.print(", ");
  Serial.print(accel_y); Serial.print(", ");
  Serial.println(accel_z);

  Serial.print("Gyro (deg/s): ");
  Serial.print(tilt_x); Serial.print(", ");
  Serial.print(tilt_y); Serial.print(", ");
  Serial.println(gz / 131.0);
}