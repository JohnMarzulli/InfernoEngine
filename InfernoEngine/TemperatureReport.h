#ifndef TEMPERATURE_REPORT_H
#define TEMPERATURE_REPORT_H

/**
 * @brief Saves a temperature report and the time offset.
 *
 */
class TemperatureReport {
public:
  /**
   * @brief The default constructor creates an invalid report.
   * This exists only for array creation.
   */
  TemperatureReport() {
    _isValid = false;
    _temp = 0.0;
    _time = 0;
  }

  /**
   * @brief Construct a new Temperature Report object
   *
   * @param currentTime The time offset in milliseconds since the start of the
   * cook when this report was taken.
   * @param currentTemp The temperature in degrees Celsius when this report was
   * taken.
   */
  TemperatureReport(unsigned long currentTime, float currentTemp) {
    _isValid = true;

    _temp = currentTemp;
    _time = currentTime;
  }

  /**
   * @brief Get the temperature reported.
   *
   * @return float The temperature in degrees Celsius. Only meaningful if
   * IsValid() is true.
   */
  float GetTemp() { return _isValid ? _temp : 0.0f; }

  /**
   * @brief Get the timestamp of the report.
   *
   * @return unsigned long int The time offset in milliseconds since the start
   * of the cook when this report was taken. Only meaningful if IsValid() is
   * true.
   */
  unsigned long int GetTimestamp() { return _isValid ? _time : 0; }

  /**
   * @brief Check if the report is valid.
   *
   * @return true If the report is valid.
   * @return false If the report is invalid.
   */
  bool IsValid() { return _isValid; }

private:
  bool _isValid = false;
  float _temp = 0.0f;
  unsigned long int _time = 0;
};

#endif