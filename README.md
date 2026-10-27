# AutoPlant

AutoPlant is an Arduino-based plant monitoring and watering project. The firmware reads soil moisture and environmental conditions, exposes readings over Bluetooth Low Energy (BLE), and can control a solenoid watering valve.

## Repository Layout

```text
arduino/
  AllWorkingOptimisedNoSleep/  Recommended integrated firmware
  WORKINGsensesolBLE/          BLE sensing and solenoid variant
  allsenseBLEnosolenoid/        BLE sensing without solenoid control
  allsensorandvalveopen/        Sensor and valve experiment
  moistlightBLE/                Moisture and light BLE experiment
  ...                           Additional moisture and sensor tests
```

See [arduino/README.md](arduino/README.md) for the complete sketch inventory.

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

The repository does not currently include `arduino-cli` configuration or an automated build. Compilation should therefore be performed in the Arduino IDE until the target board and library versions are formalized.

## Development Notes

The `arduino/` directory preserves the original sketches as separate experiments. Changes to one sketch should not be assumed to apply to the others. The integrated sketch is the best base for future cleanup, calibration, and application-level BLE integration.