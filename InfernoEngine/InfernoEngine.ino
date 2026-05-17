#include "FanHardwareController.h"
#include "TemperatureHistory.h"
#include "CombustionDeviceManager.h"
#include "TemperatureReport.h"
#include "ThermalControlManager.h"
#include "UnconnectedControlManager.h"

#define FAN_CONTROL_OUTPUT_PIN 0
CombustionDeviceManager _combustionDeviceManager;
FanHardwareController _fanHardwareController(FAN_CONTROL_OUTPUT_PIN);
UnconnectedControlManager _unconnectedControlManger;
ThermalControlManager _thermalControlManager;
unsigned long _startTime;
unsigned long _lastUpdateTime;

void setup() {
  _startTime = millis();
  _lastUpdateTime = _startTime - 10000;

  // Validate config

  // Create CombustionConnectionManager

  _fanHardwareController.SetTarget(0.0);

  _unconnectedControlManger = UnconnectedControlManager();
  _thermalControlManager = ThermalControlManager();
}

void loop() {
  unsigned long timeStamp = millis();
  unsigned long timeBetweenUpdates = 1000;
  // put your main code here, to run repeatedly:
  unsigned long timeSinceLastUpdate = timeStamp - _lastUpdateTime;

  _combustionDeviceManager.Service();

  bool isConnected = _combustionDeviceManager.IsConnected();

  if (timeSinceLastUpdate > timeBetweenUpdates && isConnected) {
    _lastUpdateTime = millis();

    float currentPitTemperature = _combustionDeviceManager.GetCurrentTemperature();
    TemperatureReport newReport = TemperatureReport(timeStamp, currentPitTemperature);

    _thermalControlManager.AddReport(newReport);
    printf("Added report. Temperature: %fC\n", currentPitTemperature);
  }

  float targetFanProportion = 0;

  if (isConnected) {
    _thermalControlManager.Service();
    targetFanProportion = _thermalControlManager.GetTargetFanProportion();
  } else {
    _unconnectedControlManger.Service();
    targetFanProportion = _unconnectedControlManger.GetTargetFanProportion();
  }

  _fanHardwareController.Service(targetFanProportion);
}
