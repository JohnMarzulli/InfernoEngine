#include "FanHardwareController.h"
#include "TemperatureHistory.h"
#include "CombustionDeviceManager.h"
#include "TemperatureReport.h"
#include "ThermalControlManager.h"

CombustionDeviceManager _combustionDeviceManager;
FanHardwareController _fanHardwareController(0);
ThermalControlManager _thermalControlManager;
unsigned long _startTime;
unsigned long _lastUpdateTime;

void setup() {
  _startTime = millis();
  _lastUpdateTime = _startTime - 10000;

  // Validate config

  // Create CombustionConnectionManager

  _fanHardwareController.SetTarget(0.0);

  _thermalControlManager = ThermalControlManager();
}

void loop() {
  unsigned long timeStamp = millis();
  unsigned long timeBetweenUpdates = 1000;

  // put your main code here, to run repeatedly:
  unsigned long timeSinceLastUpdate = timeStamp - _lastUpdateTime;

  _combustionDeviceManager.Service();

  if (timeSinceLastUpdate > timeBetweenUpdates && _combustionDeviceManager.IsConnected()) {
    _lastUpdateTime = millis();

    float currentPitTemperature = _combustionDeviceManager.GetCurrentTemperature();
    TemperatureReport newReport = TemperatureReport(timeStamp, currentPitTemperature);

    _thermalControlManager.AddReport(newReport);
  }

  _thermalControlManager.Service();

  float targetFanProportion = _thermalControlManager.GetTargetFanProportion();

  _fanHardwareController.Service(targetFanProportion);
}
