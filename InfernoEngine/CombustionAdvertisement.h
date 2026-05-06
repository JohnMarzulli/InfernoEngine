#ifndef COMBUSTION_ADVERTISEMENT_H
#define COMBUSTION_ADVERTISEMENT_H

#include <stdint.h>

static const int COMBUSTION_PROBE_COUNT = 8;

/**
 * @brief Holds the data from an advertisement from a Combustion Inc device, parsed from raw BLE payload.
 * 
 */
struct CombustionAdvertisement {
  uint8_t productType;
  uint32_t serialNumber;
  float temperatures[COMBUSTION_PROBE_COUNT]; // °C, T1=tip, T8=nearest ambient
  bool isValid;

  CombustionAdvertisement() : productType(0), serialNumber(0), isValid(false) {
    for (int i = 0; i < COMBUSTION_PROBE_COUNT; i++) {
      temperatures[i] = 0.0f;
    }
  }

  // T8 is the handle-end sensor, exposed to ambient air
  float GetAmbientTemp() const {
    return temperatures[COMBUSTION_PROBE_COUNT - 1];
  }
};

#endif
