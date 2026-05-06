#ifndef THERMAL_CONTROL_MANAGER
#define THERMAL_CONTROL_MANAGER

#include "TemperatureReport.h"

/**
 * @brief Class to handle predictions of how much air is needed.
 *        using the history of the how how the BBQ or grill has been.
 */
class ThermalControlManager {
public:
  /**
   * @brief Construct a new Thermal Control Manager object. Has no history.
   *        When there is no history, the fan will reccomend 0% output.
   */
  ThermalControlManager() {}

  /**
   * @brief Set the temperature that we want the BBQ pit or grill to be at.
   *
   * @param newTarget The new target temperature.
   */
  void SetTargetTemperature(float newTarget) { _targetTemperature = newTarget; }

  /**
   * @brief Adds a new temperature report to the history.
   *        The history will then be used to determine the target fan speed.
   *
   * @param report The new report to add. Invalid reports will be NOT be added.
   */
  void AddReport(TemperatureReport report) {
    if (report.IsValid()) {
      _temperatureHistory.Report(report);
    }
  }

  /**
   * @brief Performs any updates or device management during a "tick".
   *
   */
  void Service() {}

  /**
   * @brief Get a proportion (0.0 => off, 1.0 => full) of how hard the fan
   * should be running.
   *
   * @return float A proportion of how hard the fan should be running.
   */
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
    //       the actual effect the fan is really having... and to stabilize the
    //       heat in the pit.
    return 1.0;
  }

private:
  float GetPidProportion(float tempDelta) {
    // TODO: Implement PID control here. For now, just return a proportion based
    // on the delta and the range.
    return (tempDelta / pidRange) * pidScaler;
  }

  TemperatureHistory _temperatureHistory;
  float _targetTemperature = 0.0f;
  float pidRange = 10.0f;
  float pidScaler = 0.25f;
};

#endif