#include <Wire.h>
#include "Adafruit_VEML7700.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// BLE service and characteristics UUIDs
BLECharacteristic moistureCharacteristic(BLEUUID((uint16_t)0x2A37), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
BLECharacteristic lightCharacteristic(BLEUUID((uint16_t)0x2A7A), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

#define MOISTURE_SENSOR_PIN A0  // Analog pin for soil moisture sensor

Adafruit_VEML7700 veml = Adafruit_VEML7700();  // Create sensor object

void setup() {
    Serial.begin(9600);
    Wire.begin(A4, A5);  // Start I2C (SDA: 21, SCL: 22)
    BLEDevice::init("ESP32_Sensor");  // Initialize BLE device

    // Initialize VEML7700 light sensor
    if (!veml.begin()) {
        Serial.println("VEML7700 not found! Check wiring.");
        while (1);
    }

    // Start BLE server
    BLEServer *pServer = BLEDevice::createServer();
    pServer->getAdvertising()->start();  // Start advertising

    // Create the custom service using createService
    BLEService *sensorService = pServer->createService(BLEUUID((uint16_t)0x1812));  // Custom service UUID
    // Add the characteristics to the service
    sensorService->addCharacteristic(&moistureCharacteristic);
    sensorService->addCharacteristic(&lightCharacteristic);
    // Start the service
    sensorService->start();
    
    veml.setGain(VEML7700_GAIN_1);  // Set gain (default is 1)
    veml.setIntegrationTime(VEML7700_IT_100MS);  // Set integration time
    pinMode(MOISTURE_SENSOR_PIN, INPUT);
}

void loop() {
    // Read moisture sensor value
    // the sensor value description   # 0  ~300     dry soil   # 300~700     humid soil   # 700~950     in water 
    int moistureValue = analogRead(MOISTURE_SENSOR_PIN);
    //int moisturePercent = map(moistureValue, 0, 950, 0, 100); // Convert to 0-100% scale

    // Read ambient light (lux)
    float lux = veml.readLux();

    // Print results
    Serial.print("Moisture: ");
    Serial.print(moistureValue);
    Serial.print("% | Light: ");
    Serial.print(lux);
    Serial.println(" lux");

    delay(1000);  // Delay 1 second
}
