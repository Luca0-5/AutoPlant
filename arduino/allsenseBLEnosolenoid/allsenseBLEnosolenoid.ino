#include <Wire.h>

#include <Adafruit_Sensor.h>

#include <Adafruit_BME280.h>

#include <ArduinoBLE.h>

 

// Sensor and Solenoid Pins

#define MOISTURE_SENSOR_PIN A0  // Soil moisture sensor

#define SOLENOID_PIN 4          // Solenoid control pin

#define SEALEVELPRESSURE_HPA (1013.25)

 

// BLE Service

BLEService sensorService("180C");

 

// Separate BLE Characteristics for Each Sensor

BLEStringCharacteristic moistureChar("2A58", BLERead | BLENotify, 10);

BLEStringCharacteristic tempChar("2A59", BLERead | BLENotify, 10);

BLEStringCharacteristic humidityChar("2A5A", BLERead | BLENotify, 10);

BLEStringCharacteristic pressureChar("2A5B", BLERead | BLENotify, 10);

BLEStringCharacteristic lightChar("2A5C", BLERead | BLENotify, 10);

 

// BLE Characteristic for Solenoid Control

BLEStringCharacteristic solenoidControlChar("2A57", BLEWrite, 10);

 

// BME280 Sensor Object

Adafruit_BME280 bme;

bool bme280_initialized = false;

 

void setup() {

    Serial.begin(115200);

    Wire.begin();

    pinMode(MOISTURE_SENSOR_PIN, INPUT);

    pinMode(SOLENOID_PIN, OUTPUT);

    digitalWrite(SOLENOID_PIN, LOW);  // Ensure solenoid is off at startup

 

    // Initialize BME280

    bme280_initialized = bme.begin(0x77);

    if (!bme280_initialized) {

        Serial.println("BME280 not found! Check wiring.");

    }

 

    setSensorActiveModeAndEnableALS();

 

    // Start BLE

    if (!BLE.begin()) {

        Serial.println("BLE initialization failed!");

        while (1);

    }

 

    BLE.setLocalName("ESP32_Sensor");

    BLE.setAdvertisedService(sensorService);

 

    // Add Characteristics to BLE Service

    sensorService.addCharacteristic(moistureChar);

    sensorService.addCharacteristic(tempChar);

    sensorService.addCharacteristic(humidityChar);

    sensorService.addCharacteristic(pressureChar);

    sensorService.addCharacteristic(lightChar);

    sensorService.addCharacteristic(solenoidControlChar);

 

    // Set Callback for Solenoid Control

    solenoidControlChar.setEventHandler(BLEWritten, onSolenoidCommand);

 

    // Add Service and Start Advertising

    BLE.addService(sensorService);

    BLE.advertise();

    Serial.println("BLE Device Ready. Connect to ESP32_Sensor.");

}

 

void loop() {

    BLE.poll();

 

    // Read sensor values

    int moistureValue = analogRead(MOISTURE_SENSOR_PIN);

    int moisturePercent = map(moistureValue, 0, 2714, 0, 100);

 

    float tempC = bme.readTemperature();

    float humidity = bme.readHumidity();

    float pressure = bme.readPressure() / 100.0F;

 

    uint16_t greenLight = readAmbientLightData(0x0D);

    uint16_t blueLight = readAmbientLightData(0x10);

    uint16_t redLight = readAmbientLightData(0x13);

    uint16_t lightIntensity = calculateLightIntensity(greenLight, blueLight, redLight);

 

    // Send each sensor value separately

    moistureChar.writeValue(String(moisturePercent));

    tempChar.writeValue(String(tempC));

    humidityChar.writeValue(String(humidity));

    pressureChar.writeValue(String(pressure));

    lightChar.writeValue(String(lightIntensity));

    Serial.print("Moisture: "); Serial.print(moisturePercent); Serial.println("%");
    Serial.print("Temperature: "); Serial.print(tempC); Serial.println("°C");
    Serial.print("Humidity: "); Serial.print(humidity); Serial.println("%");
    Serial.print("Pressure: "); Serial.print(pressure); Serial.println(" hPa");
    Serial.print("Light Intensity: "); Serial.print(lightIntensity); Serial.println(" lx");

    checkUserInput();

    delay(1000);
}

 

// BLE Callback: Handle Incoming Commands for Solenoid

void onSolenoidCommand(BLEDevice central, BLECharacteristic characteristic) {

    String command = String((char*)characteristic.value()); // Convert BLE value to String

    command.trim();

 

    if (command == "open") {

        Serial.println("Solenoid ON for 5 seconds.");

        digitalWrite(SOLENOID_PIN, HIGH);

        delay(5000);

        digitalWrite(SOLENOID_PIN, LOW);

        Serial.println("Solenoid OFF.");

    } else {

        Serial.println("Invalid command.");

    }

}

 

// Function to Enable Ambient Light Sensor

void setSensorActiveModeAndEnableALS() {

    Wire.beginTransmission(0x52);

    Wire.write(0x00); 

    Wire.write(0x07);

    Wire.endTransmission();

    delay(100);

}

 

// Read Ambient Light Sensor Data

uint16_t readAmbientLightData(uint8_t registerAddress) {

    Wire.beginTransmission(0x52);

    Wire.write(registerAddress);

    Wire.endTransmission(false);

    Wire.requestFrom((uint8_t)0x52, (uint8_t)2);

   

    uint16_t lightValue = 0;

    if (Wire.available() == 2) {

        uint8_t lsb = Wire.read();

        uint8_t msb = Wire.read();

        lightValue = (msb << 8) | lsb;

    }

    return lightValue;

}

void checkUserInput() {
    if (Serial.available() > 0) {  // Check if user input is available
        String command = Serial.readStringUntil('\n'); // Read user input
        command.trim(); // Remove whitespace and newline characters

        if (command == "open") { // If user types "open"
            Serial.println("Solenoid ON for 5 seconds.");
            digitalWrite(SOLENOID_PIN, HIGH); // Turn solenoid ON
            delay(5000); // Wait 5 seconds
            digitalWrite(SOLENOID_PIN, LOW); // Turn solenoid OFF
            Serial.println("Solenoid OFF.");
        } else {
            Serial.println("Invalid command. Type 'open' to activate the solenoid.");
        }
    }
}
 

// Calculate Total Light Intensity

uint16_t calculateLightIntensity(uint16_t green, uint16_t blue, uint16_t red) {

    return (green + blue + red) / 3;

}