#ifndef COMBUSTION_DEVICE_MANAGER
#define COMBUSTION_DEVICE_MANAGER

#include "TemperatureReport.h"

class CombustionDeviceManager {
public:
  CombustionDeviceManager() {
  }

  void Service() {}

  float GetCurrentTemperature() {
    return 0.0;
  }

  bool IsConnected() {
    return false;
  }
};

#endif