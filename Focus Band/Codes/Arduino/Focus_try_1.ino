#include <MPU6050.h>

#include <Wire.h>
#include <MPU6050.h>   // Library to use the motion sensor

MPU6050 mpu;            // Create an object for the MPU6050 sensor

// --- Variables for motion data ---
float prevAx = 0, prevAy = 0, prevAz = 0;  // Store previous acceleration values
float motionSmooth = 0;                    // Filtered/smoothed motion value

// --- Pulse sensor setup ---
int pulsePin = A0;        // Signal pin connected to A0
int signal;               // To store current pulse sensor value

// --- Adjustable sensitivity levels ---
float motionThreshold = 0.32;   // Higher = less sensitive to movement
int pulseThreshold = 550;       // Higher = less sensitive to heartbeats

// --- LEDs ---
int greenLED = 5;   // Green LED → focused
int redLED = 6;     // Red LED → distracted

// --- Heartbeat calculation helpers ---
unsigned long lastBeatTime = 0;
int bpm = 0;

void setup() {
  Serial.begin(9600);     // For debugging and data display
  Wire.begin();           // Start I2C communication for MPU6050
  mpu.initialize();       // Initialize the motion sensor

  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(pulsePin, INPUT); 
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(8,OUTPUT);
  pinMode(9,OUTPUT);

  digitalWrite(8,LOW);
  digitalWrite(9, HIGH);
  digitalWrite(A1, HIGH);
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
  
  Serial.println("Focus Tracker Started!");
}

void loop() {
  unsigned long currentTime = millis();  // Track time in milliseconds

  // --- 1️⃣ READ MOTION DATA ---
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);  // Get acceleration values from sensor

  // Convert raw data into "g" units
  float fax = ax / 16384.0;
  float fay = ay / 16384.0;
  float faz = az / 16384.0;

  // Calculate total movement difference from previous reading
  float motionDiff = abs(fax - prevAx) + abs(fay - prevAy) + abs(faz - prevAz);

  // Apply a smoothing filter to ignore tiny jitters
  motionSmooth = motionSmooth * 0.8 + motionDiff * 0.2;

  // Store current readings for next loop
  prevAx = fax;
  prevAy = fay;
  prevAz = faz;

  // --- 2️⃣ READ PULSE SENSOR DATA ---
  signal = analogRead(pulsePin);  // Get analog value (0–1023)

  // Detect a heartbeat when signal crosses threshold
  if (signal > pulseThreshold && (currentTime - lastBeatTime) > 300) {
    bpm = 60000 / (currentTime - lastBeatTime);  // Calculate BPM
    lastBeatTime = currentTime;                  // Save beat time
  }

  // --- 3️⃣ DECIDE IF USER IS DISTRACTED ---
  bool distracted = false;

  // If movement too high → distracted
  if (motionSmooth > motionThreshold) distracted = true;
  if (motionSmooth < motionThreshold) distracted = false;
  // If heartbeat too high or too low → stressed or tired
  if (bpm > 110 || bpm < 50) distracted = true;

  // --- 4️⃣ CONTROL LED OUTPUT ---
  if (distracted) {
    digitalWrite(redLED, HIGH);     // Red ON (distracted)
    digitalWrite(greenLED, LOW); 
    //Serial.print("Focus Bro");
    //Serial.println(motionSmooth);
      // Green OFF
  } else {
    digitalWrite(redLED, LOW);      // Red OFF
    digitalWrite(greenLED, HIGH);  
    //Serial.print("Good Job"); // Green ON (focused)
  }

  // --- 5️⃣ OPTIONAL: DISPLAY DATA ON SERIAL MONITOR ---
 // Serial.print("BPM: ");
  //Serial.print(bpm);
  //Serial.print("Motion:");
  //Serial.println(motionSmooth);
  Serial.print(bpm);
  Serial.print(",");
  Serial.println(motionSmooth);


  delay(100); // Short delay for stable readings
}