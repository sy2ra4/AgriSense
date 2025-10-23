#include "DHT.h"
#include <Servo.h>

// --- Configuration Constants ---
#define DHTPIN 2       // DHT data pin
#define DHTTYPE DHT22  // DHT sensor type

// Soil Moisture Pins
const int MOISTURE_PIN_Z1 = A0; // Zone 1 Sensor
const int MOISTURE_PIN_Z2 = A3; // Zone 2 Sensor
const int MOISTURE_PIN_Z3 = A5; // Zone 3 Sensor (Main Line/Area)

// Servo Pins
const int SERVO_PIN_Z1 = 9;  // Zone 1 Gate (Servo 1)
const int SERVO_PIN_Z2 = 10; // Zone 2 Gate (Servo 2)

// Relay Pin (Controls Water Pump)
const int PUMP_RELAY_PIN = 4;

// Calibration Values (Adjust based on your sensors)
const int DRY_VALUE = 800;  // Analog value for bone dry soil (high value)
const int WET_VALUE = 300;  // Analog value for fully wet soil (low value)

// Irrigation Thresholds
const int IRRIGATION_THRESHOLD_PERCENT = 30; // Start watering if moisture < 40%

// Servo Angles (Adjust based on your physical gate mechanism)
// Servo 1 (Zone 1) opens by moving from 0 to 90 degrees
const int Z1_GATE_OPEN = 90;
const int Z1_GATE_CLOSED = 180;
// Servo 2 (Zone 2) opens by moving from 180 to 90 degrees
const int Z2_GATE_OPEN = 90;
const int Z2_GATE_CLOSED = 0;

// --- Global Variables and Objects ---
DHT dht(DHTPIN, DHTTYPE);
Servo servoZ1;
Servo servoZ2;

// --- Helper Function ---
int getMoisturePercent(int rawValue) {
  // Map the raw value to a percentage (0-100)
  int moisturePercent = map(rawValue, DRY_VALUE, WET_VALUE, 0, 100);
  // Constrain the result to 0-100%
  return constrain(moisturePercent, 0, 100);
}

// --- Setup ---
void setup() {
  Serial.begin(9600);
  Serial.println(F("Smart Irrigation Manifold Starting..."));

  // Initialize DHT sensor
  dht.begin();

  // Initialize Servos
  servoZ1.attach(SERVO_PIN_Z1);
  servoZ2.attach(SERVO_PIN_Z2);

  // Ensure gates are initially closed
  servoZ1.write(Z1_GATE_CLOSED);
  servoZ2.write(Z2_GATE_CLOSED);

  // Initialize Relay pin for Pump control
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  digitalWrite(PUMP_RELAY_PIN, HIGH); // Assuming a common active-LOW relay, HIGH = OFF
}

