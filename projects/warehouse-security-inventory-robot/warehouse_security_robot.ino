#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// Warehouse Security Robot - Arduino Uno
// Status: source available; hardware behaviour still requires testing.

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo scanServo;

const byte SERVO_PIN = 9;
const int SERVO_CENTER = 90;
const int SCAN_ANGLES[] = {30, 60, 90, 120, 150};
const byte NUM_SCAN = sizeof(SCAN_ANGLES) / sizeof(SCAN_ANGLES[0]);

const byte TRIG_PIN = 2;
const byte ECHO_PIN = 3;
const byte PIR_PIN = 4;

const byte IN1 = A0;
const byte IN2 = A1;
const byte IN3 = 7;
const byte IN4 = 8;
const byte ENA = 5;
const byte ENB = 6;

const byte BUZZER_PIN = 11;

const float OBSTACLE_DISTANCE_CM = 20.0;
const int LEFT_SPEED = 170;
const int RIGHT_SPEED = 170;

const unsigned long DRIVE_STEP_MS = 250;
const unsigned long TURN_90_MS = 450;
const unsigned long REVERSE_MS = 350;
const unsigned long SERVO_SETTLE_MS = 300;
const unsigned long COOLDOWN_MS = 2000;
const unsigned long BEEP_INTERVAL_MS = 350;
const unsigned long ECHO_TIMEOUT_US = 25000UL;

const byte NUM_DISTANCE_READS = 3;
const byte MAX_CONSECUTIVE_BAD_READS = 2;

unsigned long lastMotionTime = 0;
unsigned long lastBeepTime = 0;
bool previousMotion = false;
int currentServoAngle = -1;
byte consecutiveBadReadings = 0;

void printPadded(const char *message) {
  byte i = 0;
  while (message[i] != '\0' && i < 16) {
    lcd.print(message[i++]);
  }
  while (i < 16) {
    lcd.print(' ');
    i++;
  }
}

void showMessage(const char *line1, const char *line2) {
  lcd.setCursor(0, 0);
  printPadded(line1);
  lcd.setCursor(0, 1);
  printPadded(line2);
}

void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void setMotorDirections(bool leftForward, bool rightForward) {
  digitalWrite(IN1, leftForward ? HIGH : LOW);
  digitalWrite(IN2, leftForward ? LOW : HIGH);
  digitalWrite(IN3, rightForward ? HIGH : LOW);
  digitalWrite(IN4, rightForward ? LOW : HIGH);
}

bool safeWait(unsigned long durationMs) {
  const unsigned long started = millis();
  while (millis() - started < durationMs) {
    if (digitalRead(PIR_PIN) == HIGH) {
      return false;
    }
    delay(5);
  }
  return true;
}

bool runMotors(bool leftForward, bool rightForward, unsigned long durationMs) {
  setMotorDirections(leftForward, rightForward);
  analogWrite(ENA, LEFT_SPEED);
  analogWrite(ENB, RIGHT_SPEED);

  const bool completed = safeWait(durationMs);
  stopMotors();
  return completed;
}

bool motionDetected() {
  return digitalRead(PIR_PIN) == HIGH;
}

void handleMotionAlert() {
  stopMotors();
  lastMotionTime = millis();
  previousMotion = true;
  showMessage("SECURITY ALERT", "MOTION DETECTED");

  if (millis() - lastBeepTime >= BEEP_INTERVAL_MS) {
    tone(BUZZER_PIN, 2000, 220);
    lastBeepTime = millis();
  }
}

bool pointSensor(int angle) {
  if (angle == currentServoAngle) {
    return true;
  }

  scanServo.write(angle);
  currentServoAngle = angle;
  return safeWait(SERVO_SETTLE_MS);
}

float readOneDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  const unsigned long duration = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  if (duration == 0) {
    return -1.0;
  }

  const float distance = (duration * 0.0343f) / 2.0f;
  if (distance < 2.0 || distance > 400.0) {
    return -1.0;
  }
  return distance;
}

float readDistanceCm() {
  float values[NUM_DISTANCE_READS];
  byte count = 0;

  for (byte i = 0; i < NUM_DISTANCE_READS; i++) {
    const float distance = readOneDistanceCm();
    if (distance >= 2.0 && distance <= 400.0) {
      values[count++] = distance;
    }
    delay(35);
  }

  if (count == 0) {
    return -1.0;
  }

  for (byte i = 0; i < count - 1; i++) {
    for (byte j = i + 1; j < count; j++) {
      if (values[j] < values[i]) {
        const float temporary = values[i];
        values[i] = values[j];
        values[j] = temporary;
      }
    }
  }

  return values[count / 2];
}

