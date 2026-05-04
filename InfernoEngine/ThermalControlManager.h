#ifndef THERMAL_CONTROL_MANAGER
#define THERMAL_CONTROL_MANAGER

#include "TemperatureReport.h"

class ThermalControlManager {
public:

  ThermalControlManager() {}

  void SetTargetTemperature() {}

  void AddReport(
    TemperatureReport report) {
  }

  void Service() {}

  float GetTargetFanProportion() {
    return 1.0;
  }
};

#endif