// --- Main Loop ---
void loop() {
  delay(2000); // Wait between sensor readings

  // --- 1. Read Environmental Sensors (DHT22) ---
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("❌ DHT Read Failed!"));
  } else {
    Serial.print(F("🌡️ Temp: "));
    Serial.print(t);
    Serial.print(F("°C | 💧 Humid: "));
    Serial.print(h);
    Serial.println(F("%"));
  }

  // --- 2. Read Soil Moisture Sensors ---
  int rawZ1 = analogRead(MOISTURE_PIN_Z1);
  int percentZ1 = getMoisturePercent(rawZ1);
  bool needsWaterZ1 = (percentZ1 < IRRIGATION_THRESHOLD_PERCENT);

  int rawZ2 = analogRead(MOISTURE_PIN_Z2);
  int percentZ2 = getMoisturePercent(rawZ2);
  bool needsWaterZ2 = (percentZ2 < IRRIGATION_THRESHOLD_PERCENT);

  int rawZ3 = analogRead(MOISTURE_PIN_Z3);
  int percentZ3 = getMoisturePercent(rawZ3);
  bool needsWaterZ3 = (percentZ3 < IRRIGATION_THRESHOLD_PERCENT); // Zone 3 moisture check
  
  Serial.println(F("--- Soil Moisture Status ---"));
  Serial.print(F("Zone 1: ")); Serial.print(percentZ1); Serial.print(F("%")); 
  if (needsWaterZ1) Serial.print(F(" ⚠️ DRY"));
  Serial.println();
  
  Serial.print(F("Zone 2: ")); Serial.print(percentZ2); Serial.print(F("%")); 
  if (needsWaterZ2) Serial.print(F(" ⚠️ DRY"));
  Serial.println();

  Serial.print(F("Zone 3: ")); Serial.print(percentZ3); Serial.print(F("%")); 
  if (needsWaterZ3) Serial.print(F(" ⚠️ DRY (Main Area)"));
  Serial.println();


  // --- 3. Determine Irrigation Need and Control Pump & Gates ---
  // The pump turns ON if ANY zone (including the main area Z3) needs water.
  bool anyZoneNeedsWater = needsWaterZ1 || needsWaterZ2 || needsWaterZ3;

  if (anyZoneNeedsWater) {
    
    // 3a. Control Zone Gates (Servos) - Multi-Zone Manifold Logic
    
    // Zone 1 Gate Control
    if (needsWaterZ1 && !needsWaterZ2 && !needsWaterZ3) {
      servoZ1.write(Z1_GATE_OPEN);
      Serial.println(F("🚪 Zone 1 Gate OPENED (Moisture Low)."));
    }

    // Zone 2 Gate Control
    if (needsWaterZ2 && !needsWaterZ1 && !needsWaterZ3) {
      servoZ2.write(Z2_GATE_OPEN);
      Serial.println(F("🚪 Zone 2 Gate OPENED (Moisture Low)."));
    }

    /*
    * Logic for Zone 3 Irrigation:
    * Zone 3 is the main area/line. If it needs water, but Z1 and Z2 don't, 
    * we open BOTH gates (Z1 and Z2) to ensure water is distributed widely. 
    * This achieves a "Zone 3 ON" mode using the two existing servos.
    */
    if (needsWaterZ3 && !needsWaterZ1 && !needsWaterZ2) {
        servoZ1.write(Z1_GATE_CLOSED);
        delay(1000);
        servoZ2.write(Z2_GATE_CLOSED);
        Serial.println(F("🚪 Zone 3 Gate OPENED (Moisture Low)."));
    }

    if (needsWaterZ2 && needsWaterZ1 && !needsWaterZ3) {
      servoZ1.write(0);
      delay(1000);
      servoZ2.write(Z2_GATE_OPEN);
      Serial.println(F("🚪 Zone 1 & 2 Gate OPENED (Moisture Low)."));
    }

    if (!needsWaterZ2 && needsWaterZ1 && needsWaterZ3) {
      servoZ1.write(0);
      delay(1000);
      servoZ2.write(Z2_GATE_CLOSED);
      Serial.println(F("🚪 Zone 1 & 3 Gate OPENED (Moisture Low)."));
    }

    if (needsWaterZ2 && !needsWaterZ1 && needsWaterZ3) {
      servoZ1.write(Z1_GATE_CLOSED);
      delay(1000);
      servoZ2.write(180);
      Serial.println(F("🚪 Zone 2 & 3 Gate OPENED (Moisture Low)."));
    }

    if (needsWaterZ2 && needsWaterZ1 && needsWaterZ3) {
      servoZ1.write(0);
      delay(1000);
      servoZ2.write(180);
      Serial.println(F("🚪 Zone 1, 2 & 3 Gate OPENED (Moisture Low)."));
    }

    // 3b. Turn ON the Water Pump
    digitalWrite(PUMP_RELAY_PIN, LOW); // Active-LOW relay
    Serial.println(F("✅ PUMP ON: One or more areas require irrigation."));

    // Hold watering for a brief period before checking again (allows soil to absorb water)
    delay(1500); 

  } else {
    // 3c. Turn OFF the Water Pump
    digitalWrite(PUMP_RELAY_PIN, HIGH); // Active-LOW relay
    
    // 3d. Ensure all gates are closed when the pump is OFF
    servoZ1.write(Z1_GATE_CLOSED);
    servoZ2.write(Z2_GATE_CLOSED);

    Serial.println(F("🛑 PUMP OFF: All zones are adequately moist."));
  }

  Serial.println(F("-----------------------------------"));
}
