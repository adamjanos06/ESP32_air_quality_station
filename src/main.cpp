#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_SGP30.h>

Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;
Adafruit_SGP30 sgp;

// Baseline pressure for relative altitude
float baselinePressure = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(21, 22); // SDA, SCL

  Serial.println("Starting sensors...");

  // AHT20
  if (!aht.begin()) {
    Serial.println("AHT20 not found!");
    while (1);
  }

  // BMP280 (you already confirmed 0x77 works)
  if (!bmp.begin(0x77)) {
    Serial.println("BMP280 not found!");
    while (1);
  }

  // SGP30
  if (!sgp.begin()) {
    Serial.println("SGP30 not found!");
    while (1);
  }

  Serial.println("All sensors ready!");

  // Set baseline pressure (ground reference)
  baselinePressure = bmp.readPressure() / 100.0;

  Serial.print("Baseline pressure: ");
  Serial.print(baselinePressure);
  Serial.println(" hPa");
}

void loop() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  // SGP30 must be measured each loop
  if (!sgp.IAQmeasure()) {
    Serial.println("SGP30 measurement failed");
    delay(1000);
    return;
  }

  // Temperature
  Serial.print("Temp: ");
  Serial.print(temp.temperature);
  Serial.println(" °C");

  // Humidity
  Serial.print("Humidity: ");
  Serial.print(humidity.relative_humidity);
  Serial.println(" %");

  // Pressure
  float pressure = bmp.readPressure() / 100.0;

  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" hPa");

  // ===== Relative Altitude =====
  float relativeAltitude = 44330 * (1.0 - pow(pressure / baselinePressure, 0.1903));

  Serial.print("Altitude: ");
  Serial.print(relativeAltitude);
  Serial.println(" m");

  // ===== eCO2 =====
  Serial.print("eCO2: ");
  Serial.print(sgp.eCO2);
  Serial.print(" ppm - ");

  if (sgp.eCO2 < 400) {
    Serial.println("Fresh outdoor air");
  } else if (sgp.eCO2 <= 800) {
    Serial.println("Good indoor air");
  } else if (sgp.eCO2 <= 1200) {
    Serial.println("Stuffy air");
  } else {
    Serial.println("Poor air quality");
  }

  // ===== TVOC =====
  Serial.print("TVOC: ");
  Serial.print(sgp.TVOC);
  Serial.print(" ppb - ");

  if (sgp.TVOC <= 100) {
    Serial.println("Excellent air");
  } else if (sgp.TVOC <= 300) {
    Serial.println("Normal indoor air");
  } else if (sgp.TVOC <= 1000) {
    Serial.println("Noticeable pollution");
  } else {
    Serial.println("Poor air quality");
  }

  Serial.println("-------------------");

  delay(1000);
}