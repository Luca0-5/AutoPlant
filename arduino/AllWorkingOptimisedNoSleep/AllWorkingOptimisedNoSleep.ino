#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ArduinoBLE.h>

// Sensor and Solenoid Pins
#define MOISTURE_SENSOR_PIN A0
#define SOLENOID_PIN 4
#define SEALEVELPRESSURE_HPA (1013.25)

// BLE Service and Characteristics
BLEService sensorService("12345678-1234-5678-1234-56789abcdef0"); 
BLEStringCharacteristic moistureChar("12345678-1234-5678-1234-56789abcdef1", BLERead | BLENotify, 10);
BLEStringCharacteristic tempChar("12345678-1234-5678-1234-56789abcdef2", BLERead | BLENotify, 10);
BLEStringCharacteristic humidityChar("12345678-1234-5678-1234-56789abcdef3", BLERead | BLENotify, 10);
BLEStringCharacteristic pressureChar("12345678-1234-5678-1234-56789abcdef4", BLERead | BLENotify, 10);
BLEStringCharacteristic lightChar("12345678-1234-5678-1234-56789abcdef5", BLERead | BLENotify, 10);
BLEStringCharacteristic solenoidControlChar("12345678-1234-5678-1234-56789abcdef6", BLERead | BLEWrite | BLENotify, 10);

// BME280 Sensor Object
Adafruit_BME280 bme;
bool bme280_initialized = false;

// Solenoid state tracking
bool solenoidState = false;
unsigned long solenoidStartTime = 0;

// Setup
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
    sensorService.addCharacteristic(moistureChar);
    sensorService.addCharacteristic(tempChar);
    sensorService.addCharacteristic(humidityChar);
    sensorService.addCharacteristic(pressureChar);
    sensorService.addCharacteristic(lightChar);
    sensorService.addCharacteristic(solenoidControlChar);
    BLE.addService(sensorService);
    BLE.advertise();

    Serial.println("BLE Device Ready. Connect to ESP32_Sensor.");
}

// Loop
void loop() {
    BLEDevice central = BLE.central();
    if (central) {
        Serial.print("Connected to: ");
        Serial.println(central.address());

        while (central.connected()) {
            BLE.poll();  // Handle BLE events

            // Read BLE Command for Solenoid
            if (solenoidControlChar.written()) {
                handleSolenoidCommand();
            }

            // Auto-close solenoid after 5 seconds
            if (solenoidState && millis() - solenoidStartTime >= 5000) {
                closeSolenoid();
            }

            // Read Sensors
            readAndSendSensorData();

            // Delay to reduce BLE polling frequency and save power
            delay(1000);  // Use a longer delay for better power efficiency
        }
    }
}

// Function to Handle Solenoid Commands
void handleSolenoidCommand() {
    String command = solenoidControlChar.value();
    command.trim();
    if (command.equalsIgnoreCase("open")) {
        openSolenoid();
    } else if (command.equalsIgnoreCase("close")) {
        closeSolenoid();
    } else {
        Serial.println("Invalid command received.");
    }
}

void openSolenoid() {
    Serial.println("Solenoid ON.");
    digitalWrite(SOLENOID_PIN, HIGH);
    solenoidState = true;
    solenoidStartTime = millis();
}

void closeSolenoid() {
    Serial.println("Solenoid OFF.");
    digitalWrite(SOLENOID_PIN, LOW);
    solenoidState = false;
}

// Function to Read and Send Sensor Data
void readAndSendSensorData() {
    int moisturePercent = getMoisturePercent();
    float tempC = bme.readTemperature();
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F;
    uint16_t lightIntensity = getLightIntensity();

    sendDataToBLE(moisturePercent, tempC, humidity, pressure, lightIntensity);
}

// Function to Read Moisture Sensor Value and Convert to Percentage
int getMoisturePercent() {
    int moistureValue = analogRead(MOISTURE_SENSOR_PIN);
    return map(moistureValue, 1000, 3000, 0, 100);  // Adjust to your sensor range
}

// Function to Calculate Light Intensity
uint16_t getLightIntensity() {
    uint16_t green = readAmbientLightData(0x0D);
    uint16_t blue = readAmbientLightData(0x10);
    uint16_t red = readAmbientLightData(0x13);
    return (green + blue + red) / 3;  // Use a simple average of RGB channels
}

// Function to Read Ambient Light Data
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

// Function to Send Sensor Data to BLE
void sendDataToBLE(int moisturePercent, float tempC, float humidity, float pressure, uint16_t lightIntensity) {
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
}

// Function to Enable Ambient Light Sensor
void setSensorActiveModeAndEnableALS() {
    Wire.beginTransmission(0x52);
    Wire.write(0x00);
    Wire.write(0x07);
    Wire.endTransmission();
    delay(100);
}
