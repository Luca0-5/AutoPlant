#define SENSOR_PIN A0  // A2 on Nano ESP32 (ADC1)

void setup() {
  Serial.begin(115200);  // Start Serial Monitor at 115200 baud
}

void loop() {
  int sensorValue = analogRead(SENSOR_PIN);  // Read analog value
  Serial.print("Moisture Sensor Value: ");
  Serial.println(sensorValue);  // Print raw ADC value
  delay(1000);  // Wait 1 second
}
