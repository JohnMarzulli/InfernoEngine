#ifndef PREDICTIVE_THERMOMETER_H
#define PREDICTIVE_THERMOMETER_H

#include "CombustionAdvertisement.h"
#include <stdint.h>

// Provide millis() for non-Arduino builds (native tests).
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
 * @brief
 * Tracks connection state and the most recent ambient temperature reading
 * for a single Combustion Predictive Thermometer, identified by serial number.
 *
 * Feed incoming advertisements via OnAdvertisement().
 * Call ReportMissed() from CombustionDeviceManager for each scan cycle in
 * which this device was not observed.
 *
 */
class PredictiveThermometer {
public:
  static const uint8_t MAX_CONSECUTIVE_FAILURES = 5;
  static const unsigned long CONNECTION_TIMEOUT_MS = 120000UL; // 2 minutes

  explicit PredictiveThermometer(uint32_t serialNumber)
      : _serialNumber(serialNumber), _cachedAmbientTemp(0.0f),
        _lastSuccessfulReadMs(0), _consecutiveFailures(0) {}

  uint32_t GetSerialNumber() const { return _serialNumber; }

  // IsConnected rules:
  //   false — never received a valid advertisement
  //   false — MAX_CONSECUTIVE_FAILURES missed scan cycles in a row
  //   true  — last advertisement was within CONNECTION_TIMEOUT_MS
  bool IsConnected() const {
    if (_lastSuccessfulReadMs == 0)
      return false;
    if (_consecutiveFailures >= MAX_CONSECUTIVE_FAILURES)
      return false;
    return (millis() - _lastSuccessfulReadMs) <= CONNECTION_TIMEOUT_MS;
  }

  // Returns the ambient temperature (T8, handle-end sensor) in °C
  // from the most recent advertisement. Returns 0.0 before first read.
  float GetTemp() const { return _cachedAmbientTemp; }

  // Called by CombustionDeviceManager when a matching advertisement arrives.
  void OnAdvertisement(const CombustionAdvertisement &ad) {
    _cachedAmbientTemp = ad.GetAmbientTemp();
    _lastSuccessfulReadMs = millis();
    _consecutiveFailures = 0;
  }

  // Called by CombustionDeviceManager for each scan cycle that completed
  // without seeing this device. Accumulates toward MAX_CONSECUTIVE_FAILURES.
  void ReportMissed() {
    if (_lastSuccessfulReadMs == 0)
      return; // never seen — don't penalise
    if (_consecutiveFailures < MAX_CONSECUTIVE_FAILURES) {
      _consecutiveFailures++;
    }
  }

  void Service() {}

private:
  uint32_t _serialNumber;
  float _cachedAmbientTemp;
  unsigned long _lastSuccessfulReadMs;
  uint8_t _consecutiveFailures;
};

#endif
