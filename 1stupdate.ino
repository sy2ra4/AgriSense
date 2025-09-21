#include "DHT.h"

#define DHTPIN 2 

#define DHTTYPE DHT22   

DHT dht(DHTPIN, DHTTYPE);

const int sensorPin = A0;


const int dryValue = 1023; 

const int wetValue = 210; 

const int minPercent = 0; 
const int maxPercent = 100;

void setup() {
  Serial.begin(9600);
  Serial.println(F("DHT22 & Soil Moisture Sensor Reading!"));


  dht.begin();

}

void loop() {
  delay(2000);

  
  // Read humidity
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Celsius (or Fahrenheit)
  float hic = dht.computeHeatIndex(t, h, false);
  float hif = dht.computeHeatIndex(f, h);

  int rawValue = analogRead(sensorPin);
  
  int moisturePercent = map(rawValue, dryValue, wetValue, minPercent, maxPercent);
  
  moisturePercent = constrain(moisturePercent, minPercent, maxPercent);

  

  // Print the results to the Serial Monitor
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat Index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  Serial.print("Raw: ");
  Serial.print(rawValue);
  Serial.print(" \tMoisture: ");
  Serial.print(moisturePercent);
  Serial.println("%");

}
