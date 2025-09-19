// Define the analog pin
const int sensorPin = A0;

// --- CALIBRATION VALUES
// This is the analog reading when the sensor is fully dry (in air or dry soil)
const int dryValue = 1023; 

// This is the analog reading when the sensor is fully wet (in water)
const int wetValue = 210; 

// The desired output range for the percentage: 0% is dry, 100% is wet
const int minPercent = 0; 
const int maxPercent = 100;

void setup() {
  Serial.begin(9600);
  Serial.println("Soil Moisture Sensor Reading");
}

void loop() {
  // 1. Read the raw analog value
  int rawValue = analogRead(sensorPin);
  
  // 2. Map the raw value to a percentage
  // We use the dryValue (900) as the lower limit (0%) and wetValue (300) as the upper limit (100%)
  // to correctly invert the reading (Higher analog value -> Lower moisture percentage)
  int moisturePercent = map(rawValue, dryValue, wetValue, minPercent, maxPercent);
  
  // 3. Constrain the value to ensure it stays between 0 and 100
  // Readings outside your calibrated range will be capped at 0 or 100.
  moisturePercent = constrain(moisturePercent, minPercent, maxPercent);

  // 4. Print the results
  Serial.print("Raw: ");
  Serial.print(rawValue);
  Serial.print(" \tMoisture: ");
  Serial.print(moisturePercent);
  Serial.println("%");

  delay(1000); 
}