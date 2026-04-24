// AI Driver Monitoring Smart Dashboard
// Arduino Control Code

#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11
#define ENA 5
#define ENB 6
#define LED 2
#define BUZZER 3
#define ALCOHOL_PIN A0
#define ALCOHOL_THRESHOLD 800
#define NORMAL_SPEED 180

void setup() {
  Serial.begin(9600);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  Serial.println("Arduino ready");
  moveForward(NORMAL_SPEED);
}

void loop() {
  String command = "";
  int alcoholValue = analogRead(ALCOHOL_PIN);

  if (alcoholValue > ALCOHOL_THRESHOLD) {
    Serial.println("ALCOHOL_DETECTED");
    soundAlert();
    parkingStop();
    // Stay stopped — wait here until potentiometer drops
    while (analogRead(ALCOHOL_PIN) > ALCOHOL_THRESHOLD) {
      delay(500);
    }
    silenceAlert();
    return;
  }

  silenceAlert();

  if (Serial.available()) {
    command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "DROWSY") {
      soundAlert();
      parkingStop();
    } else if (command == "OK") {
      silenceAlert();
      moveForward(NORMAL_SPEED);
    }
  } else {
    moveForward(NORMAL_SPEED);
  }
}
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
  int leftSpeed = NORMAL_SPEED;

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  while (rightSpeed > 0) {
    rightSpeed -= 3;
    leftSpeed -= 7;
    if (rightSpeed < 0) rightSpeed = 0;
    if (leftSpeed < 0) leftSpeed = 0;
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

void soundAlert() {
  tone(BUZZER, 2000);  // 2000Hz = higher pitched beep
  digitalWrite(LED, HIGH);
}

void silenceAlert() {
  noTone(BUZZER);
  digitalWrite(LED, LOW);
}