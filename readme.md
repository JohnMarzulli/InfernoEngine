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
