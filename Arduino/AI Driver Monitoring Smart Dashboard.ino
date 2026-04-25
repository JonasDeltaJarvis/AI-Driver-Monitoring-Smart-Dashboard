// =============================================
// AI Driver Monitoring Smart Dashboard
// Arduino Control Code — with MPU6050 + LCD
// =============================================

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD setup (address 0x27, 16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Motor pins
#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11
#define ENA 5
#define ENB 6

// Alert pins
#define LED    2
#define BUZZER 3

// Alcohol sensor
#define ALCOHOL_PIN       A0
#define ALCOHOL_THRESHOLD 800

// Speed settings
#define NORMAL_SPEED 180

// MPU6050
#define MPU6050_ADDR   0x68
#define JOLT_THRESHOLD 3.0
#define TILT_THRESHOLD 30.0

// Raw MPU6050 values
int16_t ax, ay, az;
int16_t gx, gy, gz;

// Converted values
float accel_x, accel_y, accel_z;
float tilt_x, tilt_y;

void setup() {
  Serial.begin(9600);

  // Motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Alert pins
  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Wake up MPU6050
  Wire.begin();
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);

  // Start LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System starting");
  lcd.setCursor(0, 1);
  lcd.print("Please wait...");
  delay(2000);
  lcd.clear();

  Serial.println("Arduino ready");
  moveForward(NORMAL_SPEED);
  updateLCD("Status: OK", "Monitoring...");
}

void loop() {
  String command = "";

  // Read MPU6050
  readMPU6050();
  detectMotionEvents();

  // Check alcohol sensor
  int alcoholValue = analogRead(ALCOHOL_PIN);
  if (alcoholValue > ALCOHOL_THRESHOLD) {
    Serial.println("ALCOHOL_DETECTED");
    soundAlert();
    updateLCD("!! ALCOHOL !!", "Pull over now");
    parkingStop();
    while (analogRead(ALCOHOL_PIN) > ALCOHOL_THRESHOLD) {
      delay(500);
    }
    silenceAlert();
    updateLCD("Status: OK", "Monitoring...");
    return;
  }

  silenceAlert();

  // Listen for Pi commands
  if (Serial.available()) {
    command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "DROWSY") {
      soundAlert();
      updateLCD("!! DROWSY !!", "Stopping car");
      parkingStop();

    } else if (command.startsWith("EAR:")) {
      // Pi can send EAR value for display
      // Format: "EAR:0.33"
      String earStr = command.substring(4);
      updateLCD("Status: OK", "EAR: " + earStr);
      moveForward(NORMAL_SPEED);

    } else if (command == "WARNING") {
      updateLCD("WARNING", "Eyes closing...");

    } else if (command == "OK") {
      silenceAlert();
      updateLCD("Status: OK", "Monitoring...");
      moveForward(NORMAL_SPEED);
    }
  } else {
    moveForward(NORMAL_SPEED);
  }
}

// ── LCD function ──────────────────────────

void updateLCD(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

// ── MPU6050 functions ─────────────────────

void readMPU6050() {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 14, true);

  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();

  Wire.read(); Wire.read();  // temperature (unused)

  gx = Wire.read() << 8 | Wire.read();
  gy = Wire.read() << 8 | Wire.read();
  gz = Wire.read() << 8 | Wire.read();

  accel_x = ax / 16384.0;
  accel_y = ay / 16384.0;
  accel_z = az / 16384.0;

  tilt_x = gx / 131.0;
  tilt_y = gy / 131.0;
}

void detectMotionEvents() {
  bool alert = false;

  if (abs(accel_x) > JOLT_THRESHOLD ||
      abs(accel_y) > JOLT_THRESHOLD) {
    Serial.println("JOLT_DETECTED");
    updateLCD("!! JOLT !!", "Erratic motion");
    alert = true;
  }

  if (abs(tilt_x) > TILT_THRESHOLD ||
      abs(tilt_y) > TILT_THRESHOLD) {
    Serial.println("TILT_DETECTED");
    updateLCD("!! TILT !!", "Check vehicle");
    alert = true;
  }

  if (alert) {
    tone(BUZZER, 2000);
    digitalWrite(LED, HIGH);
    delay(500);
    noTone(BUZZER);
    digitalWrite(LED, LOW);
    updateLCD("Status: OK", "Monitoring...");
  }
}

// ── Motor functions ───────────────────────

void moveForward(int speed) {
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void parkingStop() {
  int rightSpeed = NORMAL_SPEED;
  int leftSpeed  = NORMAL_SPEED;

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  while (rightSpeed > 0) {
    rightSpeed -= 3;
    leftSpeed  -= 7;
    if (rightSpeed < 0) rightSpeed = 0;
    if (leftSpeed  < 0) leftSpeed  = 0;
    analogWrite(ENA, leftSpeed);
    analogWrite(ENB, rightSpeed);
    delay(80);
  }

  stopMotors();
  Serial.println("Parking stop complete");
}

void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// ── Alert functions ───────────────────────

void soundAlert() {
  tone(BUZZER, 2000);
  digitalWrite(LED, HIGH);
}

void silenceAlert() {
  noTone(BUZZER);
  digitalWrite(LED, LOW);
}
