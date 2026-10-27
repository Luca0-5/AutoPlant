# AutoPlant

AutoPlant is an Arduino-based plant monitoring and watering project. The firmware reads soil moisture and environmental conditions, exposes readings over Bluetooth Low Energy (BLE), and can control a solenoid watering valve.

The repository also contains the initial Android application frame that will be used to build the mobile interface for the plant system.

## Repository Layout

```text
app/                         Android application module
gradle/                      Gradle wrapper and version catalog
build.gradle.kts             Root Android build configuration
settings.gradle.kts          Android project settings
arduino/
  AllWorkingOptimisedNoSleep/  Recommended integrated firmware
  WORKINGsensesolBLE/          BLE sensing and solenoid variant
  allsenseBLEnosolenoid/        BLE sensing without solenoid control
  allsensorandvalveopen/        Sensor and valve experiment
  moistlightBLE/                Moisture and light BLE experiment
  ...                           Additional moisture and sensor tests
```

See [arduino/README.md](arduino/README.md) for the complete sketch inventory.

## Android Application

The Android project is a Java and XML AppCompat application using the `com.example.autoplant` namespace. Its current activity is the original BLE implementation with scan/connect controls, queued sensor reads, and pump control.

Open the repository root in Android Studio and let Gradle sync before adding application features. The local `local.properties` file is intentionally not copied into the repository because it contains machine-specific Android SDK paths.

## Recommended Firmware

Start with [AllWorkingOptimisedNoSleep.ino](arduino/AllWorkingOptimisedNoSleep/AllWorkingOptimisedNoSleep.ino). It combines:

- Soil moisture readings converted to a percentage
- BME280 temperature, humidity, and pressure readings
- Ambient light readings
- BLE telemetry and solenoid commands
- Automatic solenoid close after five seconds

The sketch uses a custom BLE service and characteristics defined in the source. The device advertises as `ESP32_Sensor`.

## Hardware Assumptions

The imported sketches were written for an ESP32-class board and commonly assume:

- Analog soil moisture sensor on `A0` or `A2`, depending on the sketch
- Solenoid control on digital pin `4`
- BME280 environmental sensor connected over I2C
- Ambient light sensor connected over I2C
- A suitable driver or relay between the microcontroller and solenoid

Verify wiring, board selection, voltage levels, and calibration values before powering a valve. Pin assignments and moisture ranges are not consistent across the experimental sketches.

## Arduino Setup

1. Install the Arduino IDE and the board package for the target ESP32 board.
2. Open the recommended `.ino` file from its matching folder.
3. Install the libraries required by that sketch, including `ArduinoBLE`, `Adafruit BME280`, and `Adafruit VEML7700` where applicable.
4. Select the correct board and serial port.
5. Upload the sketch and open the serial monitor to inspect sensor readings and BLE status.

The repository does not currently include `arduino-cli` configuration or an automated Arduino build. Compilation should therefore be performed in the Arduino IDE until the target board and library versions are formalized. The Android app can be built with `./gradlew :app:assembleDebug` on systems with a configured JDK and Android SDK.

## Development Notes

The `arduino/` directory preserves the original sketches as separate experiments. Changes to one sketch should not be assumed to apply to the others. The integrated sketch is the best base for future cleanup, calibration, and application-level BLE integration. The Android app is intentionally still a frame so the BLE data contract and primary user flows can be designed around the chosen firmware.