#ifndef TEMP_HISTORY
#define TEMP_HISTORY

#include "TemperatureReport.h"

const int MAX_REPORTS = 500; // If sampling at once every 5 seconds, this allows
                             // for about 40 minutes of history

class TemperatureHistory {
public:
  TemperatureHistory() {}

  void Report(TemperatureReport report) {
    if (_reportCount >= MAX_REPORTS) {
      _head = (_head + 1) % MAX_REPORTS;
      --_reportCount;
    }

    _reports[(_head + _reportCount) % MAX_REPORTS] = report;
    ++_reportCount;
  }

  float GetCurrentTemp() {
    if (_reportCount <= 0) {
      return 0.0f;
    }

    return _reports[(_head + _reportCount - 1) % MAX_REPORTS].GetTemp();
  }

  TemperatureReport GetReport(int index) {
    if (index < 0 || index >= _reportCount) {
      return TemperatureReport();
    }

    return _reports[(_head + index) % MAX_REPORTS];
  }

  int GetReportCount() { return _reportCount; }

private:
  TemperatureReport _reports[MAX_REPORTS];
  int _head = 0;
  int _reportCount = 0;
};

#endif
