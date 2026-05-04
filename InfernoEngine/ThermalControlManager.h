#ifndef THERMAL_CONTROL_MANAGER
#define THERMAL_CONTROL_MANAGER

#include "TemperatureReport.h"

class ThermalControlManager {
public:

  ThermalControlManager() {}

  void SetTargetTemperature(
    float newTarget) {
    _targetTemperature = newTarget;
  }

  void AddReport(
    TemperatureReport report) {
  }

  void Service() {}

  float GetTargetFanProportion() {
    if (_temperatureHistory.GetReportCount() <= 0) {
      return 0.0f;
    }

    float currentTemp = _temperatureHistory.GetCurrentTemp();
    float tempDelta = _targetTemperature - currentTemp;

    if (currentTemp >= _targetTemperature) {
      return 0.0f;
    }

    if (tempDelta <= pidRange) {
      return GetPidProportion(tempDelta);
    }

    // TODO: Use the regression system to determine the proportion here.
    // TODO: Implement a system where the fan turns off for a while to determine
    //       the actual effect the fan is really having... and to stabilize the heat in the pit.
    return 1.0;
  }

private:

  float GetPidProportion(
    float tempDelta) {
    // TODO: Implement PID control here. For now, just return a proportion based on the delta and the range.
    return (tempDelta / pidRange) * pidScaler;
  }

  TemperatureHistory _temperatureHistory;
  float _targetTemperature = 0.0f;
  float pidRange = 10.0f;
  float pidScaler = 0.25f;
};

#endif