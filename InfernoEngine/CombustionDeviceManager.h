#ifndef COMBUSTION_DEVICE_MANAGER_H
#define COMBUSTION_DEVICE_MANAGER_H

#include "CombustionAdvertisement.h"
#include "CombustionBleParser.h"
#include "GaugeAdvertisement.h"
#include "GrillGauge.h"
#include "PredictiveThermometer.h"
#include <string.h>

static const int MAX_COMBUSTION_DEVICES = 8;

/**
 * @brief Manages discovery and lifetime of all Combustion Inc BLE devices.
 *
 * Routes incoming payloads by product type:
 *   0x01 (CPT) → PredictiveThermometer  identified by uint32 serial
 *   0x03 (GGG) → GrillGauge             identified by 10-char alphanumeric
 *   serial
 *
 * Call OnManufacturerPayload() from the platform BLE layer for every packet
 * whose company ID is CombustionBleParser::COMPANY_ID.
 * Call OnScanCycleComplete() at the end of each scan window.
 *
 */
class CombustionDeviceManager {
public:
  CombustionDeviceManager()
      : _thermometerCount(0), _grillGaugeCount(0), _cptSeenCount(0),
        _gggSeenCount(0) {
    for (int i = 0; i < MAX_COMBUSTION_DEVICES; i++) {
      _thermometers[i] = nullptr;
      _grillGauges[i] = nullptr;
    }
  }

  ~CombustionDeviceManager() {
    for (int i = 0; i < _thermometerCount; i++)
      delete _thermometers[i];
    for (int i = 0; i < _grillGaugeCount; i++)
      delete _grillGauges[i];
  }

  void Service() { PlatformService(); }

  // --- Thermometer (CPT) accessors ---

  int GetThermometerCount() const { return _thermometerCount; }

  PredictiveThermometer *GetThermometer(int index) {
    if (index < 0 || index >= _thermometerCount)
      return nullptr;
    return _thermometers[index];
  }

  PredictiveThermometer *GetThermometerBySerial(uint32_t serial) {
    for (int i = 0; i < _thermometerCount; i++) {
      if (_thermometers[i]->GetSerialNumber() == serial)
        return _thermometers[i];
    }
    return nullptr;
  }

  // --- Grill Gauge (GGG) accessors ---

  int GetGrillGaugeCount() const { return _grillGaugeCount; }

  GrillGauge *GetGrillGauge(int index) {
    if (index < 0 || index >= _grillGaugeCount)
      return nullptr;
    return _grillGauges[index];
  }

  GrillGauge *GetGrillGaugeBySerial(const char *serial) {
    for (int i = 0; i < _grillGaugeCount; i++) {
      if (strncmp(_grillGauges[i]->GetSerialNumber(), serial, 10) == 0)
        return _grillGauges[i];
    }
    return nullptr;
  }

  // --- Convenience ---

  bool IsConnected() const {
    for (int i = 0; i < _thermometerCount; i++) {
      if (_thermometers[i]->IsConnected())
        return true;
    }
    for (int i = 0; i < _grillGaugeCount; i++) {
      if (_grillGauges[i]->IsConnected())
        return true;
    }
    return false;
  }

  // Pit temperature from the first connected Grill Gauge whose probe is
  // present.
  float GetCurrentTemperature() const {
    for (int i = 0; i < _grillGaugeCount; i++) {
      if (_grillGauges[i]->IsConnected() && _grillGauges[i]->IsSensorPresent())
        return _grillGauges[i]->GetTemp();
    }
    return 0.0f;
  }

  // --- BLE feed ---

  void OnManufacturerPayload(const uint8_t *payload, uint8_t length) {
    if (length < 1)
      return;

    switch (payload[0]) {
    case CombustionBleParser::PRODUCT_TYPE_PREDICTIVE_THERMOMETER: {
      CombustionAdvertisement ad = CombustionBleParser::Parse(payload, length);
      if (!ad.isValid)
        return;
      PredictiveThermometer *t = FindOrCreateThermometer(ad.serialNumber);
      if (t) {
        t->OnAdvertisement(ad);
        MarkCptSeen(ad.serialNumber);
      }
      break;
    }
    case CombustionBleParser::PRODUCT_TYPE_GRILL_GAUGE: {
      GaugeAdvertisement ad = CombustionBleParser::ParseGauge(payload, length);
      if (!ad.isValid)
        return;
      GrillGauge *g = FindOrCreateGrillGauge(ad.serialNumber);
      if (g) {
        g->OnAdvertisement(ad);
        MarkGggSeen(ad.serialNumber);
      }
      break;
    }
    }
  }

