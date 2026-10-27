# AutoPlant Arduino sketches

These sketches were imported from the local Arduino workspace because they contain BLE, water-control, soil-moisture, or related environmental-sensor code.

## Recommended starting point

- `AllWorkingOptimisedNoSleep/AllWorkingOptimisedNoSleep.ino` combines BLE telemetry, soil-moisture percentage, BME280 temperature/humidity/pressure, ambient light, and timed solenoid control.

## BLE and water-control variants

- `WORKINGsensesolBLE/WORKINGsensesolBLE.ino`
- `allsenseBLEnosolenoid/allsenseBLEnosolenoid.ino`
- `allsensorandvalveopen/allsensorandvalveopen.ino`
- `moistlightBLE/moistlightBLE.ino`

## Moisture and environmental sensor experiments

- `WORKINGmoistlighttemp/WORKINGmoistlighttemp.ino`
- `moisttemplightNolight/moisttemplightNolight.ino`
- `moisttemp/moisttemp.ino`
- `workingmosittemp/workingmosittemp.ino`
- `moistandlight/moistandlight.ino`
- `moistureproperpercent/moistureproperpercent.ino`
- `MoistTest/MoistTest.ino`

## Earlier sketches

- `final/final.ino`
- `finalfinal/finalfinal.ino`

Most sketches target an ESP32-class board and require libraries such as `ArduinoBLE`, `Adafruit BME280`, `Adafruit VEML7700`, and their dependencies. Pin assignments and moisture calibration ranges remain specific to each original sketch.