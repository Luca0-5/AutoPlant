#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <DFRobot_VEML7700.h>  // Include the VEML7700 library

#define MOISTURE_SENSOR_PIN A0  // Analog pin for soil moisture sensor
#define SEALEVELPRESSURE_HPA (1013.25)  // Standard sea level pressure

// Global sensor objects
Adafruit_BME280 bme;  // Create BME280 sensor object
DFRobot_VEML7700 lightSensor;  // Create VEML7700 sensor object

// Global variables
bool bme280_initialized = false;
bool veml_initialized = false;
float lux = 0.0;  // Store lux value globally
int moisturePercent = 0;  // Store moisture percentage globally
float temperatureC = 0.0, temperatureF = 0.0, humidity = 0.0, pressure = 0.0;

void setup() {
    Serial.begin(9600);
    Wire.begin();  // Initialize I2C

    pinMode(MOISTURE_SENSOR_PIN, INPUT);

    // Try to initialize BME280
    bme280_initialized = bme.begin(0x77);  // Default I2C address is 0x76
    if (!bme280_initialized) {
        Serial.println("Warning: BME280 not found! Check wiring.");
    }

    // Initialize the VEML7700 with default settings
    lightSensor.begin(0x10);  // Default I2C address is 0x10

    // Test if the sensor works by attempting to read lux
    if (lightSensor.getALSLux(lux) == 0) {
        veml_initialized = true;
        Serial.println("VEML7700 initialized successfully.");
    } else {
        veml_initialized = false;
        Serial.println("Failed to initialize VEML7700.");
    }
  }

void loop() {
    // Read moisture sensor value and update global variable
    int moistureValue = analogRead(MOISTURE_SENSOR_PIN);
    moisturePercent = map(moistureValue, 0, 4095, 100, 0); // Convert to 0-100% scale

    // Print moisture data
    Serial.print("Moisture: ");
    Serial.print(moisturePercent);
    Serial.println("%");

    // Read BME280 data and update global variables
    temperatureC = bme.readTemperature();
    temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;  // Convert to Fahrenheit
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0F;  // Convert to hPa 

    // Print BME280 data
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

    // Get ALS (ambient light sensor) lux value and update global variable
    lightSensor.getALSLux(lux); // Update the lux value

    // Print the lux value
    if (lux == 255.0) {
        Serial.println("Light Intensity: Error or max value (255 lux)");
    } else {
        Serial.print("Light Intensity: ");
        Serial.print(lux);
        Serial.println(" lux");
    }

    delay(1000);  // Delay 1 second
  }
