#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define MOISTURE_SENSOR_PIN A0  // Analog pin for soil moisture sensor
#define SEALEVELPRESSURE_HPA (1013.25)  // Standard sea level pressure

Adafruit_BME280 bme;  // Create BME280 sensor object
bool bme280_initialized = false;

void setup() {
    Serial.begin(9600);  // Start serial communication
    Wire.begin();  // Initialize I2C
    pinMode(MOISTURE_SENSOR_PIN, INPUT);  // Set moisture sensor pin as input

    // Try to initialize BME280
    bme280_initialized = bme.begin(0x77);  // Default I2C address is 0x77
    if (!bme280_initialized) {
        Serial.println("BME280 not found! Check wiring.");
    }

    // Set the sensor to Active Mode and Enable Light Sensor (LS_EN)
    setSensorActiveModeAndEnableALS();
}

void loop() {
    // Read moisture sensor value
    int moistureValue = analogRead(MOISTURE_SENSOR_PIN);
    int moisturePercent = map(moistureValue, 0, 2714, 0, 100); // Convert to 0-100% scale

    // Read and display moisture data
    Serial.print("Moisture: ");
    Serial.print(moisturePercent);
    Serial.print("%  (Raw: ");
    Serial.print(moistureValue);
    Serial.println(")");

    if (bme280_initialized) {
        // Read and display BME280 sensor data
        float temperatureC = bme.readTemperature();
        float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;  // Convert to Fahrenheit
        float humidity = bme.readHumidity();
        float pressure = bme.readPressure() / 100.0F;  // Convert to hPa 

        Serial.print("Temp: ");
        Serial.print(temperatureC);
        Serial.print("°C / ");
        Serial.print(temperatureF);
        Serial.println("°F");

        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.println("%");

        Serial.print("Pressure: ");
        Serial.print(pressure);
        Serial.println(" hPa");
    }

    // Read ALS data for Green, Blue, and Red channels
    uint16_t greenLight = readAmbientLightData(0x0D);  // Green channel (LSB)
    uint16_t blueLight = readAmbientLightData(0x10);   // Blue channel (LSB)
    uint16_t redLight = readAmbientLightData(0x13);    // Red channel (LSB)

    // Calculate and display light intensity
    uint16_t lightIntensity = calculateLightIntensity(greenLight, blueLight, redLight);
    Serial.print("Light Intensity (Lux): ");
    Serial.println(lightIntensity);

    delay(1000);  // Delay 1 second between readings
}

// Function to set the sensor to Active Mode and Enable Light Sensor (LS_EN)
void setSensorActiveModeAndEnableALS() {
    Wire.beginTransmission(0x52);
    Wire.write(0x00);  // Main Control Register
    Wire.write(0x07);  // Set LS_EN (Light Sensor Enable) and activate the sensor
    Wire.endTransmission();

    delay(100);  // Wait for the sensor to activate
}

// Function to read Ambient Light Sensor (ALS) data from a specific channel
uint16_t readAmbientLightData(uint8_t registerAddress) {
    Wire.beginTransmission(0x52);
    Wire.write(registerAddress);  // Address of the specific ALS channel (LSB)
    Wire.endTransmission(false);

    Wire.requestFrom(0x52, (uint8_t)2);  // Request 2 bytes (LSB + MSB)

    uint16_t lightValue = 0;
    if (Wire.available() == 2) {
        uint8_t lsDataLSB = Wire.read();  // Read LSB
        uint8_t lsDataMSB = Wire.read();  // Read MSB

        // Combine MSB and LSB to form the 16-bit light sensor data value
        lightValue = (lsDataMSB << 8) | lsDataLSB;
    }

    return lightValue;
}

// Function to calculate total light intensity in Lux from Green, Blue, and Red channels
uint16_t calculateLightIntensity(uint16_t green, uint16_t blue, uint16_t red) {
    // Calculate the average of the three channels to approximate total light intensity
    uint16_t totalLight = (green + blue + red) / 3;
    return totalLight;
}
