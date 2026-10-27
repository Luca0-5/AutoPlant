#include <Wire.h>

#include <Adafruit_Sensor.h>

#include <Adafruit_BME280.h>

#include <ArduinoBLE.h>

 

// Sensor and Solenoid Pins

#define MOISTURE_SENSOR_PIN A2

#define SOLENOID_PIN 4

#define SEALEVELPRESSURE_HPA (1013.25)

 

// BLE Service
BLEService sensorService("180C");

 

// BLE Characteristics

BLEStringCharacteristic moistureChar("2A58", BLERead | BLENotify, 10);

BLEStringCharacteristic tempChar("2A59", BLERead | BLENotify, 10);

BLEStringCharacteristic humidityChar("2A5A", BLERead | BLENotify, 10);

BLEStringCharacteristic pressureChar("2A5B", BLERead | BLENotify, 10);

BLEStringCharacteristic lightChar("2A5C", BLERead | BLENotify, 10);

BLEStringCharacteristic solenoidControlChar("2A57", BLERead | BLEWrite | BLENotify, 10); // Fixed to Write + Notify

 

// BME280 Sensor Object

Adafruit_BME280 bme;

bool bme280_initialized = false;

 

// Solenoid state tracking

bool solenoidState = false;

unsigned long solenoidStartTime = 0;

 

void setup() {

    Serial.begin(115200);

    Wire.begin();

    pinMode(MOISTURE_SENSOR_PIN, INPUT);

    pinMode(SOLENOID_PIN, OUTPUT);

    digitalWrite(SOLENOID_PIN, LOW);

 

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

 

    // Add Service and Start Advertising

    BLE.addService(sensorService);

    BLE.advertise();

    Serial.println("BLE Device Ready. Connect to ESP32_Sensor.");

}

 

void loop() {

    BLEDevice central = BLE.central();

 

    if (central) {

        Serial.print("Connected to: ");

        Serial.println(central.address());

 

        while (central.connected()) {

            BLE.poll();  // Continuously check for BLE events

 

            // Read BLE Command for Solenoid

            if (solenoidControlChar.written()) {

                String command = solenoidControlChar.value();

                command.trim();  // Remove extra spaces and newlines

 

                Serial.print("Received via BLE: ");

                Serial.println(command);  // Print received data for debugging

 

                if (command.equalsIgnoreCase("open")) {

                    Serial.println("Solenoid ON.");

                    digitalWrite(SOLENOID_PIN, HIGH);

                    solenoidState = true;

                    solenoidStartTime = millis();

                } else if (command.equalsIgnoreCase("close")) {

                    Serial.println("Solenoid OFF.");

                    digitalWrite(SOLENOID_PIN, LOW);

                    solenoidState = false;

                } else {

                    Serial.println("Invalid command received.");

                }

            }

 

            // Auto-close solenoid after 5 seconds

            if (solenoidState && millis() - solenoidStartTime >= 5000) {

                Serial.println("Solenoid auto-closing after 5s.");

                digitalWrite(SOLENOID_PIN, LOW);

                solenoidState = false;

            }

 

            // Read sensors

            int moistureValue = analogRead(MOISTURE_SENSOR_PIN);

            int moisturePercent = map(moistureValue, 0, 2714, 0, 100);

            float tempC = bme.readTemperature();

            float humidity = bme.readHumidity();

            float pressure = bme.readPressure() / 100.0F;

            uint16_t lightIntensity = calculateLightIntensity(

                readAmbientLightData(0x0D),

                readAmbientLightData(0x10),

                readAmbientLightData(0x13));

 

            // Send sensor data via BLE

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

 

            delay(500);

        }

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

 

// Calculate Total Light Intensity

uint16_t calculateLightIntensity(uint16_t green, uint16_t blue, uint16_t red) {

    return (green + blue + red) / 3;

}