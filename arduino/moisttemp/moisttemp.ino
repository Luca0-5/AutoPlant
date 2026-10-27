#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define MOISTURE_SENSOR_PIN A0  // Analog pin for soil moisture sensor
#define SEALEVELPRESSURE_HPA (1013.25)  // Standard sea level pressure

Adafruit_BME280 bme;  // Create BME280 sensor object
bool bme280_initialized = false;

void setup() {
    Serial.begin(9600);
    Wire.begin();  // Initialize I2C

    pinMode(MOISTURE_SENSOR_PIN, INPUT);

    // Try to initialize BME280
    bme280_initialized = bme.begin(0x76);  // Default I2C address is 0x76
    if (!bme280_initialized) {
        Serial.println("Warning: BME280 not found! Check wiring.");
    }
}

void loop() {
    // Read moisture sensor value
    int moistureValue = analogRead(MOISTURE_SENSOR_PIN);
    int moisturePercent = map(moistureValue, 0, 4095, 100, 0); // Convert to 0-100% scale

    // Print moisture data
    Serial.print("Moisture: ");
    Serial.print(moisturePercent);
    Serial.println("%");

    // Only read BME280 values if successfully initialized
    if (bme280_initialized) {
        float temperatureC = bme.readTemperature();
        float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;  // Convert to Fahrenheit
        float humidity = bme.readHumidity();
        float pressure = bme.readPressure() / 100.0F;  // Convert to hPa

        Serial.print("Temperature: ");
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
    } else {
        Serial.println("BME280 data unavailable.");
    }

    delay(1000);  // Delay 1 second
}
