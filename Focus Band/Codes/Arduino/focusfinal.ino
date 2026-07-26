#include <MPU6050.h>

#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

// --- Variables for motion data ---
float prevAx = 0, prevAy = 0, prevAz = 0;
float motionSmooth = 0;

// --- Pulse sensor setup ---
int pulsePin = A0;
int signal;

// --- Calibrated Sensitivity Levels ---
float motionThreshold = 0.20;     // ⭐ PERFECT threshold based on your readings
float smoothingFactorOld = 0.85;  // ⭐ Strong smoothing
float smoothingFactorNew = 0.15;  // ⭐ Smooth incoming data

// --- Pulse ---
int pulseThreshold = 550;

// --- LEDs ---
int greenLED = 5;
int redLED = 6;

// --- PASSIVE BUZZER ---
int buzzerPin = 9;

// --- Smart distraction timer ---
unsigned long distractedStart = 0;
unsigned long distractionLimit = 3500; // ⭐ 3.5 seconds
bool isDistracted = false;
bool buzzerPatternActive = false;
unsigned long patternTimer = 0;

// --- Heartbeat ---
unsigned long lastBeatTime = 0;
int bpm = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  mpu.initialize();

  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  pinMode(pulsePin, INPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(8, OUTPUT);

  noTone(buzzerPin); // Passive buzzer OFF

  digitalWrite(8, LOW);
  digitalWrite(A1, HIGH);
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);

  Serial.println("Focus Band Calibrated & Running!");
}

void loop() {
  unsigned long currentTime = millis();

  // --- READ MOTION DATA ---
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  float fax = ax / 16384.0;
  float fay = ay / 16384.0;
  float faz = az / 16384.0;

  float motionDiff = abs(fax - prevAx) + abs(fay - prevAy) + abs(faz - prevAz);

  // ⭐ Strong smoothing to avoid small false triggers
  motionSmooth = (motionSmooth * smoothingFactorOld) + (motionDiff * smoothingFactorNew);

  prevAx = fax;
  prevAy = fay;
  prevAz = faz;

  // --- READ PULSE ---
  signal = analogRead(pulsePin);

  if (signal > pulseThreshold && (currentTime - lastBeatTime) > 300) {
    bpm = 60000 / (currentTime - lastBeatTime);
    lastBeatTime = currentTime;
  }

  // --- DISTRACTION CHECK ---
  bool distracted = (motionSmooth > motionThreshold);

  if (distracted) {
    if (!isDistracted) {
      isDistracted = true;
      distractedStart = millis();
    }
  } else {
    isDistracted = false;
    buzzerPatternActive = false;
    noTone(buzzerPin);
  }

  // Activate buzzer pattern after 3.5 sec
  if (isDistracted && !buzzerPatternActive) {
    if (millis() - distractedStart >= distractionLimit) {
      buzzerPatternActive = true;
      patternTimer = millis();
    }
  }

  // --- PASSIVE BUZZER BEEP PATTERN ---
  if (buzzerPatternActive) {
    unsigned long t = millis() - patternTimer;

    if (t < 400) {
      tone(buzzerPin, 3000);  // BEEP 1 (400ms)
    } 
    else if (t < 550) {
      noTone(buzzerPin);      // pause 150ms
    } 
    else if (t < 950) {
      tone(buzzerPin, 3000);  // BEEP 2 (400ms)
    } 
    else if (t < 1200) {
      noTone(buzzerPin);      // pause 250ms
    }
    else {
      patternTimer = millis(); // restart cycle
    }
  }

  // --- LED CONTROL ---
  if (distracted) {
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
  } else {
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);
  }

  // --- JSON OUTPUT FOR PYTHON BRIDGE ---
  Serial.print("{\"ax\":");
  Serial.print(ax);
  Serial.print(",\"ay\":");
  Serial.print(ay);
  Serial.print(",\"az\":");
  Serial.print(az);
  Serial.print(",\"motion\":");
  Serial.print(motionSmooth);
  Serial.println(",\"status\":\"idle\"}");

  delay(200);
}