bool scanForOpenWay(int &bestAngle, float &bestDistance) {
  bestAngle = SERVO_CENTER;
  bestDistance = -1.0;

  for (byte i = 0; i < NUM_SCAN; i++) {
    if (!pointSensor(SCAN_ANGLES[i])) {
      return false;
    }

    const float distance = readDistanceCm();

    Serial.print(F("Scan "));
    Serial.print(SCAN_ANGLES[i]);
    Serial.print(F(" deg: "));
    if (distance < 0) {
      Serial.println(F("NO ECHO"));
    } else {
      Serial.print(distance);
      Serial.println(F(" cm"));
    }

    if (distance >= 0 && distance > bestDistance) {
      bestDistance = distance;
      bestAngle = SCAN_ANGLES[i];
    }
  }

  return true;
}

bool turnTowards(int angle) {
  const int offset = angle - SERVO_CENTER;
  const unsigned long turnMs =
      (TURN_90_MS * (unsigned long)abs(offset)) / 90UL;

  if (turnMs == 0) {
    return true;
  }

  if (offset < 0) {
    return runMotors(true, false, turnMs);
  }
  return runMotors(false, true, turnMs);
}

void escapeFromTrap() {
  showMessage("NO CLEAR PATH", "Reversing...");
  if (!runMotors(false, false, REVERSE_MS)) {
    return;
  }
  runMotors(true, false, TURN_90_MS * 2UL);
}

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(TRIG_PIN, LOW);
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  stopMotors();
  noTone(BUZZER_PIN);

  scanServo.attach(SERVO_PIN);
  scanServo.write(SERVO_CENTER);
  currentServoAngle = SERVO_CENTER;

  showMessage("WAREHOUSE ROBOT", "Starting...");
  delay(1000);

  showMessage("PIR WARMING UP", "Wait 30 seconds");
  const unsigned long warmupStart = millis();
  while (millis() - warmupStart < 30000UL) {
    stopMotors();
    delay(50);
  }

  showMessage("AUTONOMOUS MODE", "Patrol starting");
  delay(1000);
}

void loop() {
  if (motionDetected()) {
    handleMotionAlert();
    return;
  }
  noTone(BUZZER_PIN);

  if (previousMotion) {
    if (millis() - lastMotionTime < COOLDOWN_MS) {
      stopMotors();
      showMessage("MOTION CLEARED", "Waiting...");
      return;
    }
    previousMotion = false;
  }

  if (!pointSensor(SERVO_CENTER)) {
    return;
  }

  const float ahead = readDistanceCm();

  if (ahead < 0) {
    consecutiveBadReadings++;
    stopMotors();
    Serial.println(F("HC-SR04: NO VALID ECHO"));

    if (consecutiveBadReadings >= MAX_CONSECUTIVE_BAD_READS) {
      showMessage("SENSOR NO ECHO", "Check HC-SR04");
    } else {
      showMessage("CHECKING SENSOR", "No echo; stopped");
    }
    delay(150);
    return;
  }

  consecutiveBadReadings = 0;
  Serial.print(F("Front distance: "));
  Serial.print(ahead);
  Serial.println(F(" cm"));

  if (ahead > OBSTACLE_DISTANCE_CM) {
    char line2[17];
    snprintf(line2, sizeof(line2), "Distance: %d cm", (int)(ahead + 0.5));
    showMessage("PATH CLEAR", line2);
    runMotors(true, true, DRIVE_STEP_MS);
    return;
  }

  stopMotors();
  showMessage("OBSTACLE AHEAD", "Scanning sides...");

  int bestAngle;
  float bestDistance;
  if (!scanForOpenWay(bestAngle, bestDistance)) {
    return;
  }

  if (bestDistance < 0) {
    showMessage("SCAN FAILED", "Check sensor");
    stopMotors();
    delay(250);
    return;
  }

  if (bestDistance <= OBSTACLE_DISTANCE_CM) {
    escapeFromTrap();
    return;
  }

  showMessage("CLEAR PATH FOUND", "Turning...");
  turnTowards(bestAngle);
}
