#ifndef COMBUSTION_DEVICE_MANAGER_H
#define COMBUSTION_DEVICE_MANAGER_H

#include <BTstackLib.h>

#include "CombustionAdvertisement.h"
#include "CombustionBleParser.h"
#include "GaugeAdvertisement.h"
#include "GrillGauge.h"
#include "PredictiveThermometer.h"
#include "TemperatureHistory.h"
#include "TemperatureReport.h"
#include <string.h>

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
  CombustionDeviceManager() {
    _grillGauge = nullptr;
  }

  ~CombustionDeviceManager() {
    delete _grillGauge;
  }

  void Service() {
    bool rawConnected = _grillGauge != nullptr && _grillGauge->IsConnected();
    if (rawConnected) {
      _isConnected = true;
      _disconnectTicks = 0;
    } else if (++_disconnectTicks >= DISCONNECT_HYSTERESIS_TICKS) {
      _isConnected = false;
    }
  }

  // --- Convenience ---

  bool IsConnected() const { return _isConnected; }

  float GetTargetTemperature() const {
    if (_grillGauge != nullptr) {
      return _grillGauge->GetTargetTemperature();
    }

    return 0.0f;
  }

  // Pit temperature from the first connected Grill Gauge whose probe is
  // present.
  float GetCurrentTemperature() const {
    if (_grillGauge != nullptr && _grillGauge->IsConnected() && _grillGauge->IsSensorPresent()) {
      return _grillGauge->GetTemp();
    }

    return 0.0f;
  }

  // --- BLE feed ---

  void OnManufacturerPayload(const uint8_t *payload, uint8_t length) {
    if (length < 1)
      return;

    switch (payload[0]) {
      case CombustionBleParser::PRODUCT_TYPE_PREDICTIVE_THERMOMETER:
        {
          Serial.printf("[CPT] Payload received, length=%u\n", length);

          break;
        }
      case CombustionBleParser::PRODUCT_TYPE_GRILL_GAUGE:
        {
          /*
      Serial.printf("[GGG] Payload received, length=%u\n", length);
      */
          GaugeAdvertisement ad = CombustionBleParser::ParseGauge(payload, length);
          if (!ad.isValid) {
            Serial.println("[GGG] Parse failed — advertisement marked invalid\n");

            return;
          }
          Serial.printf("[GGG] Parsed - serial=%.10s, temp=%.1fC\n", ad.serialNumber,
                        ad.temperature);

          if (_grillGauge == nullptr) {
            Serial.printf("[GGG] New gauge discovered serial=%.10s\n",
                          ad.serialNumber);
            _grillGauge = new GrillGauge(ad.serialNumber);
          }

          _grillGauge->OnAdvertisement(ad);
          break;
        }
    }
  }

private:
  static const uint8_t DISCONNECT_HYSTERESIS_TICKS = 3;

  GrillGauge *_grillGauge;
  bool _isConnected = false;
  uint8_t _disconnectTicks = 0;
};

#endif
