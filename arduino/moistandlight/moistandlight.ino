#include <Wire.h>
#include "Adafruit_VEML7700.h"  

#define MOISTURE_SENSOR_PIN 34  // Use a proper ADC pin on ESP32

Adafruit_VEML7700 veml = Adafruit_VEML7700();  

void setup() {
    Serial.begin(9600);
    
    // Start I2C with correct ESP32 pins (21: SDA, 22: SCL)
    Wire.begin(21, 22);

    // Initialize ADC resolution (ESP32 default is 12-bit, but better to set explicitly)
    analogReadResolution(12);  

    // Initialize VEML7700 light sensor
    delay(100);  // Small delay for stability
    if (!veml.begin()) {
        Serial.println("VEML7700 not found! Check wiring.");
        while (1);  // Stop execution if sensor is missing
    }
    
    veml.setGain(VEML7700_GAIN_1);  
    veml.setIntegrationTime(VEML7700_IT_100MS);  
    pinMode(MOISTURE_SENSOR_PIN, INPUT);
}

void loop() {
    int moistureValue = analogRead(MOISTURE_SENSOR_PIN);
    int moisturePercent = map(moistureValue, 0, 4095, 100, 0);  // Convert to 0-100%

    float lux = veml.readLux();

    Serial.print("Moisture: ");
    Serial.print(moisturePercent);
    Serial.print("% | Light: ");
    Serial.print(lux);
    Serial.println(" lux");

    delay(1000);
}
