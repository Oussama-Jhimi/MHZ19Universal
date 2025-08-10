# MHZ19Universal

Arduino/Embedded library for MH-Z19 CO₂ sensors (B/C/D) — Version 1.1.0

## Purpose
Provide a unified, robust, and portable API to read CO₂ concentration and temperature from MH-Z19B, MH-Z19C, and MH-Z19D sensors.  
Includes: variant detection, calibration, auto-calibration control, range change, and optional software filtering (moving average).

## Contents
- `MHZ19Universal.h` / `MHZ19Universal.cpp`: library source files
- `examples/ExempleDeBase/ExempleDeBase.ino`: example sketch
- `README.md`: this file
- `library.properties`: Arduino IDE metadata
- `keywords.txt`: syntax highlighting for Arduino IDE

## Installation
1. Extract the `MHZ19Universal` folder into `Documents/Arduino/libraries/` (Windows) or `~/Arduino/libraries/` (Linux/Mac).
2. Restart the Arduino IDE.
3. Open `File > Examples > MHZ19Universal > ExempleDeBase`.

## Basic Usage
- Include the library:
  ```cpp
  #include <MHZ19Universal.h>
  ```
- Initialize (e.g., using SoftwareSerial):
  ```cpp
  SoftwareSerial sw(10, 11);
  MHZ19Universal sensor;
  sensor.begin(sw);
  ```
- Read CO₂:
  ```cpp
  int co2;
  if (sensor.readCO2(co2) == MHZ19_OK) {
    Serial.println(co2);
  }
  ```
- Read temperature:
  ```cpp
  float temp;
  sensor.readTemperature(temp);
  ```

## Important Notes
- **Logic voltage**: Most MH-Z19 modules use 5V TTL. If using an ESP (3.3V logic), use a level shifter.
- **Temperature offset**: The library uses `temp = buf[4] - 40`. Some clones output a different offset (e.g., `-2`). Verify against a thermometer and adjust if needed.
- **Version command / detection**: Automatic detection is tolerant (checks multiple bytes). If your module does not respond to `CMD_VERSION`, detection will return `Unknown`.
- **Unlock/lock**: Some firmware versions require unlocking before changing parameters. The `0xA5A5` code in the example is illustrative—check your module’s documentation.

## Debug
- To enable detailed serial debug logs, uncomment `#define MHZ19_DEBUG` at the top of `MHZ19Universal.h`.
- You can retrieve the last raw 9-byte response with `getLastRawResponse()`.

## Changelog
- v1.1.0: More tolerant detection, optional debug, `getLastRawResponse`, improved `receiveResponse`, updated documentation.

## License
LGPLv3 — See the official license files if you redistribute this code.
