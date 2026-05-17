#ifndef UNCONNTECTED_THERMAL_CONTROL_MANAGER
#define UNCONNTECTED_THERMAL_CONTROL_MANAGER

#include "TemperatureReport.h"

/**
 * @brief Class to handle predictions of how much air is needed.
 *        using the history of the how how the BBQ or grill has been.
 */
class UnconnectedControlManager {
public:
  /**
   * @brief Construct a new Thermal Control Manager object. Has no history.
   *        When there is no history, the fan will reccomend 0% output.
   */
  UnconnectedControlManager() { _startTime = millis(); }

  /**
   * @brief Not implemented. Used to maitain interface.
   *
   * @param newTarget The new target temperature.
   */
  void SetTargetTemperature(float newTarget) {}

  /**
   * @brief Not implemented. Used to maitain interface.
   *
   * @param report The new report to add. Invalid reports will be NOT be added.
   */
  void AddReport(TemperatureReport report) {}

  /**
   * @brief Performs any updates or device management during a "tick".
   *
   */
  void Service() {
    const int FAN_CYCLE_TIME_SECONDS = 4;
    unsigned long currentTime = millis();
    unsigned long deltaTime = currentTime - _startTime;
    unsigned long secondsSinceStart = deltaTime / 1000;

    _targetProportion = (float)(secondsSinceStart % FAN_CYCLE_TIME_SECONDS) /
                        (float)FAN_CYCLE_TIME_SECONDS;
  }

  /**
   * @brief Get a proportion (0.0 => off, 1.0 => full) of how hard the fan
   * should be running.
   *
   * @return float A proportion of how hard the fan should be running.
   */
  float GetTargetFanProportion() { return _targetProportion; }

private:
  unsigned long _startTime;
  float _targetProportion;
};

#endif