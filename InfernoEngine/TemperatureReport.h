#ifndef TEMPERATURE_REPORT_H
#define TEMPERATURE_REPORT_H

class TemperatureReport {
public:
  TemperatureReport() {
    _isValid = false;
    _temp = 0.0;
    _time = 0;
  }

  TemperatureReport(
    unsigned long currentTime,
    float currentTemp) {
    _isValid = true;

    _temp = currentTemp;
    _time = currentTime;
  }

  float GetTemp() {
    return _temp;
  }

  unsigned long int GetTimestamp() {
    return _time;
  }

  bool IsValid() {
    return _isValid;
  }

private:
  bool _isValid = false;
  float _temp = 0.0f;
  unsigned long int _time = 0;
};

#endif