  void OnScanCycleComplete() {
    for (int i = 0; i < _thermometerCount; i++) {
      if (!WasCptSeenThisCycle(_thermometers[i]->GetSerialNumber()))
        _thermometers[i]->ReportMissed();
    }
    for (int i = 0; i < _grillGaugeCount; i++) {
      if (!WasGggSeenThisCycle(_grillGauges[i]->GetSerialNumber()))
        _grillGauges[i]->ReportMissed();
    }
    ClearSeenThisCycle();
  }

private:
  PredictiveThermometer *_thermometers[MAX_COMBUSTION_DEVICES];
  int _thermometerCount;

  GrillGauge *_grillGauges[MAX_COMBUSTION_DEVICES];
  int _grillGaugeCount;

  // CPT seen-this-cycle: tracked by uint32 serial
  uint32_t _cptSeenSerials[MAX_COMBUSTION_DEVICES];
  bool _cptSeenThisCycle[MAX_COMBUSTION_DEVICES];
  int _cptSeenCount;

  // GGG seen-this-cycle: tracked by 10-char alphanumeric serial
  char _gggSeenSerials[MAX_COMBUSTION_DEVICES][11];
  bool _gggSeenThisCycle[MAX_COMBUSTION_DEVICES];
  int _gggSeenCount;

  PredictiveThermometer *FindOrCreateThermometer(uint32_t serial) {
    for (int i = 0; i < _thermometerCount; i++) {
      if (_thermometers[i]->GetSerialNumber() == serial)
        return _thermometers[i];
    }
    if (_thermometerCount >= MAX_COMBUSTION_DEVICES)
      return nullptr;
    PredictiveThermometer *t = new PredictiveThermometer(serial);
    _thermometers[_thermometerCount] = t;
    _cptSeenSerials[_cptSeenCount] = serial;
    _cptSeenThisCycle[_cptSeenCount] = false;
    _cptSeenCount++;
    _thermometerCount++;
    return t;
  }

  GrillGauge *FindOrCreateGrillGauge(const char *serial) {
    for (int i = 0; i < _grillGaugeCount; i++) {
      if (strncmp(_grillGauges[i]->GetSerialNumber(), serial, 10) == 0)
        return _grillGauges[i];
    }
    if (_grillGaugeCount >= MAX_COMBUSTION_DEVICES)
      return nullptr;
    GrillGauge *g = new GrillGauge(serial);
    _grillGauges[_grillGaugeCount] = g;
    strncpy(_gggSeenSerials[_gggSeenCount], serial, 10);
    _gggSeenSerials[_gggSeenCount][10] = '\0';
    _gggSeenThisCycle[_gggSeenCount] = false;
    _gggSeenCount++;
    _grillGaugeCount++;
    return g;
  }

  void MarkCptSeen(uint32_t serial) {
    for (int i = 0; i < _cptSeenCount; i++) {
      if (_cptSeenSerials[i] == serial) {
        _cptSeenThisCycle[i] = true;
        return;
      }
    }
  }

  bool WasCptSeenThisCycle(uint32_t serial) const {
    for (int i = 0; i < _cptSeenCount; i++) {
      if (_cptSeenSerials[i] == serial)
        return _cptSeenThisCycle[i];
    }
    return false;
  }

  void MarkGggSeen(const char *serial) {
    for (int i = 0; i < _gggSeenCount; i++) {
      if (strncmp(_gggSeenSerials[i], serial, 10) == 0) {
        _gggSeenThisCycle[i] = true;
        return;
      }
    }
  }

  bool WasGggSeenThisCycle(const char *serial) const {
    for (int i = 0; i < _gggSeenCount; i++) {
      if (strncmp(_gggSeenSerials[i], serial, 10) == 0)
        return _gggSeenThisCycle[i];
    }
    return false;
  }

  void ClearSeenThisCycle() {
    for (int i = 0; i < _cptSeenCount; i++)
      _cptSeenThisCycle[i] = false;
    for (int i = 0; i < _gggSeenCount; i++)
      _gggSeenThisCycle[i] = false;
  }

  void PlatformService() {
#if defined(ARDUINO_ARCH_RP2040)
    // TODO: drive BTstack scan loop (earlephilhower/arduino-pico)
    // BTstack.loop();
#elif defined(ESP32)
    // TODO: drive ESP32 BLE scan loop
#else
    // Native / test build — no BLE hardware
#endif
  }
};

#endif
