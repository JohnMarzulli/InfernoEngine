#ifndef GRILL_GAUGE_H
#define GRILL_GAUGE_H

#include "GaugeAdvertisement.h"
#include <stdint.h>
#include <string.h>

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <chrono>
static inline unsigned long millis() {
  using namespace std::chrono;
  return (unsigned long)duration_cast<milliseconds>(
             steady_clock::now().time_since_epoch())
      .count();
}
#endif

/**
 * @brief Tracks connection state and temperature for a single Combustion Inc
 * Giant Grill Gauge, identified by its 10-character alphanumeric serial number.
 *
 * Feed incoming advertisements via OnAdvertisement().
 * Call ReportMissed() from CombustionDeviceManager for each scan cycle in
 * which this device was not observed.
 *
 * GetTemp() returns the pit temperature in °C when the probe is connected.
 * IsSensorPresent() indicates whether the probe is physically attached.
 *
 */
class GrillGauge {
public:
  static const unsigned long CONNECTION_TIMEOUT_MS = 30000UL;

  explicit GrillGauge(const char *serialNumber)
      : _temperature(0.0f), _sensorPresent(false), _sensorOverheating(false),
        _lowBattery(false), _lastSuccessfulReadMs(0) {
    strncpy(_serialNumber, serialNumber, 10);
    _serialNumber[10] = '\0';
  }

  const char *GetSerialNumber() const { return _serialNumber; }

  bool IsConnected() const {
    if (_lastSuccessfulReadMs == 0)
      return false;
    return (millis() - _lastSuccessfulReadMs) <= CONNECTION_TIMEOUT_MS;
  }

  // True when the temperature probe is physically attached to the gauge.
  bool IsSensorPresent() const { return _sensorPresent; }
  bool IsSensorOverheating() const { return _sensorOverheating; }
  bool IsLowBattery() const { return _lowBattery; }

  // Pit temperature in °C. Only meaningful when IsSensorPresent() is true.
  float GetTemp() const { return _temperature; }
  float GetTargetTemperature() const {
    return (_alarmLowCelsius + _alarmHighCelsius) / 2.0f;
  }

  unsigned long GetLastReadMs() const { return _lastSuccessfulReadMs; }

  void OnAdvertisement(const GaugeAdvertisement &ad) {
    _temperature = ad.temperature;
    _sensorPresent = ad.sensorPresent;
    _sensorOverheating = ad.sensorOverheating;
    _lowBattery = ad.lowBattery;
    _lastSuccessfulReadMs = millis();
    _alarmLowCelsius = ad.lowAlarm.set ? ad.lowAlarm.threshold : 0.0f;
    _alarmHighCelsius = ad.highAlarm.set ? ad.highAlarm.threshold : 0.0f;
  }

private:
  char _serialNumber[11];
  float _alarmLowCelsius = 0.0f;
  float _alarmHighCelsius = 0.0f;
  float _temperature = 0.0f;
  bool _sensorPresent = false;
  bool _sensorOverheating = false;
  bool _lowBattery = false;
  unsigned long _lastSuccessfulReadMs = 0;
};

#endif
