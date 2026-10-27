#define MOISTURE_SENSOR_PIN A0  // Analog pin for soil moisture sensor

void setup() {
    Serial.begin(9600);
    pinMode(MOISTURE_SENSOR_PIN, INPUT);
}

void loop() {
    // Read moisture sensor value
    int moistureValue = analogRead(MOISTURE_SENSOR_PIN);
    int moisturePercent = map(moistureValue, 0, 2714, 0, 100); // Convert to 0-100% scale

    // Print raw and percentage moisture data
    Serial.print("Moisture Raw Value: ");
    Serial.println(moistureValue);
    Serial.print("Moisture Percentage: ");
    Serial.print(moisturePercent);
    Serial.println("%");

    delay(500);  // Delay 1 second
}
