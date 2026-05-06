#ifndef GAUGE_ADVERTISEMENT_H
#define GAUGE_ADVERTISEMENT_H

#include <stdint.h>

/**
 * @brief Decoded state of one high or low alarm threshold.
 * Temperature encoding: (raw13 * 0.1) - 20.0  (same as gauge temperature).
 */
struct AlarmStatus {
  bool set;        // alarm threshold has been configured
  bool tripped;    // temperature has crossed the threshold
  bool alarming;   // alarm is actively sounding
  float threshold; // °C

  AlarmStatus()
      : set(false), tripped(false), alarming(false), threshold(0.0f) {}
};

/**
 * @brief
 * Parsed representation of a Combustion Inc Giant Grill Gauge BLE
 * advertisement.
 *
 * Temperature encoding: (raw_13bit * 0.1) - 20.0  → range -20°C to 799°C
 * Serial number: 10-byte alphanumeric, stored as null-terminated string.
 *
 */
struct GaugeAdvertisement {
  char serialNumber[11]; // 10 bytes alphanumeric + null terminator
  float temperature;     // °C; 0.1°C resolution
  bool sensorPresent;    // probe is physically connected
  bool sensorOverheating;
  bool lowBattery;
  AlarmStatus highAlarm;
  AlarmStatus lowAlarm;
  bool isValid;

  GaugeAdvertisement()
      : temperature(0.0f), sensorPresent(false), sensorOverheating(false),
        lowBattery(false), isValid(false) {
    serialNumber[0] = '\0';
  }
};

#endif
