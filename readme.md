# Inferno Engine

## Summary

A project born out of a broken Egg Genius and the need to have something to help with BBQ while waiting for the Combustion Engine

## Compiling & Flashing

## Building The Enclosure

## Running Unit Tests

Unit tests are accomplished using "PlatformIO".

This keeps the logic of the ESP32 code separate from the unit tests.

First you may need to install PlatformIO. On Pop_OS! or Ubuntu:

(Warning: DO NOT install platform IO by apt. This will cause version conflicts)

```bash
sudo apt install pipx
pipx install platformio
sudo apt-get install gdb
```

To run the tests, execute from the project home directory:

```bash
pio test -e native
```

You can also use the launch configuration to debug.

## Debugging

Included in the `Tools` folder is a Python program.

This will search for a CPT and GGG, printing connection and read data to the console.

Use this if the suspect the devices are not being found or connected to.

## Documentation

Combustion has placed their specification, and a plethora of helpful documentation at [Docs](https://github.com/combustion-inc/combustion-documentation/blob/main/gauge_ble_specification.rst#alarm-status)

## Wiring Diagram

### R-Pi Pico W

| Wire Color | Start        | End        |
| ---------- | ------------ | ---------- |
| Red        | Pi V-Sys Bus | 4056 Out + |
| Black      | Pi GND       | 4056 Out - |
| Orange     | Pi GP IO 0   | L298N ENA  |
| Red        | Pi 3V3       | L298N IN1  |
| Black      | Pi GND       | L298N IN2  |

### 4056 USB-C Charger

| Wire Color | Start      | End            |
| ---------- | ---------- | -------------- |
| Red        | 4056 B+    | JST 1.25 Red   |
| Black      | 4056 B-    | JST 1.25 Black |
| Red        | 4056 Out + | MT3608 VIN +   |
| Black      | 4056 Out - | MT3608 VIN -   |

Note that the "Out +" and "Out -" connections will need to both have "Y" pigtails.

Both "Out +" and "Out -" travel to both the MT3608 and the R-Pi Pico W

### MT3608 Voltage Controller

| Wire Color | Start          | End       |
| ---------- | -------------- | --------- |
| Red        | MT3608 V Out + | L298N 12V |
| Black      | MT3608 V Out - | L298N GND |

### L298N Motor Controller

The motor controller is a destination for all of the other modules.

Please remove the jumper on ENA. The target pin for ENA is the one closes to the edge of the board.
